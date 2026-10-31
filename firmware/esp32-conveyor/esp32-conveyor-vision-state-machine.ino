/*
  ESP32 Vision-Based Conveyor Sorting System
  Board: classic ESP32 DevKit V1 / ESP32-WROOM-32
  Libraries: ESP32Servo (ESP32 Arduino core 3.x)

  Workflow:
    IDLE -> BELT_FEEDING -> WAITING_FOR_VISION -> SERVO_HOLD
         -> SERVO_HOME_SETTLE -> CLEARING_OBJECT -> IDLE

  The ESP32 owns the conveyor, sensors, physical mode button and servos.
  A PC running the Python vision program polls /vision-request. Vision only
  runs while the ESP32 reports an active request after the IR sensor detects
  an object. Python submits one result to POST /classification.

  Wi-Fi AP: Sorting-Conveyor
  Password: sort1234
  ESP32 address: http://192.168.4.1

  GPIO map:
    4 Red status LED             5 Passive buzzer
    12 Mode button               13 Colour mode LED
    14 Servo 2                   18 HC-SR04 TRIG
    19 HC-SR04 ECHO              21 Shape mode LED
    22 Green status LED          23 Yellow status LED
    25 L298N ENB                 26 L298N IN3
    27 L298N IN4                 32 IR sensor OUT
    33 Servo 1                   34 Potentiometer signal

  Vision result mapping (edit in applyVisionResult() if needed):
    COLOUR: GREEN -> Servo 1 / 90 deg; BLUE -> Servo 1 / 0 deg;
            ORANGE -> Servo 2 / 135 deg; YELLOW/UNKNOWN -> bypass.
    SHAPE:  TRIANGLE -> Servo 1 / 90 deg; SQUARE -> Servo 1 / 0 deg;
            CIRCLE -> Servo 2 / 135 deg; RECTANGLE/OTHER/UNKNOWN -> bypass.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// ----------------------------- Wi-Fi ------------------------------------
const char* AP_SSID = "Sorting-Conveyor";
const char* AP_PASSWORD = "sort1234";
WebServer server(80);

// ----------------------------- GPIO -------------------------------------
constexpr int PIN_LED_RED = 4;
constexpr int PIN_BUZZER = 5;
constexpr int PIN_BUTTON_MODE = 12;
constexpr int PIN_LED_COLOUR = 13;
constexpr int PIN_SERVO_2 = 14;
constexpr int PIN_TRIG = 18;
constexpr int PIN_ECHO = 19;
constexpr int PIN_LED_SHAPE = 21;
constexpr int PIN_LED_GREEN = 22;
constexpr int PIN_LED_YELLOW = 23;
constexpr int PIN_MOTOR_ENB = 25;
constexpr int PIN_MOTOR_IN3 = 26;
constexpr int PIN_MOTOR_IN4 = 27;
constexpr int PIN_IR = 32;
constexpr int PIN_SERVO_1 = 33;
constexpr int PIN_POT = 34;

// ----------------------------- Settings ---------------------------------
constexpr bool IR_ACTIVE_LOW = true;
constexpr float OBJECT_DISTANCE_CM = 12.0;
constexpr uint32_t SERVO_HOLD_MS = 5000;
constexpr uint32_t SERVO_HOME_SETTLE_MS = 400;
constexpr uint32_t DEBOUNCE_MS = 45;
constexpr uint32_t DOUBLE_CLICK_MS = 350;
constexpr uint32_t DISTANCE_SAMPLE_MS = 100;
constexpr uint32_t PWM_FREQ = 20000;
constexpr uint8_t PWM_BITS = 8;
constexpr int MIN_MOTOR_PWM = 95;
constexpr int MAX_MOTOR_PWM = 255;

// ----------------------------- Hardware ---------------------------------
Servo servo1;
Servo servo2;
ESP32PWM motorPwm;

// ----------------------------- State machine ----------------------------
enum State {
  IDLE,
  BELT_FEEDING,
  WAITING_FOR_VISION,
  SERVO_HOLD,
  SERVO_HOME_SETTLE,
  CLEARING_OBJECT
};

State state = IDLE;

// colourMode is the currently selected physical mode.
// activeColourMode is locked when the IR sensor detects an object.
bool colourMode = true;
bool activeColourMode = true;
bool previousModeButton = HIGH;

uint32_t lastButtonPress = 0;
uint32_t lastClickTime = 0;
uint8_t clickCount = 0;

uint32_t stateStarted = 0;
uint32_t lastDistanceAt = 0;
uint32_t visionRequestId = 0;
float lastDistanceCm = -1.0;
int motorPWM = 0;
int filteredPot = 0;
int selectedProduct = 0;
String lastVisionResult = "None";
String messageText = "Ready. Waiting for an object.";

// ----------------------------- Helpers ----------------------------------
const char* stateText() {
  switch (state) {
    case IDLE: return "Idle";
    case BELT_FEEDING: return "Conveyor running";
    case WAITING_FOR_VISION: return "Waiting for vision result";
    case SERVO_HOLD: return "Sorting action";
    case SERVO_HOME_SETTLE: return "Servo returning home";
    case CLEARING_OBJECT: return "Clearing object";
    default: return "Unknown";
  }
}

bool objectAtIR() {
  const int value = digitalRead(PIN_IR);
  return IR_ACTIVE_LOW ? value == LOW : value == HIGH;
}

float distanceCM() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(3);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  const uint32_t duration = pulseIn(PIN_ECHO, HIGH, 30000UL);
  if (duration == 0) return -1.0;
  return duration * 0.0343f / 2.0f;
}

void updateDistance() {
  const uint32_t now = millis();
  if (now - lastDistanceAt < DISTANCE_SAMPLE_MS) return;
  lastDistanceAt = now;
  lastDistanceCm = distanceCM();
}

void readPot() {
  const int raw = analogRead(PIN_POT);
  filteredPot = (filteredPot * 3 + raw) / 4;
  motorPWM = map(filteredPot, 0, 4095, MIN_MOTOR_PWM, MAX_MOTOR_PWM);
  motorPWM = constrain(motorPWM, MIN_MOTOR_PWM, MAX_MOTOR_PWM);
}

void motorStop() {
  motorPwm.write(0);
  digitalWrite(PIN_MOTOR_IN3, LOW);
  digitalWrite(PIN_MOTOR_IN4, LOW);
}

void motorRun() {
  readPot();
  digitalWrite(PIN_MOTOR_IN3, HIGH);
  digitalWrite(PIN_MOTOR_IN4, LOW);
  motorPwm.write(motorPWM);
}

void statusLEDs(bool red, bool green, bool yellow) {
  digitalWrite(PIN_LED_RED, red ? HIGH : LOW);
  digitalWrite(PIN_LED_GREEN, green ? HIGH : LOW);
  digitalWrite(PIN_LED_YELLOW, yellow ? HIGH : LOW);
}

void updateModeLEDs() {
  digitalWrite(PIN_LED_COLOUR, colourMode ? HIGH : LOW);
  digitalWrite(PIN_LED_SHAPE, colourMode ? LOW : HIGH);
}

void beep(uint16_t durationMs = 40) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(durationMs);
  digitalWrite(PIN_BUZZER, LOW);
}

void homeServos() {
  servo1.write(180);
  servo2.write(180);
}

// ----------------------------- State transitions ------------------------
void beginFeed() {
  state = BELT_FEEDING;
  stateStarted = millis();
  selectedProduct = 0;
  lastVisionResult = "None";
  messageText = "Object detected by ultrasonic sensor. Conveyor started.";
  Serial.println(messageText);
  motorRun();
  statusLEDs(false, true, false);
}

void beginVisionRequest() {
  motorStop();

  // Lock the mode for this object so a button press cannot change the
  // classification type while Python is processing the current request.
  activeColourMode = colourMode;

  ++visionRequestId;
  if (visionRequestId == 0) ++visionRequestId;

  state = WAITING_FOR_VISION;
  stateStarted = millis();
  lastVisionResult = "None";

  messageText = activeColourMode
    ? "IR detected object. Waiting for COLOUR vision result."
    : "IR detected object. Waiting for SHAPE vision result.";

  Serial.printf("Vision request %lu started. Mode=%s\n",
                static_cast<unsigned long>(visionRequestId),
                activeColourMode ? "COLOUR" : "SHAPE");
  statusLEDs(false, false, true);
}

// Maps a vision class to the existing three servo actions plus bypass.
// Change these labels/angles here if your physical sorting lanes differ.
int applyVisionResult(const String& result) {
  int product = 4; // Default: bypass / no servo movement.

  if (activeColourMode) {
    if (result == "GREEN") product = 1;
    else if (result == "BLUE") product = 2;
    else if (result == "ORANGE") product = 3;
    else product = 4; // YELLOW or UNKNOWN -> bypass for now.
  } else {
    if (result == "TRIANGLE") product = 1;
    else if (result == "SQUARE") product = 2;
    else if (result == "CIRCLE") product = 3;
    else product = 4; // RECTANGLE, OTHER or UNKNOWN -> bypass for now.
  }

  selectedProduct = product;
  lastVisionResult = result;

  switch (product) {
    case 1:
      servo1.write(90);
      messageText = "Vision result " + result + ". Servo 1 commanded to 90 degrees.";
      break;
    case 2:
      servo1.write(0);
      messageText = "Vision result " + result + ". Servo 1 commanded to 0 degrees.";
      break;
    case 3:
      servo2.write(135);
      messageText = "Vision result " + result + ". Servo 2 commanded to 135 degrees.";
      break;
    default:
      messageText = "Vision result " + result + ". Bypass: no servo movement.";
      break;
  }

  Serial.println(messageText);

  // Classification is complete. Changing to SERVO_HOLD makes
  // /vision-request report active=false, so Python stops classifying.
  state = SERVO_HOLD;
  stateStarted = millis();
  motorRun();
  statusLEDs(false, true, false);
  beep(60);
  return product;
}

// ----------------------------- HTTP API ---------------------------------
void handleVisionRequest() {
  const bool active = (state == WAITING_FOR_VISION);
  String json = "{\"active\":";
  json += active ? "true" : "false";
  json += ",\"requestId\":" + String(visionRequestId);
  json += ",\"mode\":\"";
  json += active ? (activeColourMode ? "COLOUR" : "SHAPE")
                 : (colourMode ? "COLOUR" : "SHAPE");
  json += "\"}";
  server.send(200, "application/json", json);
}

void handleClassification() {
  if (state != WAITING_FOR_VISION) {
    server.send(409, "text/plain", "No active vision request.");
    return;
  }

  if (!server.hasArg("requestId") || !server.hasArg("mode") || !server.hasArg("result")) {
    server.send(400, "text/plain", "Required fields: requestId, mode, result.");
    return;
  }

  const uint32_t receivedRequestId = static_cast<uint32_t>(server.arg("requestId").toInt());
  String receivedMode = server.arg("mode");
  String result = server.arg("result");
  receivedMode.trim();
  receivedMode.toUpperCase();
  result.trim();
  result.toUpperCase();

  if (receivedRequestId != visionRequestId) {
    server.send(409, "text/plain", "Stale requestId. Result ignored.");
    return;
  }

  const String expectedMode = activeColourMode ? "COLOUR" : "SHAPE";
  if (receivedMode != expectedMode) {
    server.send(409, "text/plain", "Mode mismatch. Result ignored.");
    return;
  }

  bool validResult = false;
  if (activeColourMode) {
    validResult = (result == "GREEN" || result == "BLUE" || result == "ORANGE" ||
                   result == "YELLOW" || result == "UNKNOWN");
  } else {
    validResult = (result == "TRIANGLE" || result == "SQUARE" ||
                   result == "RECTANGLE" || result == "CIRCLE" ||
                   result == "OTHER" || result == "UNKNOWN");
  }

  if (!validResult) {
    server.send(400, "text/plain", "Unsupported result label for the active mode.");
    return;
  }

  applyVisionResult(result);
  server.send(200, "text/plain", "Accepted result: " + result);
}

void handleStatus() {
  String json = "{";
  json += "\"state\":\"" + String(stateText()) + "\",";
  json += "\"mode\":\"" + String(colourMode ? "Colour" : "Shape") + "\",";
  json += "\"activeMode\":\"" + String(activeColourMode ? "Colour" : "Shape") + "\",";
  json += "\"pwm\":" + String(motorPWM) + ",";
  json += "\"distance\":" + String(lastDistanceCm, 1) + ",";
  json += "\"ir\":" + String(objectAtIR() ? "true" : "false") + ",";
  json += "\"modeButton\":" + String(digitalRead(PIN_BUTTON_MODE) == LOW ? "true" : "false") + ",";
  json += "\"requestId\":" + String(visionRequestId) + ",";
  json += "\"lastResult\":\"" + lastVisionResult + "\",";
  json += "\"message\":\"" + messageText + "\"}";
  server.send(200, "application/json", json);
}

String makePage() {
  return R"HTML(
<!doctype html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Vision Conveyor</title>
<style>
body{font-family:Arial,sans-serif;background:#101820;color:#eef3f7;margin:0;padding:16px}
main{max-width:760px;margin:auto}.card{background:#1c2a36;border-radius:12px;padding:16px;margin:12px 0}
.muted{color:#b7c5d0}.badge{background:#314758;padding:5px 8px;border-radius:7px}
.value{font-weight:bold}
</style>
</head>
<body>
<main>
<h1>Vision-Based Conveyor Sorting</h1>
<p class="muted">The Python vision program runs only after the IR sensor detects an object.</p>
<div class="card">
<h2>Live status</h2>
<p>State: <span class="badge" id="state">Loading</span></p>
<p>Selected physical mode: <span class="value" id="mode">--</span></p>
<p>Object mode locked for current cycle: <span class="value" id="activeMode">--</span></p>
<p>Motor PWM: <span class="value" id="pwm">--</span>/255</p>
<p>Ultrasonic distance: <span class="value" id="distance">--</span></p>
<p>IR sensor: <span class="value" id="ir">--</span></p>
<p>Vision request ID: <span class="value" id="requestId">--</span></p>
<p>Last vision result: <span class="value" id="lastResult">--</span></p>
<p class="muted" id="msg">--</p>
</div>
<div class="card">
<h3>Operating sequence</h3>
<p>1. Select colour mode with one click or shape mode with a double click of the GPIO 12 button.</p>
<p>2. The ultrasonic sensor starts the conveyor when an object enters its detection range.</p>
<p>3. The IR sensor stops the conveyor and activates one vision request for the selected mode.</p>
<p>4. Python sends one classification result to the ESP32. The vision request then becomes inactive.</p>
<p>5. The ESP32 performs the mapped servo action, resumes the conveyor and waits for the object to clear the IR sensor.</p>
</div>
</main>
<script>
async function refresh(){
  try{
    const response=await fetch('/status');
    const d=await response.json();
    document.getElementById('state').textContent=d.state;
    document.getElementById('mode').textContent=d.mode;
    document.getElementById('activeMode').textContent=d.activeMode;
    document.getElementById('pwm').textContent=d.pwm;
    document.getElementById('distance').textContent=d.distance<0?'No echo':d.distance.toFixed(1)+' cm';
    document.getElementById('ir').textContent=d.ir?'Object detected':'Clear';
    document.getElementById('requestId').textContent=d.requestId;
    document.getElementById('lastResult').textContent=d.lastResult;
    document.getElementById('msg').textContent=d.message;
  }catch(e){}
}
setInterval(refresh,500);
refresh();
</script>
</body>
</html>
)HTML";
}

// ----------------------------- Button logic -----------------------------
void handleModeButton() {
  const uint32_t now = millis();
  const bool currentButton = digitalRead(PIN_BUTTON_MODE);

  if (previousModeButton == HIGH && currentButton == LOW) {
    if (now - lastButtonPress >= DEBOUNCE_MS) {
      lastButtonPress = now;
      ++clickCount;
      lastClickTime = now;
      Serial.printf("Mode button click detected: %u\n", clickCount);
    }
  }
  previousModeButton = currentButton;

  if (clickCount > 0 && now - lastClickTime >= DOUBLE_CLICK_MS) {
    // Permit mode changes while idle or while an object is still approaching.
    // The mode is locked as soon as the IR sensor starts a vision request.
    if (state == IDLE || state == BELT_FEEDING) {
      if (clickCount == 1) {
        colourMode = true;
        messageText = "Colour mode selected by single click.";
      } else {
        colourMode = false;
        messageText = "Shape mode selected by double click.";
      }
      updateModeLEDs();
      Serial.println(messageText);
      beep(60);
    } else {
      Serial.println("Mode change ignored: current object cycle is in progress.");
      beep(20);
    }
    clickCount = 0;
  }
}

// ----------------------------- Setup / loop -----------------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nESP32 Vision Conveyor - booting");

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BUTTON_MODE, INPUT_PULLUP);
  pinMode(PIN_LED_COLOUR, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED_SHAPE, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_MOTOR_IN3, OUTPUT);
  pinMode(PIN_MOTOR_IN4, OUTPUT);
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_POT, INPUT);
  // Servo pins and ENB are configured by their PWM drivers.

  digitalWrite(PIN_TRIG, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_POT, ADC_11db);
  filteredPot = analogRead(PIN_POT);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  servo1.setPeriodHertz(50);
  servo2.setPeriodHertz(50);
  const int servo1Attach = servo1.attach(PIN_SERVO_1, 500, 2400);
  const int servo2Attach = servo2.attach(PIN_SERVO_2, 500, 2400);
  Serial.printf("Servo 1 GPIO %d attach=%d attached=%s\n", PIN_SERVO_1, servo1Attach, servo1.attached() ? "YES" : "NO");
  Serial.printf("Servo 2 GPIO %d attach=%d attached=%s\n", PIN_SERVO_2, servo2Attach, servo2.attached() ? "YES" : "NO");

  motorPwm.attachPin(PIN_MOTOR_ENB, PWM_FREQ, PWM_BITS);
  motorStop();
  homeServos();

  updateModeLEDs();
  statusLEDs(true, false, false);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("Wi-Fi AP: "); Serial.println(AP_SSID);
  Serial.print("Dashboard/API: http://"); Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() { server.send(200, "text/html", makePage()); });
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/vision-request", HTTP_GET, handleVisionRequest);
  server.on("/classification", HTTP_POST, handleClassification);
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
  server.begin();

  messageText = "Ready. Connect to Sorting-Conveyor Wi-Fi and open 192.168.4.1.";
  Serial.println(messageText);
}

void loop() {
  server.handleClient();
  handleModeButton();
  readPot();
  updateDistance();

  switch (state) {
    case IDLE:
      motorStop();
      statusLEDs(true, false, false);
      if (lastDistanceCm > 0 && lastDistanceCm <= OBJECT_DISTANCE_CM) {
        beginFeed();
      }
      break;

    case BELT_FEEDING:
      motorRun();
      statusLEDs(false, true, false);
      if (objectAtIR()) {
        beginVisionRequest();
      }
      break;

    case WAITING_FOR_VISION:
      // Keep the conveyor stopped until one valid result arrives.
      motorStop();
      statusLEDs(false, false, true);
      break;

    case SERVO_HOLD:
      motorRun();
      statusLEDs(false, true, false);
      if (millis() - stateStarted >= SERVO_HOLD_MS) {
        homeServos();
        state = SERVO_HOME_SETTLE;
        stateStarted = millis();
        messageText = "Servos returned to home position.";
        Serial.println(messageText);
      }
      break;

    case SERVO_HOME_SETTLE:
      motorRun();
      statusLEDs(false, true, false);
      if (millis() - stateStarted >= SERVO_HOME_SETTLE_MS) {
        state = CLEARING_OBJECT;
        messageText = "Conveyor running until the IR sensor clears.";
        Serial.println(messageText);
      }
      break;

    case CLEARING_OBJECT:
      motorRun();
      statusLEDs(false, true, false);
      if (!objectAtIR()) {
        motorStop();
        state = IDLE;
        selectedProduct = 0;
        messageText = "Cycle complete. Waiting for the next object.";
        Serial.println(messageText);
        statusLEDs(true, false, false);
      }
      break;
  }

  delay(10);
}
