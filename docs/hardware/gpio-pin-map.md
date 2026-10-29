# ESP32 GPIO Pin Map

## 1. Overview

This document records the GPIO assignments for the Vision-Based Automated Conveyor Sorting System using an ESP32 DevKit V1 / ESP32-WROOM-32.

The pin assignments are based on the supplied ESP32 firmware. The intended physical mode-selection design uses one pushbutton: a single press selects colour mode, while a double press selects shape mode.

## 2. GPIO Assignment Table

| ESP32 GPIO | Component | Function |
|---|---|---|
| GPIO 4 | Red status LED | Indicates idle or waiting status |
| GPIO 5 | Passive buzzer | Audible feedback |
| GPIO 12 | Physical pushbutton | Single click: colour mode; double click: shape mode |
| GPIO 13 | Colour-mode LED | Indicates colour mode |
| GPIO 14 | Servo 2 signal | Controls the second sorting servo |
| GPIO 15 | Unused in the intended circuit | Declared as a legacy shape-button input in the supplied sketch |
| GPIO 18 | HC-SR04 TRIG | Ultrasonic trigger output |
| GPIO 19 | HC-SR04 ECHO | Ultrasonic echo input |
| GPIO 21 | Shape-mode LED | Indicates shape mode |
| GPIO 22 | Green status LED | Status indication |
| GPIO 23 | Yellow status LED | Status indication |
| GPIO 25 | L298N ENB | Motor speed control using PWM |
| GPIO 26 | L298N IN3 | Motor direction control |
| GPIO 27 | L298N IN4 | Motor direction control |
| GPIO 32 | IR sensor OUT | Object detection input |
| GPIO 33 | Servo 1 signal | Controls the first sorting servo |
| GPIO 34 | Potentiometer signal | Analog input for motor PWM adjustment |
| GPIO 35 | Unused | Reserved for possible future use |

## 3. Physical Mode Selection

The intended design uses one physical pushbutton on GPIO12.

| Button action | Selected mode | Indicator |
|---|---|---|
| Single click | Colour mode | GPIO13 HIGH, GPIO21 LOW |
| Double click | Shape mode | GPIO13 LOW, GPIO21 HIGH |

The firmware uses a debounce interval of 45 ms and a double-click window of 350 ms.

**GPIO15 clarification:** The supplied sketch declares GPIO15 as `PIN_BUTTON_SHAPE`, configures it as an input with an internal pull-up, and reports its state on the dashboard. However, the mode-selection logic uses GPIO12 for both single-click and double-click detection. GPIO15 is not used to select the operating mode in the supplied sketch, so it is documented as unused in the intended physical circuit.

## 4. Conveyor Motor Connections

The L298N motor driver interfaces with the ESP32 as follows:

| L298N connection | ESP32 connection | Purpose |
|---|---|---|
| ENB | GPIO 25 | PWM speed control |
| IN3 | GPIO 26 | Direction control |
| IN4 | GPIO 27 | Direction control |
| OUT3 / OUT4 | DC motor terminals | Motor output |
| GND | Common ground | Electrical reference |

The motor supply must be suitable for the DC motor and driver. Configure the ENB jumper appropriately for external PWM control from GPIO25.

## 5. Sensor Connections

| Sensor | ESP32 GPIO | Function |
|---|---|---|
| HC-SR04 TRIG | GPIO 18 | Starts an ultrasonic measurement |
| HC-SR04 ECHO | GPIO 19 | Receives the echo pulse |
| IR sensor OUT | GPIO 32 | Detects an object at the conveyor sensing position |
| Potentiometer wiper | GPIO 34 | Supplies the analog speed-control input |

GPIO34 and GPIO35 are input-only on the classic ESP32. GPIO34 is appropriate for reading the potentiometer signal, but it cannot be used as a digital output.

## 6. Servo Connections

| Servo | ESP32 GPIO | Firmware configuration |
|---|---|---|
| Servo 1 | GPIO 33 | 50 Hz servo signal, pulse range 500–2400 µs |
| Servo 2 | GPIO 14 | 50 Hz servo signal, pulse range 500–2400 µs |

Servo power requirements depend on the actual servo models. Use a suitable external supply where required and connect the supply ground to ESP32 ground. Do not power the servos directly from the ESP32's 3.3 V pin.

The firmware commands both servos to 180° as their configured home position. Confirm mechanical clearance before allowing full movement.

## 7. Status LEDs and Buzzer

| Component | GPIO | Function |
|---|---|---|
| Red status LED | GPIO 4 | Idle / waiting indication |
| Passive buzzer | GPIO 5 | Short audible feedback |
| Colour-mode LED | GPIO 13 | Colour-mode indication |
| Shape-mode LED | GPIO 21 | Shape-mode indication |
| Green status LED | GPIO 22 | Conveyor-running indication |
| Yellow status LED | GPIO 23 | Waiting-for-selection indication |

Use appropriate current-limiting resistors for LEDs. Confirm the buzzer's electrical requirements before connecting it directly to a GPIO.

## 8. Electrical and Boot Considerations

- **GPIO12 is a strapping pin** on the classic ESP32. Its level during reset can affect boot configuration. Test startup with the intended pushbutton wiring and avoid holding the button while powering on until correct behaviour has been verified.
- The HC-SR04 ECHO output may be 5 V. Use a suitable voltage divider or level shifter before connecting it to GPIO19.
- Ensure the ESP32, motor driver and external servo supply share a suitable common ground.
- Disconnect power before modifying connections.
- Keep motor power and signal wiring secure and separated where practical.

## 9. Firmware and Circuit Consistency

This map reflects the supplied firmware and the intended one-button physical design. GPIO15 remains declared in the current sketch, but it is not part of the intended physical mode-selection circuit.

The vision-to-ESP32 Wi-Fi classification endpoint is not yet implemented in the supplied sketch. The current firmware uses manual product selection through its dashboard.

Before making hardware changes, verify the actual assembled circuit against this map and test each input and output independently.
