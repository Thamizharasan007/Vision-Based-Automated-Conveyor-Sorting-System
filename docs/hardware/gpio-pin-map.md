# ESP32 GPIO Pin Map

## 1. Overview

This document describes the GPIO assignments for the ESP32-based vision-guided conveyor sorting system.

The ESP32 controls the conveyor motor, reads sensor inputs, operates the sorting servos, and manages the physical mode-selection controls and status indicators.

The computer vision application runs separately on a computer using Python and OpenCV. A mobile phone camera provides the live video stream to the computer.

## 2. Hardware Connections

| Component | ESP32 GPIO | Function |
|---|---:|---|
| Red status LED | GPIO 4 | Red status indication |
| Passive buzzer | GPIO 5 | Audible feedback |
|Mode-selection button | GPIO 12 | Single press selects colour mode; double press selects shape mode|
| Colour-mode LED | GPIO 13 | Colour-mode indication |
| Servo 2 | GPIO 14 | Sorting mechanism actuation |
| Ultrasonic TRIG | GPIO 18 | Ultrasonic trigger signal |
| Ultrasonic ECHO | GPIO 19 | Ultrasonic echo input |
| Shape-mode LED | GPIO 21 | Shape-mode indication |
| Green status LED | GPIO 22 | Green status indication |
| Yellow status LED | GPIO 23 | Yellow status indication |
| L298N ENB | GPIO 25 | Motor enable and speed control |
| L298N IN3 | GPIO 26 | Motor direction control |
| L298N IN4 | GPIO 27 | Motor direction control |
| IR sensor OUT | GPIO 32 | Object detection input |
| Servo 1 | GPIO 33 | Sorting mechanism actuation |
| Potentiometer signal | GPIO 34 | Conveyor speed adjustment |
| Unused | GPIO 35 | Reserved |

**Important:** Verify this table against the latest ESP32 firmware and actual wiring before using it as a wiring reference.

## 3. Motor Driver

The L298N motor driver controls the conveyor's DC motor.

- GPIO 25 connects to ENB for motor enable and speed control.
- GPIO 26 connects to IN3 for motor direction control.
- GPIO 27 connects to IN4 for motor direction control.

The ENB jumper configuration must be appropriate for GPIO-controlled speed adjustment.

## 4. Sensors

### Ultrasonic sensor

- TRIG connects to GPIO 18.
- ECHO connects to GPIO 19.

**Electrical safety:** If the ultrasonic sensor's ECHO output is 5 V, use a suitable voltage divider or level shifter before connecting it to the ESP32's 3.3 V GPIO input.

### IR sensor

- OUT connects to GPIO 32.
- The sensor detects objects passing through its detection area.

Verify that the sensor output voltage is compatible with the ESP32.

## 5. Servo Motors

- Servo 1 signal connects to GPIO 33.
- Servo 2 signal connects to GPIO 14.

Use a suitable external power supply for the servos when required. Connect the external supply ground to the ESP32 ground.

Do not power high-current servos directly from an ESP32 GPIO pin.

## 6. Physical Controls

The system uses a single physical pushbutton connected to GPIO 12 to select between colour and shape detection modes.

| User action | Operation |
|---|---|
| Single press | Select colour detection mode |
| Double press | Select shape detection mode |

- GPIO 12: Mode-selection button input.
- GPIO 13: Colour-mode status LED.
- GPIO 21: Shape-mode status LED.
- GPIO 34: Potentiometer input for conveyor speed adjustment.

The firmware uses button debouncing and a double-click detection interval to distinguish a single press from a double press. The selected mode is indicated by the corresponding mode LED.

**Hardware note:** GPIO 12 is an ESP32 strapping pin. Its electrical level during startup can affect boot behaviour, so verify reliable startup with the button circuit connected.
## 7. Computer Vision Connection

The mobile phone camera streams video to the computer running Python and OpenCV.

The computer processes the video to identify object properties such as colour and shape. The classification result can then be sent to the ESP32 through a communication interface.

The camera stream and the Python-to-ESP32 communication are separate from the GPIO connections documented here.

## 8. Validation Checklist

Before operating the complete system, verify:

- [ ] GPIO assignments match the firmware.
- [ ] All grounds are connected appropriately.
- [ ] The motor driver receives the correct supply voltage.
- [ ] Servo power is adequate.
- [ ] Ultrasonic ECHO voltage is ESP32-compatible.
- [ ] The potentiometer output stays within the permitted ADC input range.
- [ ] Physical buttons do not interfere with ESP32 startup.
- [ ] Sensors and motor controls work individually before integration.

---

**Document status:** Initial documentation; verify against the final hardware configuration.
