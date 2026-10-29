# Software Installation Guide

## 1. Overview

This project uses Python and OpenCV for vision processing and an ESP32 microcontroller for conveyor and servo control.

The software has two main parts:

- **Vision software:** Runs on the computer and processes the live video stream from a mobile phone used as an IP camera.
- **ESP32 firmware:** Runs on the ESP32 and controls the conveyor, sensors, servos and Wi-Fi dashboard.

The vision software and firmware are currently developed and tested as separate modules. Full vision-driven sorting over Wi-Fi remains an integration task.

## 2. Software Requirements

Install the following software:

| Software | Purpose |
|---|---|
| Python 3 | Runs the vision-processing scripts |
| Visual Studio Code | Python development and testing |
| Git | Version control and repository management |
| Arduino IDE | Compiles and uploads ESP32 firmware |
| ESP32 Arduino core 3.x | ESP32 board support |
| ESP32Servo library | Controls the sorting servos |
| OpenCV | Camera capture and image processing |
| NumPy | Image and array operations |
| PySerial | Available for future or optional serial communication |

## 3. Download the Repository

Clone the repository from GitHub:

```bash
git clone YOUR_REPOSITORY_URL
```

Replace `YOUR_REPOSITORY_URL` with your repository's actual clone URL.

Move into the project directory:

```bash
cd vision-based-conveyor-sorting-system
```

Alternatively, download the repository as a ZIP file from GitHub and extract it.

## 4. Set Up Python

Open the project folder in Visual Studio Code.

Open the integrated terminal and create a virtual environment:

```bash
python -m venv .venv
```

Activate it on Windows Command Prompt:

```bat
.venv\Scripts\activate
```

For PowerShell:

```powershell
.\.venv\Scripts\Activate.ps1
```

Install the project dependencies:

```bash
python -m pip install -r requirements.txt
```

The requirements file includes OpenCV, NumPy and PySerial.

## 5. Configure the Mobile Phone IP Camera

1. Install and open an IP camera application on your mobile phone.
2. Start its video-streaming service.
3. Connect the phone and computer so the computer can access the phone's video stream.
4. Copy the video-stream URL displayed by the application.
5. Update `CAMERA_URL` in `vision/camera_config.py`.

Example:

```python
CAMERA_URL = "http://192.168.1.4:8080/video"
```

The address above is an example from this project's development setup. Your phone may have a different IP address or streaming port.

Before running the vision scripts, verify that the stream URL is reachable from the computer.

## 6. Run Colour Detection

From the project root, run:

```bash
python vision/color_detection.py
```

The script should display the camera feed and classify the most prominent configured colour when enough matching pixels are present.

Press `q` to close the camera window.

## 7. Run Shape Detection

Run:

```bash
python vision/shape_detection.py
```

The script processes the video stream, detects contours and attempts to classify basic shapes.

Detection results depend on lighting, object size, background contrast and camera position.

Press `q` to exit.

## 8. Upload the ESP32 Firmware

1. Open Arduino IDE.
2. Open `firmware/esp32-conveyor/esp32-conveyor.ino`.
3. Select the appropriate ESP32 board and USB port.
4. Verify that the ESP32Servo library is installed.
5. Compile and upload the sketch.
6. Open Serial Monitor at 115200 baud.
7. Check the startup messages for the access-point name and dashboard address.

The current firmware creates a Wi-Fi access point:

- Network: `Sorting-Conveyor`
- Password: `sort1234`
- Dashboard: `http://192.168.4.1`

Change the example password before deployment or public demonstration.

## 9. Network Considerations

The mobile phone IP camera and ESP32 dashboard use separate IP addresses.

The current example camera stream is `192.168.1.4`, while the ESP32 access point uses `192.168.4.1`.

If the computer needs to access both devices at the same time, verify that the network arrangement permits access to both networks. Depending on the phone, computer and Wi-Fi configuration, this may require a shared router, an additional network interface or a change to the ESP32 networking configuration.

The current firmware does not yet implement the `/classification` endpoint required by the proposed `vision/wifi_communication.py` module. Do not expect automatic sorting from vision results until the endpoint and matching firmware logic have been implemented and tested.

## 10. Troubleshooting

### Camera cannot open

- Check that the phone's IP camera service is running.
- Confirm the URL and port.
- Check connectivity between the computer and phone.
- Confirm that a firewall is not blocking access.

### Python cannot import a library

Activate the virtual environment and run:

```bash
python -m pip install -r requirements.txt
```

### ESP32 does not appear in Arduino IDE

- Check the USB cable and USB port.
- Confirm the required ESP32 board package is installed.
- Check whether the operating system detects the board's serial port.

### ESP32 dashboard cannot be reached

- Connect the computer or phone to `Sorting-Conveyor`.
- Confirm that the firmware uploaded successfully.
- Open `http://192.168.4.1` in a browser.

### Vision results do not control the conveyor

The current firmware uses manual dashboard selection. Verify the Wi-Fi communication endpoint and the ESP32 message-handling logic before testing automatic control.

## 11. Safety

- Disconnect motor power before changing wiring.
- Protect ESP32 inputs from signals exceeding their permitted voltage.
- Use an appropriate power supply for the motor and servos.
- Secure the conveyor and keep hands clear of moving parts during testing.
