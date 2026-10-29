# Project Diagrams

## Overview

This directory contains diagrams that explain the electrical connections and system architecture of the Vision-Based Automated Conveyor Sorting System.

The diagrams support hardware assembly, software development, testing and project documentation.

## Available Diagrams

### 1. Circuit Diagram

**File:** `circuit-diagram.png`

The circuit diagram illustrates the electrical connections between the ESP32, motor driver, sensors, servo motors, LEDs, buzzer and other hardware components.

The diagram should match the actual assembled circuit. Verify GPIO assignments, power connections and sensor voltage levels before using it as a wiring reference.

The circuit diagram image should be placed in this directory when added to the repository.

### 2. System Architecture

**File:** [`system-architecture.md`](system-architecture.md)

The system architecture document explains how the main subsystems interact:

- Mobile phone used as an IP camera.
- Computer running Python and OpenCV.
- Colour and shape detection modules.
- ESP32 controller and Wi-Fi dashboard.
- Ultrasonic and IR sensors.
- L298N motor driver and conveyor motor.
- Servo-based sorting mechanism.
- Planned Wi-Fi communication between the vision software and ESP32.

The document distinguishes the existing manual-control implementation from the planned automatic vision-driven sorting integration.

## Diagram Maintenance

When updating a diagram:

1. Verify that it matches the current hardware or software implementation.
2. Update associated GPIO and wiring documentation if connections change.
3. Use descriptive filenames and readable labels.
4. Keep the circuit diagram and software architecture consistent with the actual project.
5. Update this README when diagrams are added, replaced or renamed.

## Related Documentation

- [GPIO Pin Map](../hardware/gpio-pin-map.md)
- [Wiring Guide](../hardware/wiring-guide.md)
- [System Architecture](system-architecture.md)
- [System Test Plan](../testing/test-plan.md)

