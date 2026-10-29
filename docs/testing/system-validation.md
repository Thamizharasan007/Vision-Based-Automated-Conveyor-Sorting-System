# System Validation Report

## 1. Objective

The purpose of this document is to validate the operation and integration of the vision-based automated conveyor sorting system.

Validation covers the mechanical conveyor, ESP32 controller, sensors, motor driver, servos, Wi-Fi dashboard, mobile phone IP camera and Python/OpenCV vision modules.

Tests must be performed on the assembled system. Individual module tests alone do not establish that the complete system works correctly.

## 2. System Architecture

The project consists of three main subsystems.

### Vision Subsystem
- Mobile phone used as an IP camera.
- Live video stream accessed by the computer over the network.
- Python and OpenCV for image processing.
- Colour detection and basic geometric shape classification.

### Embedded Control Subsystem
- ESP32 DevKit V1 / ESP32-WROOM-32.
- Ultrasonic sensor for object proximity detection.
- IR sensor for detecting an object at the sorting position.
- L298N motor driver and DC conveyor motor.
- Two servo motors for sorting actions.
- Potentiometer for motor PWM adjustment.
- Physical mode-selection button, LEDs and buzzer.
- ESP32-hosted Wi-Fi dashboard.

### Integration Subsystem
The intended integrated system sends classification results from Python to the ESP32 over Wi-Fi. The ESP32 then validates the result and executes the appropriate sorting action.

**Current status:** The supplied ESP32 firmware supports manual dashboard selection. The proposed Wi-Fi classification endpoint is not yet implemented in that firmware.

## 3. Validation Status

Use the following statuses:

- **Not tested:** No test has been performed.
- **Pass:** The observed result matches the expected behaviour.
- **Fail:** The observed result does not match the expected behaviour.
- **Blocked:** Testing depends on an unfinished feature or unavailable component.

All tests below begin as unverified. Update their statuses only after testing the physical system.

## 4. Individual Subsystem Validation

### A. Mechanical System

| Test ID | Validation item | Expected result | Status |
|---|---|---|---|
| MEC-01 | Inspect conveyor frame and mounting | Structure is stable during operation | Not tested |
| MEC-02 | Inspect motor bracket and roller coupler | Motor drives the conveyor without noticeable slipping | Not tested |
| MEC-03 | Run the conveyor without an object | Belt movement is consistent and unobstructed | Not tested |
| MEC-04 | Test object movement along the conveyor | Object travels towards the sensing and sorting position | Not tested |
| MEC-05 | Inspect sorting mechanism clearance | Servo linkages move without binding or obstruction | Not tested |

### B. Vision System

| Test ID | Validation item | Expected result | Status |
|---|---|---|---|
| VIS-01 | Start the mobile phone IP camera | Computer receives live video frames | Not tested |
| VIS-02 | Test configured colour detection | Supported colours are classified under suitable conditions | Not tested |
| VIS-03 | Test basic shape detection | Supported shapes are classified under suitable conditions | Not tested |
| VIS-04 | Test changes in lighting and object distance | Detection limitations are identified and documented | Not tested |
| VIS-05 | Test camera stream interruption | Program reports the failure and releases resources safely | Not tested |

Detailed vision test procedures and results belong in `vision-test-results.md`.

### C. Embedded Control System

| Test ID | Validation item | Expected result | Status |
|---|---|---|---|
| MCU-01 | Start ESP32 firmware | Controller boots successfully | Not tested |
| MCU-02 | Connect to the dashboard | Dashboard opens at the configured address | Not tested |
| MCU-03 | Test ultrasonic trigger | Conveyor starts when an object enters the configured detection range | Not tested |
| MCU-04 | Test IR sensor | Conveyor stops when the IR sensor detects the object during feeding | Not tested |
| MCU-05 | Test potentiometer | Motor PWM command responds to the potentiometer | Not tested |
| MCU-06 | Test manual product selection | Appropriate servo command is issued for the selected option | Not tested |
| MCU-07 | Test conveyor clearing sequence | Conveyor continues until the IR sensor clears | Not tested |
| MCU-08 | Test mode-selection button | Single click selects colour mode; double click selects shape mode | Not tested |

