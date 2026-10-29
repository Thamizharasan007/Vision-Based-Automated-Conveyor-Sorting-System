# Changelog

All notable changes to this project will be documented in this file.

The project is currently under development. Features are identified according to their implementation and testing status.

## [Unreleased]

### Added
- Initial project documentation and repository structure.
- ESP32-based conveyor control using an L298N motor driver.
- IR sensor for detecting objects on the conveyor.
- Ultrasonic sensor for proximity detection.
- Two servo motors for object sorting mechanisms.
- Potentiometer-based conveyor speed adjustment.
- Physical controls for selecting colour and shape operating modes.
- Status LEDs and a buzzer for system feedback.
- Python and OpenCV-based computer vision development.
- Mobile phone camera as the video input source for the computer vision system.
- Planned communication between the Python vision application and ESP32.

### In Progress
- Integrating mobile camera video streaming with the Python/OpenCV application.
- Developing colour and shape classification.
- Establishing communication between the vision application and ESP32.
- Integrating object classification with automatic servo actuation.
- Testing the complete conveyor sorting workflow.

### Known Limitations
- The conveyor prototype uses a cardboard structure.
- Camera-based classification performance depends on lighting, camera positioning, and object appearance.
- Complete end-to-end vision-to-ESP32 sorting requires further validation.

## Version History

No official release has been published yet.
