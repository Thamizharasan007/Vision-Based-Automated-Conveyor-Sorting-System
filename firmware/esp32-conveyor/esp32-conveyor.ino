/*
 ESP32 Conveyor Manual Test Sketch  (FIXED)
 Board: classic ESP32 DevKit V1 / ESP32-WROOM-32
 Libraries: ESP32Servo (install from Library Manager), ESP32 Arduino core 3.x

 GPIO map supplied by user:
 4 Red status LED | 5 Passive buzzer | 12 Colour button | 13 Colour LED
 14 Servo 2 | 15 Shape button | 18 HC-SR04 TRIG | 19 HC-SR04 ECHO
 21 Shape LED | 22 Green status LED | 23 Yellow status LED
 25 L298N ENB | 26 L298N IN3 | 27 L298N IN4 | 32 IR OUT
 33 Servo 1 | 34 potentiometer signal | 35 unused

 CHANGES vs. previous version (everything else is untouched):
 1. Motor PWM now uses ESP32Servo's ESP32PWM class instead of raw ledcAttach(),
    and all 4 LEDC timers are reserved first. Mixing ledcAttach() with ESP32Servo
    lets both claim the same LEDC channel/timer, which can silently disable one
    servo and/or the motor PWM.
 2. Removed pinMode(OUTPUT) on the servo pins (the standalone test never did this).
 3. Pot is mapped to MIN_MOTOR_PWM..255 (was 0..255), so the motor cannot be
    stuck below its stall duty. Light low-pass filter added.
 4. Ultrasonic is sampled once in loop() and cached; /status no longer fires a
    second blocking measurement.
 5. Servo attach results are printed at boot. Button debug prints added.

 IMPORTANT:
 - HC-SR04 ECHO is 5 V: use a voltage divider to keep GPIO19 at <=3.3 V.
 - GPIO12 and GPIO15 are boot-strapping pins. Do not press/hold the buttons
   during reset/boot if it causes boot problems.
 - Use a suitable external 5 V supply for servos. Join all grounds.
 - L298N ENB jumper must be removed if GPIO25 is controlling ENB with PWM.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* AP_SSID = "Sorting-Conveyor";
const char* AP_PASSWORD = "sort1234";
WebServer server(80);

// GPIO assignments
constexpr int PIN_LED_RED = 4;
constexpr int PIN_BUZZER = 5;
constexpr int PIN_BUTTON_COLOUR = 12;
constexpr int PIN_LED_COLOUR = 13;
constexpr int PIN_SERVO_2 = 14;
constexpr int PIN_BUTTON_SHAPE = 15;
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

// Adjust these if your sensor behaves differently.
constexpr bool IR_ACTIVE_LOW = true;
constexpr float OBJECT_DISTANCE_CM = 12.0;
constexpr uint32_t SERVO_HOLD_MS = 5000;
constexpr uint32_t SERVO_HOME_SETTLE_MS = 400;
constexpr uint32_t DEBOUNCE_MS = 45;
constexpr uint32_t DISTANCE_SAMPLE_MS = 100;
constexpr uint32_t PWM_FREQ = 20000;
constexpr uint8_t PWM_BITS = 8;
constexpr int MIN_MOTOR_PWM = 95;   // raise if the motor hums/stalls at low pot setting
constexpr int MAX_MOTOR_PWM = 255;

Servo servo1, servo2;
ESP32PWM motorPwm;

enum State {
  IDLE, BELT_FEEDING, WAITING_FOR_SELECTION,
  SERVO_HOLD, SERVO_HOME_SETTLE, CLEARING_OBJECT
};
State state = IDLE;

bool colourMode = true;
bool previousColourButton = HIGH;

uint32_t lastButtonPress = 0;
uint32_t lastClickTime = 0;

uint8_t clickCount = 0;

constexpr uint32_t DOUBLE_CLICK_MS = 350;
uint32_t stateStarted = 0;
uint32_t lastDistanceAt = 0;
float lastDistanceCm = -1.0;
int motorPWM = 0;
int filteredPot = 0;
int selectedProduct = 0;
String messageText = "Ready. Waiting for an object.";

const char* stateText() {
  switch (state) {
    case IDLE: return "Idle";
    case BELT_FEEDING: return "Conveyor running";
    case WAITING_FOR_SELECTION: return "IR detected - choose manually";
    case SERVO_HOLD: return "Sorting action";
    case SERVO_HOME_SETTLE: return "Servo returning home";
    case CLEARING_OBJECT: return "Clearing IR sensor";
    default: return "Unknown";
  }
}

bool objectAtIR() {
  int v = digitalRead(PIN_IR);
  return IR_ACTIVE_LOW ? v == LOW : v == HIGH;
}

float distanceCM() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(3);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  uint32_t duration = pulseIn(PIN_ECHO, HIGH, 30000UL);
  if (duration == 0) return -1.0;
  return duration * 0.0343 / 2.0;
}

// Sample the ultrasonic sensor at a fixed rate; everything else uses the cached value.
void updateDistance() {
  uint32_t now = millis();
  if (now - lastDistanceAt < DISTANCE_SAMPLE_MS) return;
  lastDistanceAt = now;
  lastDistanceCm = distanceCM();
}

void readPot() {
  int raw = analogRead(PIN_POT);
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

void beep(uint16_t ms = 40) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(ms);
  digitalWrite(PIN_BUZZER, LOW);
}

void homeServos() {
  servo1.write(180);
  servo2.write(180);
}

void beginFeed() {
  state = BELT_FEEDING;
  stateStarted = millis();
  selectedProduct = 0;
  messageText = "Object in ultrasonic range. Conveyor started.";
  Serial.println(messageText);
  motorRun();
  statusLEDs(false, true, false);
}

void doManualSort(int product) {
  selectedProduct = product;
  motorStop();

  // Manual dashboard selection only. No camera/OpenCV code is active.
  // Green -> Servo 1 at 45 degrees; Blue -> Servo 1 at 135 degrees;
  // Orange -> Servo 2 at 45 degrees; Other -> no servo movement.
  switch (product) {
    case 1:
      servo1.write(90);
      messageText = colourMode
        ? "Colour mode: Green selected. Servo 1 commanded to 45 degrees."
        : "Shape mode: Shape A selected. Servo 1 commanded to 45 degrees.";
      Serial.println(messageText);
      break;
    case 2:
      servo1.write(0);
      messageText = colourMode
        ? "Colour mode: Blue selected. Servo 1 commanded to 135 degrees."
        : "Shape mode: Shape B selected. Servo 1 commanded to 135 degrees.";
      Serial.println(messageText);
      break;
    case 3:
      servo2.write(135);
      messageText = colourMode
        ? "Colour mode: Orange selected. Servo 2 commanded to 45 degrees."
        : "Shape mode: Shape C selected. Servo 2 commanded to 45 degrees.";
      Serial.println(messageText);
      break;
    case 4:
      messageText = "Other selected: no servo movement.";
      Serial.println(messageText);
      break;
    default:
      return;
  }
  state = SERVO_HOLD;
  stateStarted = millis();
  // Restart the belt immediately after product selection.
  motorRun();
  Serial.printf("Belt restarted, PWM=%d/255\n", motorPWM);
  statusLEDs(false, true, false);
  beep(60);
}

String makePage() {
  return R"HTML(
<!doctype html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Conveyor Test</title>
<style>
body{font-family:Arial;background:#101820;color:#eef3f7;margin:0;padding:16px}
main{max-width:760px;margin:auto}.card{background:#1c2a36;border-radius:12px;padding:16px;margin:12px 0}
button{padding:13px 16px;margin:5px;border:0;border-radius:9px;font-weight:bold;font-size:1rem}
.green{background:#42c878}.blue{background:#5aa7ff}.orange{background:#ffad55}.other{background:#d1d8df}
.muted{color:#b7c5d0}.badge{background:#314758;padding:5px 8px;border-radius:7px}
</style></head><body><main>
<h1>ESP32 Conveyor Test Dashboard</h1>
<p class="muted">Camera/OpenCV disabled. Select a product manually after the IR sensor stops the belt.</p>
<div class="card"><h2>Live status</h2>
<p>State: <span class="badge" id="state">Loading</span></p>
<p>Physical mode: <b id="mode">--</b></p>
<p>Motor PWM: <b id="pwm">--</b>/255</p>
<p>Ultrasonic distance: <b id="distance">--</b></p>
<p>IR sensor: <b id="ir">--</b></p>
<p>Colour button: <b id="colourButton">--</b></p>
<p>Shape button: <b id="shapeButton">--</b></p>
<p class="muted" id="msg">--</p></div>
<div class="card"><h2>Manual product selection</h2>
<p class="muted">Choose one only after the belt stops at the IR sensor.</p>
<button id="choice1" class="green" onclick="choose(1)">Green</button>
<button id="choice2" class="blue" onclick="choose(2)">Blue</button>
<button id="choice3" class="orange" onclick="choose(3)">Orange</button>
<button id="choice4" class="other" onclick="choose(4)">Other / no servo</button>
<p id="result" class="muted"></p></div>
<div class="card"><h3>Test sequence</h3>
<p>1. Place an object within 12 cm of the ultrasonic sensor.</p>
<p>2. The belt starts and stops when the IR sensor detects the object.</p>
<p>3. Select Green, Blue, Orange, or Other on this dashboard.</p>
<p>4. Selecting a product restarts the belt immediately. The servo holds for 5 seconds, returns home, and the belt continues until the IR sensor clears.</p>
</div></main>
<script>
async function refresh(){try{let r=await fetch('/status'),d=await r.json();
document.getElementById('state').textContent=d.state;
document.getElementById('mode').textContent=d.mode;
document.getElementById('pwm').textContent=d.pwm;
document.getElementById('distance').textContent=d.distance<0?'No echo':d.distance.toFixed(1)+' cm';
document.getElementById('ir').textContent=d.ir?'Object detected':'Clear';
document.getElementById('colourButton').textContent=d.colourButton?'PRESSED':'Released';
document.getElementById('shapeButton').textContent=d.shapeButton?'PRESSED':'Released';
if(d.mode==='Colour'){
 document.getElementById('choice1').textContent='Green';
 document.getElementById('choice2').textContent='Blue';
 document.getElementById('choice3').textContent='Orange';
 document.getElementById('choice4').textContent='Other / no servo';
}else{
 document.getElementById('choice1').textContent='Shape A';
 document.getElementById('choice2').textContent='Shape B';
 document.getElementById('choice3').textContent='Shape C';
 document.getElementById('choice4').textContent='Other / no servo';
}
document.getElementById('msg').textContent=d.message;}catch(e){}}
async function choose(p){let r=await fetch('/select?product='+p,{method:'POST'});
document.getElementById('result').textContent=await r.text();refresh();}
setInterval(refresh,700);refresh();
</script></body></html>
)HTML";
}

void handleStatus() {
  String json = "{";
  json += "\"state\":\"" + String(stateText()) + "\",";
  json += "\"mode\":\"" + String(colourMode ? "Colour" : "Shape") + "\",";
  json += "\"pwm\":" + String(motorPWM) + ",";
  json += "\"distance\":" + String(lastDistanceCm, 1) + ",";
  json += "\"ir\":" + String(objectAtIR() ? "true" : "false") + ",";
  json += "\"colourButton\":" + String(digitalRead(PIN_BUTTON_COLOUR) == LOW ? "true" : "false") + ",";
  json += "\"shapeButton\":" + String(digitalRead(PIN_BUTTON_SHAPE) == LOW ? "true" : "false") + ",";
  json += "\"message\":\"" + messageText + "\"}";
  server.send(200, "application/json", json);
}

void handleSelect() {
  if (state != WAITING_FOR_SELECTION) {
    server.send(409, "text/plain", "Conveyor is not waiting for a product. Wait for the IR sensor to stop it.");
    return;
  }
  if (!server.hasArg("product")) {
    server.send(400, "text/plain", "Missing product.");
    return;
  }
  int p = server.arg("product").toInt();
  if (p < 1 || p > 4) {
    server.send(400, "text/plain", "Invalid product.");
    return;
  }
  doManualSort(p);
  server.send(200, "text/plain", "Selection accepted. Sorting action started.");
}

void handleButtons() {
  uint32_t now = millis();
  bool currentButton = digitalRead(PIN_BUTTON_COLOUR);

  // Detect a new button press (INPUT_PULLUP means LOW = pressed)
  if (previousColourButton == HIGH && currentButton == LOW) {
    if (now - lastButtonPress >= DEBOUNCE_MS) {
      lastButtonPress = now;
      clickCount++;
      lastClickTime = now;

      Serial.printf("Button click detected: %d\n", clickCount);
    }
  }

  previousColourButton = currentButton;

  // Wait for the double-click window to expire before selecting a mode
  if (clickCount > 0 && now - lastClickTime >= DOUBLE_CLICK_MS) {

    if (clickCount == 1) {
      // Single click = Colour mode
      colourMode = true;

      updateModeLEDs();

      messageText = "Colour mode selected by single click.";
      Serial.println(messageText);

      beep(60);
    }
    else {
      // Two or more clicks = Shape mode
      colourMode = false;

      updateModeLEDs();

      messageText = "Shape mode selected by double click.";
      Serial.println(messageText);

      beep(60);
    }

    clickCount = 0;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nESP32 Conveyor Test - booting");

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BUTTON_COLOUR, INPUT_PULLUP);
  pinMode(PIN_LED_COLOUR, OUTPUT);
  pinMode(PIN_BUTTON_SHAPE, INPUT_PULLUP);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED_SHAPE, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_MOTOR_IN3, OUTPUT);
  pinMode(PIN_MOTOR_IN4, OUTPUT);
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_POT, INPUT);
  // NOTE: servo pins and ENB are intentionally NOT set with pinMode();
  // the PWM drivers configure them.

  digitalWrite(PIN_TRIG, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_POT, ADC_11db);
  filteredPot = analogRead(PIN_POT);

  // Reserve all LEDC timers for the ESP32Servo/ESP32PWM allocator so servos
  // (50 Hz) and the motor (20 kHz) never share a channel or timer.
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  servo1.setPeriodHertz(50);
  servo2.setPeriodHertz(50);
  int a1 = servo1.attach(PIN_SERVO_1, 500, 2400);
  int a2 = servo2.attach(PIN_SERVO_2, 500, 2400);
  Serial.printf("Servo 1 GPIO %d attach=%d attached=%s\n", PIN_SERVO_1, a1, servo1.attached() ? "YES" : "NO");
  Serial.printf("Servo 2 GPIO %d attach=%d attached=%s\n", PIN_SERVO_2, a2, servo2.attached() ? "YES" : "NO");

  // Motor ENB PWM through the same allocator as the servos.
  motorPwm.attachPin(PIN_MOTOR_ENB, PWM_FREQ, PWM_BITS);
  motorStop();
  homeServos();

  updateModeLEDs();
  statusLEDs(true, false, false);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("Wi-Fi: "); Serial.println(AP_SSID);
  Serial.print("Dashboard: http://"); Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() { server.send(200, "text/html", makePage()); });
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/select", HTTP_POST, handleSelect);
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
  server.begin();
  messageText = "Ready. Connect to Sorting-Conveyor Wi-Fi and open 192.168.4.1.";
}

void loop() {
  server.handleClient();
  handleButtons();
  readPot();
  updateDistance();

  switch (state) {
    case IDLE:
      motorStop();
      statusLEDs(true, false, false);
      if (lastDistanceCm > 0 && lastDistanceCm <= OBJECT_DISTANCE_CM) beginFeed();
      break;
    case BELT_FEEDING:
      motorRun();
      statusLEDs(false, true, false);
      if (objectAtIR()) {
        motorStop();
        state = WAITING_FOR_SELECTION;
        stateStarted = millis();
        messageText = "IR detected an object. Choose a product on the dashboard.";
        Serial.println(messageText);
        statusLEDs(false, false, true);
      }
      break;
    case WAITING_FOR_SELECTION:
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
        messageText = "Servos returned home.";
        Serial.println(messageText);
      }
      break;
    case SERVO_HOME_SETTLE:
      motorRun();
      statusLEDs(false, true, false);
      if (millis() - stateStarted >= SERVO_HOME_SETTLE_MS) {
        state = CLEARING_OBJECT;
        messageText = "Conveyor resumed to clear the IR sensor.";
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

