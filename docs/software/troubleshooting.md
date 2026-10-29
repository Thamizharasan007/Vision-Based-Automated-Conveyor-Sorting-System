# Troubleshooting Guide

## Overview

This guide covers common issues encountered while developing and testing the vision-based conveyor sorting system.

The system contains two main software components: the Python/OpenCV vision module running on a computer and the ESP32 firmware controlling the conveyor hardware.

## 1. Mobile Phone IP Camera Problems

### Camera stream does not open

**Symptoms**
- OpenCV reports that the camera could not be opened.
- The video window does not appear.
- The program cannot read camera frames.

**Checks**
1. Confirm that the IP camera application is running on the phone.
2. Confirm that video streaming is enabled.
3. Check the stream URL in `vision/camera_config.py`.
4. Verify that the computer can access the phone's camera stream.
5. Check whether the phone or computer has changed IP addresses.
6. Check firewall settings and local network connectivity.

Example camera URL:

```python
CAMERA_URL = "http://192.168.1.4:8080/video"
```

This is the development URL, not a permanent address. Replace it when your camera stream uses a different address or port.

### Video freezes or frames cannot be read

**Possible causes**
- Weak Wi-Fi signal.
- Phone camera application interruptions.
- Network congestion.
- An unavailable or expired stream.

**Recommended actions**
- Keep the phone near the Wi-Fi access point.
- Restart the camera stream.
- Verify the URL in a browser where supported.
- Reduce the camera stream resolution or frame rate if the application allows it.

## 2. Python and OpenCV Problems

### Python reports a missing module

Example:

```text
ModuleNotFoundError: No module named 'cv2'
```

Install the project dependencies from the repository root:

```bash
python -m pip install -r requirements.txt
```

If necessary, activate the project's virtual environment before installing the dependencies.

### Incorrect colour classification

**Possible causes**
- Lighting conditions differ from those used when setting the HSV ranges.
- The object is too small or too far from the camera.
- The background contains similar colours.
- The HSV threshold ranges are too broad or too narrow.

**Recommended actions**
- Use consistent lighting.
- Place the object against a contrasting background.
- Adjust the HSV limits in `vision/color_detection.py`.
- Tune `MIN_PIXELS` to suit the object size and camera distance.
- Test each colour separately before running the conveyor.

### Incorrect shape classification

**Possible causes**
- Blurred edges.
- Poor contrast between the object and its background.
- Overlapping objects.
- Incorrect contour area or approximation settings.

**Recommended actions**
- Improve lighting and background contrast.
- Adjust `MIN_CONTOUR_AREA`.
- Tune the Canny edge thresholds and contour approximation parameter.
- Test one object at a time.

The current shape detector uses contours and geometric approximations. Its output should be treated as a basic classification result rather than a guarantee of correct identification.

## 3. ESP32 Upload Problems

### ESP32 does not appear in Arduino IDE

**Checks**
1. Verify that the USB cable supports data transfer.
2. Try another USB port or cable.
3. Check whether your operating system detects a serial port.
4. Confirm that the ESP32 board package is installed.
5. Select the correct board and port in Arduino IDE.

### Compilation fails because a library is missing

The firmware requires the ESP32 Arduino core 3.x and the ESP32Servo library.

Install the ESP32Servo library through Arduino IDE's Library Manager, then compile again.

### Flash connection or upload errors

**Recommended actions**
- Disconnect and reconnect the board.
- Close applications using the ESP32's serial port.
- Check the selected board and port.
- Follow the upload instructions for the specific ESP32 development board.

Do not change wiring while the system is powered.

## 4. ESP32 Wi-Fi Dashboard Problems

### The dashboard does not open

The current firmware creates its own Wi-Fi access point.

| Setting | Current value |
|---|---|
| Network name | `Sorting-Conveyor` |
| Password in sketch | `sort1234` |
| Dashboard address | `http://192.168.4.1` |

**Checks**
1. Confirm that the ESP32 has powered on and completed startup.
2. Connect the computer or phone to the `Sorting-Conveyor` network.
3. Open the dashboard address in a browser.
4. Read the Arduino Serial Monitor at 115200 baud for startup messages.

If you change the access-point configuration, update the relevant documentation.

**Security note:** The password shown above is embedded in the current sketch. Replace it before deployment and avoid publishing real credentials in a public repository. The dashboard should also be protected appropriately if the system is used beyond a controlled demonstration.

### The dashboard opens, but product selection fails

The current firmware accepts a product selection only when the system is in `WAITING_FOR_SELECTION`.

Confirm that the IR sensor has detected the object and stopped the conveyor before selecting a product. The dashboard may return an HTTP 409 response if the conveyor is not waiting for selection.