**Firmware consistency check:** The supplied sketch also declares GPIO15 as a shape-button input, although the intended circuit uses one mode-selection button on GPIO12. Reconcile this before considering the physical circuit and firmware fully matched.

### D. Wi-Fi and Vision Integration

These tests are blocked until a matching HTTP handler is added to the ESP32 firmware and tested.

| Test ID | Validation item | Expected result | Status |
|---|---|---|---|
| INT-01 | Connect Python to the ESP32 endpoint | Computer reaches the ESP32 over the selected network | Blocked |
| INT-02 | Send a valid colour classification | ESP32 validates and acknowledges the result | Blocked |
| INT-03 | Send a valid shape classification | ESP32 validates and acknowledges the result | Blocked |
| INT-04 | Send an invalid classification | Firmware rejects the message without unintended motion | Blocked |
| INT-05 | Send a classification in an invalid system state | Firmware safely rejects or defers the request | Blocked |
| INT-06 | Complete a vision-driven sorting cycle | Correct classification leads to the intended physical sorting action | Blocked |

The computer must be able to access both the phone camera stream and ESP32 endpoint. Verify the chosen network arrangement before performing these tests.

## 5. End-to-End Sorting Validation

Perform this test only after individual hardware tests have passed and Wi-Fi integration is implemented.

1. Inspect the wiring, power supplies and mechanical assembly.
2. Start the ESP32 and confirm the dashboard is available.
3. Start the mobile phone IP camera and verify the live stream.
4. Run the Python vision module and check the classification output.
5. Introduce one test object onto the conveyor.
6. Confirm that the ultrasonic sensor initiates feeding.
7. Confirm that the IR sensor stops the belt at the intended position.
8. Allow the vision system to classify the object and send a result to the ESP32.
9. Confirm that the ESP32 accepts the valid result and performs the expected sorting action.
10. Verify that the servo returns to its configured home position and that the object clears the IR sensor.
11. Confirm that the system returns to idle and is ready for the next object.
12. Repeat the test for every supported colour or shape.

During early testing, use supervised, low-risk operating conditions and keep hands clear of moving components.

## 6. Performance and Reliability Record

Use this table to record measured results from repeated trials.

| Metric | Measurement method | Actual result |
|---|---|---|
| Colour classification accuracy | Correct colour classifications divided by evaluated colour trials | Not measured |
| Shape classification accuracy | Correct shape classifications divided by evaluated shape trials | Not measured |
| Successful sorting rate | Correctly sorted objects divided by completed sorting trials | Not measured |
| False sorting rate | Incorrect sorting actions divided by evaluated trials | Not measured |
| Camera stream stability | Record interruptions and failed frame reads during a defined test period | Not measured |
| Wi-Fi communication reliability | Record successful and failed classification requests | Not measured |
| Cycle time | Measure the duration from object detection to completion of the sorting cycle | Not measured |

Record the trial count, test conditions and measurement method with every reported result. Do not report an accuracy or reliability percentage without collected data.

## 7. Issues and Corrective Actions

| Issue ID | Observed issue | Corrective action | Retest status |
|---|---|---|---|
| SYS-01 | Record actual failure here | Document the fix after investigation | Not tested |
| SYS-02 | Record actual failure here | Document the fix after investigation | Not tested |
| SYS-03 | Record actual failure here | Document the fix after investigation | Not tested |

Populate this table only with issues observed during testing.

## 8. Acceptance Criteria

The system can be considered fully validated only when:

- The mechanical assembly operates reliably.
- The required sensors respond correctly.
- The conveyor and servos perform their intended actions.
- The camera stream and vision classifiers operate under documented test conditions.
- The ESP32 correctly validates Wi-Fi classification messages.
- Invalid messages and communication failures are handled safely.
- The integrated system completes repeated sorting cycles correctly.
- Test results, known limitations and corrective actions are documented.

## 9. Final Validation Summary

**Overall status: In progress — complete-system validation pending.**

The current firmware implements manual dashboard-based conveyor control, while the Python vision modules operate separately. The vision-to-ESP32 Wi-Fi endpoint and integrated automatic sorting workflow remain to be implemented and tested.

Update this summary after completing the end-to-end validation tests. Do not describe the system as fully automated or fully validated until the results support that conclusion.
