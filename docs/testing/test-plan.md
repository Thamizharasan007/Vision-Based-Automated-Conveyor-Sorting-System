# System Test Plan

## 1. Purpose

This document defines the test procedures for the vision-based conveyor sorting system. The objective is to verify the camera stream, Python vision modules, ESP32 firmware, sensors, motor, servos, physical controls and Wi-Fi dashboard.

Tests should be performed in stages. Record actual results rather than marking tests as passed before they have been verified.

## 2. Test Environment

### Hardware
- ESP32 DevKit V1 / ESP32-WROOM-32
- L298N motor driver
- 12 V DC conveyor motor
- HC-SR04 ultrasonic sensor
- IR obstacle sensor
- Two servo motors
- Potentiometer
- Physical pushbutton
- Indicator LEDs and passive buzzer
- Mobile phone used as an IP camera
- Computer running Python and OpenCV
- Conveyor frame and sorting mechanism

### Software
- Arduino IDE with ESP32 Arduino core 3.x
- ESP32Servo library
- Python 3
- OpenCV and NumPy
- Visual Studio Code
- Git

## 3. Test Status Convention

Use the following statuses when recording results:

- **Not tested:** Test has not yet been performed.
- **Pass:** The observed result matches the expected result.
- **Fail:** The observed result does not match the expected result.
- **Blocked:** A dependency or hardware issue prevents the test.

Do not mark a test as passed based solely on successful compilation.

## 4. Test Cases

### A. Camera and Vision Tests

| Test ID | Test procedure | Expected result | Status |
|---|---|---|---|
| VIS-01 | Start the mobile phone IP camera stream. | The camera application provides a live stream. | Not tested |
| VIS-02 | Run `vision/color_detection.py`. | OpenCV opens the stream and displays the video. | Not tested |
| VIS-03 | Place a green object in front of the camera. | The detector identifies green when its configured threshold is met. | Not tested |
| VIS-04 | Repeat with blue, orange and yellow objects. | Each configured colour can be detected under suitable conditions. | Not tested |
| VIS-05 | Replace blue with red in `ACTIVE_COLORS`. | The active colour configuration changes without modifying the detection loop. | Not tested |
| VIS-06 | Run `vision/shape_detection.py` with a triangle, square, rectangle and circle. | The detector attempts to classify each shape. | Not tested |
| VIS-07 | Run either script with the camera stream unavailable. | The program reports a camera-opening or frame-reading failure. | Not tested |
| VIS-08 | Press `q` during detection. | The video window closes and camera resources are released. | Not tested |

**Test notes:** Detection accuracy depends on lighting, background, object size, camera angle and HSV or contour settings. Record misclassifications as well as successful detections.

### B. ESP32 Startup and Dashboard Tests

| Test ID | Test procedure | Expected result | Status |
|---|---|---|---|
| MCU-01 | Compile and upload the firmware. | Compilation and upload complete without errors. | Not tested |
| MCU-02 | Open Serial Monitor at 115200 baud. | Startup messages and servo attachment information appear. | Not tested |
| MCU-03 | Connect to the `Sorting-Conveyor` access point. | The computer or phone connects using the configured credentials. | Not tested |
| MCU-04 | Open `http://192.168.4.1`. | The dashboard loads. | Not tested |
| MCU-05 | Observe the live status section. | State, mode, PWM, distance, sensor and message fields update. | Not tested |
| MCU-06 | Submit a dashboard selection while the system is not waiting for selection. | The firmware rejects the selection with an HTTP 409 response. | Not tested |

### C. Ultrasonic and IR Sensor Tests

| Test ID | Test procedure | Expected result | Status |
|---|---|---|---|
| SEN-01 | Power the firmware with no object in range. | The system remains idle if no valid in-range distance is detected. | Not tested |
| SEN-02 | Place an object within the configured 12 cm detection distance. | The conveyor starts. | Not tested |
| SEN-03 | Let the object reach the IR sensor while the conveyor is feeding. | The motor stops and the system waits for product selection. | Not tested |
| SEN-04 | Remove the object after a sorting cycle. | Once the IR sensor clears, the system returns to idle. | Not tested |
| SEN-05 | Observe the sensors with no valid ultrasonic echo. | No object distance is reported as valid from a zero-duration echo. | Not tested |

Check `IR_ACTIVE_LOW` against the actual sensor module. Record whether the sensor detects the object reliably at the intended conveyor position.

### D. Conveyor and Speed-Control Tests