## 5. Conveyor Motor Problems

### Motor does not start

**Checks**
1. Verify the motor supply and common ground connections.
2. Check L298N connections to the motor and ESP32.
3. Verify that the ENB jumper configuration permits PWM control from GPIO25.
4. Inspect the IN3 and IN4 connections to GPIO26 and GPIO27.
5. Confirm that the firmware has entered the conveyor-feeding state.
6. Check the potentiometer reading and minimum PWM setting.

The current firmware sets the minimum motor PWM command to 95 out of 255. This is a control setting, not a guarantee that every motor will start at that value.

### Motor hums but does not rotate

**Possible causes**
- PWM command is too low.
- The conveyor is mechanically obstructed.
- The motor supply cannot deliver sufficient current.
- Wiring is incorrect or loose.

Disconnect power before checking the mechanics. Increase the minimum PWM setting only after checking for obstructions, wiring and power-supply limitations.

### Motor does not stop when expected

Check the ultrasonic and IR sensor readings on the dashboard and inspect the corresponding state transitions in the firmware. Verify whether the IR sensor is active-low or active-high and adjust `IR_ACTIVE_LOW` if required by the actual sensor.

## 6. Sensor Problems

### Ultrasonic sensor reports no echo

**Checks**
- Verify the TRIG and ECHO wiring.
- Confirm the sensor has suitable power.
- Ensure the object is within the sensor's useful measurement range.
- Check whether the sensor is facing the object correctly.
- Inspect the distance displayed on the dashboard.

**Electrical caution:** The HC-SR04 commonly uses a 5 V ECHO signal. Use an appropriate voltage divider or level shifter before connecting it to ESP32 GPIO19.

### IR sensor does not detect the object

**Checks**
- Verify sensor power, ground and output wiring.
- Confirm the output is connected to GPIO32.
- Check whether the sensor's indicator responds to the object.
- Verify the `IR_ACTIVE_LOW` setting.
- Adjust sensor sensitivity if the hardware supports it.

## 7. Servo Problems

### Servo does not move

**Checks**
1. Confirm the signal assignments: Servo 1 to GPIO33 and Servo 2 to GPIO14.
2. Verify signal, power and ground connections.
3. Confirm that both servos are attached successfully in the Serial Monitor.
4. Check that the external servo supply can handle the required current.
5. Ensure the mechanism is not obstructing servo movement.

Do not power the servos directly from the ESP32's 3.3 V pin. Use a suitable supply and ensure a common ground.

### Servo moves to the wrong position

The current firmware uses these commands:

| Selection | Command |
|---|---|
| Green / Shape A | Servo 1 to 90° |
| Blue / Shape B | Servo 1 to 0° |
| Orange / Shape C | Servo 2 to 135° |
| Home position | Both servos to 180° |

The physical sorting direction depends on the way each servo and linkage is installed. Calibrate the positions with the conveyor safely stopped and ensure the linkage does not bind at either end.

## 8. Physical Mode-Selection Button

The intended hardware design uses one physical pushbutton on GPIO12:

- Single press: colour mode.
- Double press: shape mode.
- GPIO13: colour-mode indicator.
- GPIO21: shape-mode indicator.

**Firmware verification required:** The currently documented firmware also configures GPIO15 as a second shape-button input and exposes its state on the dashboard. If your physical circuit has only one button, reconcile the firmware's GPIO15 logic before treating the firmware and circuit diagram as fully matched.

GPIO12 is an ESP32 strapping pin. Test startup behaviour carefully, particularly if the button is held during reset.

## 9. Wi-Fi Vision Integration Problems

The Python vision system and ESP32 dashboard are currently separate modules.

The proposed `vision/wifi_communication.py` sends HTTP POST requests to:

```text
/classification
```

The current ESP32 sketch does not implement this endpoint. Therefore, classification messages cannot yet trigger automatic sorting through that module.

Before testing vision-driven conveyor control:

1. Add a matching HTTP handler to the ESP32 firmware.
2. Validate the message format and allowed classification values.
3. Test communication independently from motor control.
4. Confirm that the firmware handles lost or invalid messages safely.
5. Test complete sorting cycles with the mechanism clear of obstructions.

The phone camera stream address and the ESP32 dashboard address are separate. Ensure the computer's network arrangement allows it to reach both devices.

## 10. General Safety

- Disconnect motor power before changing electrical connections.
- Protect ESP32 GPIOs from excessive input voltage.
- Use suitable power supplies for the motor and servos.
- Secure the conveyor and keep hands clear of moving parts.
- Test software changes with the motor disabled when possible.
- Verify each hardware function individually before running a full sorting cycle.
