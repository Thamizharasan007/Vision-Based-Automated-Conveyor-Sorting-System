# Bill of Materials (BOM)

## 1. Project Overview

This document lists the main hardware components used in the Vision-Based Automated Conveyor Sorting System.

The system combines an ESP32 controller, a motor-driven conveyor, sensors, servo-based sorting mechanisms, and a computer vision application using a mobile phone camera.

## 2. Hardware Components

| S. No. | Component | Quantity | Purpose |
|---:|---|---:|---|
| 1 | ESP32 development board | 1 | Main controller for sensors, motor, servos, and mode selection |
| 2 | L298N motor driver | 1 | Controls the conveyor DC motor |
| 3 | 12 V Johnson DC motor | 1 | Drives the conveyor |
| 4 | IR obstacle sensor | 1 | Detects objects on the conveyor |
| 5 | Ultrasonic distance sensor | 1 | Measures distance for proximity detection |
| 6 | Servo motor | 2 | Operates the sorting mechanisms |
| 7 | Potentiometer | 1 | Provides a manual speed-control input |
| 8 | Physical pushbutton | 1 | Selects colour or shape mode |
| 9 | Green LEDs | 4 | Status indication |
| 10 | Yellow LED | 1 | Status indication |
| 11 | Red LED | 1 | Status indication |
| 12 | Passive buzzer | 1 | Audible system feedback |
| 13 | Breadboard / prototyping connections | As required | Supports temporary electrical connections |
| 14 | Jumper wires | As required | Connects electronic components |
| 15 | Suitable power supplies | As required | Supplies the controller, motor, and servos |
| 16 | Cardboard conveyor structure | 1 assembly | Supports the prototype conveyor |
| 17 | 3D-printed motor bracket | 1 | Mounts the motor |
| 18 | 3D-printed roller coupler | 1 | Couples the motor shaft to the conveyor roller |
| 19 | Mobile phone with camera | 1 | Provides the live video stream for computer vision |
| 20 | Computer | 1 | Runs Python, OpenCV, and the development tools |

## 3. Software Requirements

| Software / Library | Purpose |
|---|---|
| Python | Runs the computer vision application |
| OpenCV | Processes the mobile camera's live video |
| NumPy | Supports image processing and colour-mask operations |
| Arduino IDE | Develops and uploads firmware to the ESP32 |
| ESP32 board package | Enables ESP32 development in Arduino IDE |
| ESP32Servo library | Controls the servo motors |
| PySerial | Supports planned serial communication between Python and the ESP32 |
| GitHub | Hosts source code and project documentation |

## 4. Mechanical Construction

The current prototype uses a cardboard conveyor structure.

A 3D-printed motor bracket and roller coupler are used for motor mounting and mechanical coupling.

CAD models and engineering drawings can be added to the `mechanical-design/` directory if they are available.

## 5. Cost Estimation

Component prices are not included because the purchase prices and sources have not yet been documented.

To complete the cost analysis, record the actual purchase price of each component and calculate:

**Total prototype cost = Sum of the costs of all purchased components and materials.**

Reusable equipment, such as an existing mobile phone or computer, may be listed separately from newly purchased hardware.

## 6. Notes

- Verify component quantities against the assembled prototype.
- Confirm the exact sensor and servo models before publishing detailed specifications.
- Choose power supplies according to the motor and servo voltage and current requirements.
- Ensure all sensor signal voltages are compatible with the ESP32.
- Update this document whenever the hardware configuration changes.

---

**Document status:** Initial bill of materials. Quantities and component specifications should be verified against the final prototype.
