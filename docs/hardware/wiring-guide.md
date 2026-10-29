# Hardware Wiring Guide

## 1. Overview

This guide documents the hardware connections for the Vision-Based Automated Conveyor Sorting System.

The system uses an ESP32 microcontroller to control a conveyor motor, read sensors, operate two servo motors, and select operating modes.

A mobile phone camera streams live video to a computer running Python and OpenCV. The computer vision application processes the video to identify object colours and shapes.

## 2. Components

- ESP32 development board
- L298N motor driver
- 12 V DC geared motor
- Ultrasonic distance sensor
- IR obstacle sensor
- Two servo motors
- Potentiometer
- One physical pushbutton
- Four green LEDs
- One yellow LED
- One red LED
- Passive buzzer
- Mobile phone camera
- Computer running Python and OpenCV
- Suitable power supplies and connecting wires

## 3. ESP32 and L298N Motor Driver

| ESP32 connection | L298N connection | Function |
|---|---|---|
| GPIO 25 | ENB | Motor speed control |
| GPIO 26 | IN3 | Motor direction |
| GPIO 27 | IN4 | Motor direction |
| GND | GND | Common ground |

Connect the DC motor to the appropriate motor output terminals on the L298N.

Supply the motor driver with a suitable voltage and current for the motor.

If GPIO 25 is used for PWM speed control, configure the ENB jumper appropriately according to the motor driver's board design.

**Important:** Do not power the motor from an ESP32 GPIO pin. Check the L298N module's power connections and voltage requirements before switching on the system.

## 4. Ultrasonic Sensor

| Sensor pin | ESP32 connection |
|---|---|
| TRIG | GPIO 18 |
| ECHO | GPIO 19 |
| GND | GND |
| VCC | Appropriate sensor supply |

The ultrasonic sensor measures distance to help detect nearby objects.

**Voltage protection:** If ECHO produces a 5 V signal, use a suitable voltage divider or level shifter before connecting it to GPIO 19. ESP32 GPIO inputs are not 5 V tolerant.

## 5. IR Sensor

| Sensor pin | ESP32 connection |
|---|---|
| OUT | GPIO 32 |
| GND | GND |
| VCC | Appropriate sensor supply |

The IR sensor detects an object passing through its detection area.

Confirm the sensor's output voltage and whether its output is active HIGH or active LOW before configuring the firmware.

## 6. Servo Motors

| Servo | Signal connection |
|---|---|
| Servo 1 | GPIO 33 |
| Servo 2 | GPIO 14 |

Each servo also requires power and ground connections.

- Use a suitable regulated supply for the servos.
- Connect the servo supply ground to the ESP32 ground.
- Ensure the supply can handle the servos' peak current.
- Do not power high-current servos directly from ESP32 GPIO pins.

Check the servo voltage specifications before connecting the supply.

## 7. Single-Button Mode Selection

The system uses one physical pushbutton connected to GPIO 12.

| Button action | Selected mode |
|---|---|
| Single press | Colour detection mode |
| Double press | Shape detection mode |

The firmware must distinguish a single press from a double press using button debouncing and a defined double-click interval.

### Mode indicators

| Indicator | ESP32 GPIO |
|---|---|
| Colour-mode LED | GPIO 13 |
| Shape-mode LED | GPIO 21 |

**Note:** GPIO 15 is not used as a second mode-selection button in this configuration.

GPIO 12 is an ESP32 strapping pin, so check that the button circuit does not interfere with reliable startup.

## 8. Potentiometer

| Connection | ESP32 pin |
|---|---|
| Wiper / centre terminal | GPIO 34 |
| One outer terminal | 3.3 V |
| Other outer terminal | GND |

GPIO 34 is an input-only pin and can be used to read the potentiometer's analogue voltage.

Keep the signal within the ESP32's permitted input-voltage range. Do not apply 5 V to GPIO 34.

The firmware can use the measured value to adjust the conveyor motor's PWM duty cycle.

## 9. Status LEDs and Buzzer

| Component | ESP32 GPIO |
|---|---|
| Red status LED | GPIO 4 |
| Passive buzzer | GPIO 5 |
| Green status LED | GPIO 22 |
| Yellow status LED | GPIO 23 |

Use an appropriate current-limiting resistor for each LED.

Check the passive buzzer's electrical requirements. Use a suitable driver circuit if its current demand exceeds what an ESP32 GPIO can safely supply.

The exact meaning of each status indicator should match the implemented firmware.

## 10. Mobile Camera and Computer Vision

The mobile phone camera supplies the live video used by the computer vision application.

The general workflow is:

1. Connect the mobile phone and computer using the chosen camera-streaming method.
2. Start the mobile camera streaming application.
3. Configure the video-stream address in the Python application.
4. Open the stream using OpenCV.
5. Process the frames for colour or shape classification.
6. Send the classification result to the ESP32 through the communication interface once integration is implemented.

The mobile camera connects to the computer vision application, not directly to an ESP32 GPIO pin.

The exact camera-stream URL depends on the mobile camera application and network configuration.

## 11. Pre-Power Checklist

Before powering the complete system:

- [ ] Confirm all GPIO connections against the final firmware.
- [ ] Check polarity and supply voltages.
- [ ] Verify common ground connections where required.
- [ ] Confirm motor driver wiring.
- [ ] Confirm servo power and signal connections.
- [ ] Protect the ESP32 from 5 V sensor outputs.
- [ ] Check the potentiometer's maximum output voltage.
- [ ] Confirm the single button selects modes correctly.
- [ ] Test the motor and each sensor separately.
- [ ] Keep moving parts clear of fingers, wires, and loose objects.

## 12. Testing Status

This document describes the intended wiring configuration. Confirm each connection against the actual prototype before treating this guide as a validated assembly procedure.