| Test ID | Test procedure | Expected result | Status |
|---|---|---|---|
| MOT-01 | Verify wiring with motor power disconnected. | Connections match the approved wiring guide. | Not tested |
| MOT-02 | Start the conveyor using an ultrasonic trigger. | The motor rotates in the intended direction. | Not tested |
| MOT-03 | Adjust the potentiometer through its operating range. | The commanded motor PWM changes with the potentiometer. | Not tested |
| MOT-04 | Allow the IR sensor to detect an object during feeding. | The motor stops while waiting for manual selection. | Not tested |
| MOT-05 | Complete the sorting sequence. | The conveyor resumes and continues until the IR sensor clears. | Not tested |

PWM values are control commands, not direct measurements of motor speed. Record actual motor behaviour and check for stalling, overheating or excessive mechanical resistance.

### E. Servo Tests

| Test ID | Test procedure | Expected result | Status |
|---|---|---|---|
| SRV-01 | Start the firmware and inspect the Serial Monitor. | Both servos report successful attachment. | Not tested |
| SRV-02 | Select Green on the dashboard. | Servo 1 receives a 90° command. | Not tested |
| SRV-03 | Select Blue on the dashboard. | Servo 1 receives a 0° command. | Not tested |
| SRV-04 | Select Orange on the dashboard. | Servo 2 receives a 135° command. | Not tested |
| SRV-05 | Select Other. | No new sorting-angle command is issued; the conveyor sequence still proceeds. | Not tested |
| SRV-06 | Observe the end of the servo-hold interval. | The firmware commands both servos to the configured home position of 180°. | Not tested |

Servo commands do not guarantee the same physical angle on every installation. Check the linkage, clearance, travel limits and external power supply before testing with objects.

### F. Physical Mode-Selection Tests

The intended design uses one physical button on GPIO12.

| Test ID | Test procedure | Expected result | Status |
|---|---|---|---|
| MOD-01 | Press GPIO12 once and wait beyond the double-click interval. | Colour mode is selected and the colour-mode LED turns on. | Not tested |
| MOD-02 | Press GPIO12 twice within the double-click interval. | Shape mode is selected and the shape-mode LED turns on. | Not tested |
| MOD-03 | Repeat each test several times. | Mode selection remains consistent after debouncing. | Not tested |
| MOD-04 | Reset the ESP32 with the button released. | The ESP32 starts correctly. | Not tested |

**Firmware verification required:** The supplied sketch also declares GPIO15 as a shape-button input and reports its state on the dashboard. The actual mode-selection logic uses GPIO12 single/double clicks. Reconcile GPIO15 with the one-button circuit before considering the firmware and circuit documentation fully matched.

GPIO12 is an ESP32 strapping pin, so startup behaviour should be checked with the intended wiring.

### G. Wi-Fi Vision Integration Tests

These tests are pending implementation because the current ESP32 firmware does not yet expose the proposed `/classification` endpoint.

| Test ID | Test procedure | Expected result | Status |
|---|---|---|---|
| WIFI-01 | Confirm the computer can access the ESP32 dashboard. | The dashboard responds through the configured network. | Not tested |
| WIFI-02 | Implement the classification endpoint and send a valid message. | The ESP32 validates and acknowledges the message. | Blocked |
| WIFI-03 | Send an unsupported or malformed classification. | The endpoint rejects the message without triggering a sorting action. | Blocked |
| WIFI-04 | Simulate a connection timeout or unavailable ESP32. | Python reports the communication failure. | Blocked |
| WIFI-05 | Send a valid classification while the system is in an inappropriate state. | The ESP32 rejects or safely defers the request according to the implemented state logic. | Blocked |
| WIFI-06 | Run the complete vision-to-sorting workflow. | A correctly classified object triggers the intended sorting action. | Blocked |

The mobile phone camera address and ESP32 address are separate. Verify that the computer can reach both under the selected network arrangement.

## 5. End-to-End Validation

Once individual tests pass, perform a controlled full-system test:

1. Verify the wiring and power connections with the motor supply disconnected.
2. Power the controller and check the dashboard.
3. Confirm ultrasonic triggering and IR stopping.
4. Verify manual product selection and servo movement.
5. Confirm the conveyor resumes and clears the object.
6. After Wi-Fi classification handling is implemented, test the vision-to-ESP32 path.
7. Repeat the cycle with each supported colour or shape and record failures.

Begin with low-risk, supervised tests. Keep hands clear of moving parts and disconnect power before changing wiring.

## 6. Test Results Record

For each completed test, record:

- Test ID and date
- Hardware and software configuration
- Input or procedure used
- Expected result
- Actual result
- Pass, Fail or Blocked status
- Error messages or relevant photographs
- Corrective action and retest result

## 7. Completion Criteria

The system should not be described as fully validated until the camera, vision classification, Wi-Fi communication, ESP32 state handling, conveyor and servo operation have been tested together under representative conditions.

Passing the manual-control tests alone does not demonstrate automatic vision-driven sorting.
