# Vision-Based Automated Conveyor Sorting System

A low-cost automated conveyor sorting prototype integrating computer vision, embedded control, and mechanical design.

The project aims to identify objects using Python and OpenCV and control an ESP32-based conveyor mechanism with servo-operated sorting gates.

> **Project status:** Prototype development and subsystem testing. Update the implementation status as each subsystem is integrated and validated.

## Project Overview

Manual product sorting can be repetitive and time-consuming. This project explores an affordable automation solution that combines image processing, sensor-based object detection, and microcontroller-controlled sorting.

The system is being developed around three main subsystems:

- **Computer Vision:** Python and OpenCV for object detection and colour/shape classification.
- **Embedded Control:** ESP32 for sensor monitoring, conveyor motor control, operating-mode selection, and servo actuation.
- **Mechanical System:** A small conveyor prototype incorporating a DC motor, rollers, and fabricated mounting components.

## Objectives

- Develop a low-cost conveyor sorting prototype.
- Explore colour and shape classification using OpenCV.
- Control a DC conveyor motor using an ESP32 and motor driver.
- Implement servo-operated sorting mechanisms.
- Integrate object-detection sensors with conveyor control.
- Document hardware connections, software architecture, testing, and mechanical components.

## System Architecture

```text
Mobile Camera
   |
   v
Python + OpenCV
   |
   v
Object Classification
   |
   v
Serial Communication
   |
   v
ESP32 Controller
   |
   +-------------------+
   |                   |
   v                   v
Motor Driver       Servo Motors
   |                   |
   v                   v
Conveyor Belt      Sorting Gates

Ultrasonic Sensor ------> ESP32
IR Sensor --------------> ESP32
Physical Controls ------> ESP32
```

*Note: The diagram represents the intended integrated system. Only describe the complete Mobile camera-to-controller workflow as operational after testing the full integration.*

## Hardware Components

| Component | Purpose |
|---|---|
| ESP32 DevKit V1 | Embedded control |
| L298N motor driver | DC motor control |
| 12 V DC motor | Conveyor drive |
| Two servo motors | Sorting gate actuation |
| IR sensor | Object detection at the sorting position |
| Ultrasonic sensor | Distance-based object detection |
| Potentiometer | Conveyor speed adjustment |
| Push button | Physical operating-mode selection |
| LEDs | Status and mode indication |
| Passive buzzer | Audible feedback |
| Mobile Camera | Input for computer vision using WiFi Connection |
| Cardboard conveyor structure | Prototype conveyor frame |
| 3D-printed motor bracket | Motor mounting |
| 3D-printed roller coupler | Motor-to-roller coupling |

## Software and Technologies

- Python
- OpenCV
- NumPy
- Arduino C++
- ESP32 Arduino framework
- ESP32Servo library
- Serial communication
- Git and GitHub

## Operating Modes

### Colour-Based Sorting

The intended vision pipeline identifies an object's colour and communicates the classification to the ESP32. The controller uses the classification to select the corresponding sorting action.

### Shape-Based Sorting

The intended shape-classification pipeline identifies an object's shape and uses the result to select a sorting action.

### Manual Hardware Testing

The ESP32 prototype also includes a web dashboard for manual product selection and testing the conveyor, sensors, and servo mechanisms independently of the camera-based classification pipeline.

## Mechanical Design

The prototype uses a cardboard conveyor structure with 3D-printed components for motor mounting and roller coupling.

The mechanical-design directory is intended to contain:

- CAD files for fabricated components.
- STEP exports for interoperability.
- Engineering drawings with dimensions.
- Assembly documentation.
- Rendering images.
- Fabrication and 3D-printing information.

Only include CAD files and drawings after creating and verifying them.

## Repository Structure

```text
vision-based-conveyor-sorting-system/
├── docs/
│   ├── diagrams/
│   ├── hardware/
│   ├── software/
│   └── testing/
├── firmware/
│   └── esp32-conveyor/
├── vision/
├── mechanical-design/
│   ├── CAD/
│   ├── engineering-drawings/
│   ├── renders/
│   └── fabrication/
├── media/
│   ├── photos/
│   └── videos/
├── tests/
├── README.md
├── LICENSE
├── CHANGELOG.md
├── requirements.txt
└── .gitignore
```

## Testing and Validation

Testing is intended to cover:

- ESP32 startup and GPIO operation.
- Physical button and mode selection.
- Servo movement and return-to-home behaviour.
- Conveyor motor speed control.
- Ultrasonic and IR sensor responses.
- OpenCV classification performance.
- Serial communication.
- Integrated sorting sequences.

Test results will be documented with observations, expected behaviour, actual behaviour, and known limitations.

## Current Limitations

- The prototype uses a cardboard conveyor structure.
- Mobile Camera-based classification and ESP32 sorting control must be validated as an integrated system.
- Classification accuracy depends on lighting, camera position, object appearance, and the image-processing configuration.
- Mechanical performance depends on conveyor alignment, motor torque, belt friction, and object positioning.

## Future Improvements

- Complete and validate the camera-to-ESP32 integration.
- Improve colour and shape classification under varying lighting conditions.
- Develop CAD models and dimensioned drawings for the mechanical assembly.
- Improve sorting repeatability and sensor reliability.
- Record quantitative testing results.
- Improve the physical conveyor structure and component mounting.

## Author

**Thamizharasan**

B.E.Mechanical and Automation Engineering

## License

This project is distributed under the license specified in the repository's `LICENSE` file.
