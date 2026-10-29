# ESP32 Conveyor Firmware

## Overview

This firmware controls the conveyor-based sorting system using an ESP32 DevKit V1 / ESP32-WROOM-32.

The current firmware supports manual product selection through a Wi-Fi dashboard, ultrasonic object detection, IR-based conveyor stopping, potentiometer speed adjustment, two sorting servos, physical mode selection, status LEDs and a buzzer.

**Current limitation:** The Python/OpenCV vision system is not yet integrated with this firmware. Product selection currently comes from the dashboard.

## Hardware

- ESP32 DevKit V1 / ESP32-WROOM-32
- L298N motor driver
- 12 V DC conveyor motor
- HC-SR04 ultrasonic sensor
- IR obstacle sensor
- Two servo motors
- Potentiometer
- Physical mode-selection pushbutton
- Status LEDs and passive buzzer

Refer to [`docs/hardware/gpio-pin-map.md`](../../docs/hardware/gpio-pin-map.md) and [`docs/hardware/wiring-guide.md`](../../docs/hardware/wiring-guide.md) for connection details.

## Software Requirements

- Arduino IDE
- ESP32 Arduino core 3.x
- ESP32Servo library

The sketch uses the built-in `WiFi.h` and `WebServer.h` libraries.

## Installation

1. Open `esp32-conveyor.ino` in Arduino IDE.
2. Select the appropriate ESP32 board and USB port.
3. Install the ESP32Servo library if it is not already installed.
4. Verify the sketch and upload it to the ESP32.
5. Open Serial Monitor at **115200 baud**.
6. Check the startup messages for the Wi-Fi access point and dashboard address.

## Connecting to the Dashboard

The ESP32 creates its own Wi-Fi access point.

- **Network name:** `Sorting-Conveyor`
- **Password in the current sketch:** `sort1234`
- **Dashboard:** `http://192.168.4.1`

Connect the computer or phone to the ESP32's Wi-Fi network, then open the dashboard address in a browser.

Change the default password before publishing or deploying the project. Do not use the example credentials for a security-sensitive network.

## Operating Sequence

1. Power the system and connect to the ESP32 dashboard.
2. Place an object within the configured ultrasonic detection distance, initially 12 cm.
3. The conveyor starts when an object enters the configured detection range.
4. The IR sensor detects the object and stops the conveyor for selection.
5. Select a product through the dashboard.
6. The firmware commands the corresponding servo action and restarts the conveyor.
7. After the configured five-second servo hold, the servos return to their configured home position.
8. The conveyor continues until the IR sensor clears, then the system returns to idle.

The potentiometer adjusts the conveyor motor's PWM command.

## Manual Product Mapping

| Dashboard selection | Firmware action |
|---|---|
| Green / Shape A | Servo 1 commanded to 90° |
| Blue / Shape B | Servo 1 commanded to 0° |
| Orange / Shape C | Servo 2 commanded to 135° |
| Other | No servo movement |

The current sketch commands both servos to 180° in `homeServos()`. Servo angles are command values; actual mechanical movement depends on the servo installation and linkage.

## Physical Mode Selection

The mode-selection handler uses the pushbutton on GPIO12:

- Single click selects colour mode.
- Double click selects shape mode.
- GPIO13 indicates colour mode.
- GPIO21 indicates shape mode.

The current sketch also declares GPIO15 as a shape-button input and displays its state on the dashboard. Verify this against the actual one-button circuit before finalising the pin-map documentation.

## Important Configuration

The principal settings are defined near the beginning of `esp32-conveyor.ino`.

| Setting | Current value |
|---|---|
| Ultrasonic detection distance | 12 cm |
| Servo hold duration | 5000 ms |
| Servo home settling time | 400 ms |
| Motor PWM frequency | 20 kHz |
| Motor PWM resolution | 8 bits |
| Minimum motor PWM command | 95 |
| Maximum motor PWM command | 255 |
| Double-click interval | 350 ms |

Change these settings only after checking the actual hardware behaviour.

## Safety Notes

- Use a suitable external supply for the servos and DC motor.
- Ensure the ESP32, motor driver and servo supply share a common ground where required.
- Protect the ESP32 from the HC-SR04's 5 V ECHO output using an appropriate level shifter or voltage divider.
- Keep the motor supply disconnected while changing wiring.
- Test the conveyor and servos with the mechanism clear of hands and loose objects.

## Development Status

- Manual ESP32 dashboard control: implemented in the current sketch.
- Ultrasonic and IR-based conveyor sequence: implemented in the current sketch.
- Potentiometer-based speed adjustment: implemented in the current sketch.
- Physical colour/shape mode selection: implemented in the current sketch.
- Python/OpenCV integration over Wi-Fi: pending.
- Automatic vision-driven sorting: pending end-to-end integration and validation.

