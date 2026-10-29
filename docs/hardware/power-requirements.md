# Power Requirements and Electrical Safety

## 1. Overview

The Vision-Based Automated Conveyor Sorting System uses an ESP32, an L298N motor driver, a DC motor, two servo motors, sensors, and status indicators.

The power system must provide suitable voltage and current for each component while protecting the ESP32 from electrical damage.

The computer vision application runs on a computer and receives video from a mobile phone camera. The computer and phone require their own suitable power sources.

## 2. Main Power Consumers

| Component | Power considerations |
|---|---|
| ESP32 development board | Requires a suitable regulated supply according to the board specifications |
| L298N motor driver | Must support the motor's supply voltage and current requirements |
| 12 V DC motor | Requires a suitable motor supply |
| Servo motors | Require a supply compatible with their rated voltage and peak current |
| Ultrasonic sensor | Supply voltage depends on the exact sensor model |
| IR sensor | Supply voltage and output levels depend on the module |
| LEDs | Require current-limiting resistors |
| Passive buzzer | Must operate within its rated voltage and current |
| Potentiometer | Its output must remain within the ESP32's permitted input-voltage range |

## 3. DC Motor Supply

The prototype uses a 12 V DC motor controlled through an L298N motor driver.

- Use a power supply suitable for the motor's rated voltage.
- Ensure the supply can provide the motor's starting and operating current.
- Verify that the L298N module's current and thermal limits are appropriate for the motor.
- Keep motor wiring secure and insulated.
- Avoid connecting the motor directly to an ESP32 GPIO pin.

The motor's actual current requirements should be checked against its datasheet or measured under suitable test conditions.

## 4. Servo Motor Supply

The system uses two servo motors for sorting operations.

Servo motors can draw substantially more current while starting, moving under load, or becoming mechanically stalled.

- Use a regulated supply compatible with the servo specifications.
- Ensure the supply can handle the combined peak current of both servos.
- Connect the servo supply ground to the ESP32 ground.
- Connect each servo's signal wire to its assigned ESP32 GPIO.
- Avoid powering high-current servos directly from the ESP32's 3.3 V output.

Do not allow the servo mechanism to remain stalled against a physical obstruction.

## 5. ESP32 Supply

Power the ESP32 using a method supported by the specific development board.

- Follow the board manufacturer's power-input specifications.
- Do not apply 5 V to a 3.3 V GPIO input.
- Check supply polarity before connecting power.
- Avoid unstable supplies that may cause unexpected resets.
- Keep motor and servo current paths from overloading the controller's supply wiring.

## 6. Common Ground

Where the control signals connect separate powered devices, their signal references must be compatible.

For the usual ESP32, L298N, and servo-control arrangement:

- Connect the ESP32 ground to the appropriate motor-driver logic ground.
- Connect the servo supply ground to the ESP32 ground.
- Connect sensor grounds to the ESP32 ground.

Use suitable wiring and power distribution to minimise voltage drops and electrical noise.

Do not connect unrelated power outputs together without checking the circuit design.

## 7. Sensor Voltage Protection

### Ultrasonic sensor

The ultrasonic sensor's ECHO output may produce a 5 V signal, depending on the model.

If the output is 5 V, use a suitable voltage divider or level shifter before connecting it to the ESP32.

### IR sensor

Check the sensor module's output voltage before connecting it to GPIO 32.

### Potentiometer

The potentiometer signal connects to GPIO 34.

Ensure the wiper voltage never exceeds the ESP32's permitted input-voltage range. GPIO 34 is input-only and does not have an internal pull-up or pull-down resistor.

## 8. Motor Driver PWM

GPIO 25 is assigned to the L298N ENB input for motor speed control.

- Configure the ENB jumper appropriately for the intended PWM connection.
- Verify that the ESP32 firmware uses a PWM configuration supported by the selected board package.
- Confirm that motor speed changes when the potentiometer setting changes.
- Check the motor driver's temperature during operation.

The motor driver and motor should remain within their rated operating limits.

## 9. Computer and Mobile Camera

The computer runs Python and OpenCV to process the live video stream from the mobile phone camera.

The computer and phone should use suitable power sources.

The mobile camera's video connection is separate from the ESP32's motor and sensor power circuitry.

## 10. Pre-Power Safety Checklist

- [ ] Confirm all supply voltages and polarities.
- [ ] Check motor and servo current requirements.
- [ ] Verify common-ground connections.
- [ ] Confirm sensor output voltages are safe for ESP32 GPIO inputs.
- [ ] Check LED current-limiting resistors.
- [ ] Inspect wires for exposed conductors or loose connections.
- [ ] Keep fingers and loose objects away from moving conveyor parts.
- [ ] Test the motor, sensors, and servos individually before integrated operation.
- [ ] Disconnect power before modifying wiring.

## 11. Design Limitations

Exact current ratings, supply capacities, and power-distribution details must be confirmed from the specifications of the actual components used in the prototype.

This document provides general design guidance and is not a substitute for validating the completed electrical circuit.

---

**Document status:** Initial power and safety documentation. Verify all component ratings before final assembly and operation.
