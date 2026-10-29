# Vision Processing Module

## Overview

The vision module processes a live video stream from a mobile phone camera and identifies object properties for the conveyor sorting system.

Python and OpenCV are used for image acquisition and processing, while NumPy supports image and array operations.

## System Workflow

1. **Camera input:** A mobile phone streams live video to the computer over the local network.
2. **Frame acquisition:** Python captures video frames from the camera stream.
3. **Image processing:** OpenCV processes the frames to isolate relevant object features.
4. **Object classification:** The system is being developed to classify objects by colour and shape.
5. **ESP32 control:** Classification results are intended to be sent to the ESP32 so it can control the conveyor and sorting servos.

## Technologies

- Python
- OpenCV
- NumPy
- Mobile phone camera stream
- ESP32 microcontroller
- Serial communication using PySerial (planned integration)

## Development Status

- [x] Mobile phone camera video stream tested.
- [x] Initial colour detection code developed.
- [ ] Shape detection module completed and tested.
- [ ] Vision classification integrated with ESP32 firmware.
- [ ] End-to-end sorting system validated.

Update these checkboxes as each feature is verified on the physical system.

## Running the Vision Module

1. Install Python.
2. Open a terminal in the local project directory.
3. Install the dependencies:

   ```bash
   python -m pip install -r requirements.txt
   ```

4. Configure the camera stream address in the relevant Python configuration.
5. Run the vision script from the project directory.

The camera address may differ between networks. Keep local-only configuration out of public commits when appropriate.

## Planned Improvements

- Improve colour detection under different lighting conditions.
- Add robust shape classification.
- Separate camera configuration from image-processing logic.
- Add serial communication with the ESP32.
- Test classification accuracy and complete sorting cycles.

## Notes

The vision module and ESP32 firmware are separate parts of the system. Do not consider the complete vision-to-controller workflow operational until communication and physical sorting have been tested together.
