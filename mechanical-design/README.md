# Mechanical Design and Fabrication

## 1. Overview

The mechanical subsystem of the vision-based automated conveyor sorting system provides the structure for transporting objects and positioning them for sorting.

The prototype uses a cardboard conveyor structure, a DC motor-driven roller mechanism, and 3D-printed components for motor mounting and roller coupling.

The mechanical design supports the integration of sensors, the conveyor drive and the servo-based sorting mechanism.

## 2. Mechanical Components

| Component | Purpose | Implementation |
|---|---|---|
| Conveyor frame | Supports the conveyor mechanism | Cardboard prototype |
| DC motor bracket | Holds the conveyor drive motor in position | 3D-printed component |
| Roller coupler | Connects the motor shaft to the conveyor roller mechanism | 3D-printed component |
| Conveyor drive motor | Provides mechanical movement | 12 V Johnson DC motor |
| Sorting mechanism | Redirects selected objects | Two servo motors |
| Sensor mounting positions | Allow object detection along the conveyor | Prototype arrangement |

## 3. Design Approach

The mechanical prototype was developed around the following objectives:

- Provide a functional conveyor path for moving objects.
- Mount the DC motor securely to drive the conveyor roller.
- Use a custom motor bracket to support the motor installation.
- Use a 3D-printed coupler to transfer rotation to the roller.
- Provide suitable positions for object detection and sorting.
- Keep the structure accessible for assembly, testing and modification.

The prototype prioritises functional demonstration and integration with the electronic control system.

## 4. Fabrication and Assembly

The current prototype uses cardboard for the conveyor structure and 3D-printed components for selected mechanical interfaces.

The main assembly tasks include:

1. Constructing the conveyor frame and belt arrangement.
2. Positioning and securing the DC motor.
3. Installing the motor bracket and roller coupler.
4. Connecting the motor to the L298N motor driver.
5. Positioning the sensors near the intended detection locations.
6. Mounting the servo motors and checking the sorting mechanism's range of movement.
7. Testing the conveyor for mechanical interference and inconsistent movement.

Record actual fabrication dimensions and assembly details when available.

## 5. CAD and Engineering Drawings

The motor bracket and roller coupler are suitable candidates for CAD documentation because they are custom mechanical components.

The following items can be added to the repository as they become available:

- Native CAD files for the motor bracket.
- Native CAD files for the roller coupler.
- Exported STL files used for 3D printing.
- Engineering drawings with dimensions and tolerances.
- Photographs of fabricated parts.
- Assembly photographs and annotated views.
- A complete conveyor assembly model, if developed later.

**Current limitation:** A complete CAD assembly has not been documented as available. Do not represent a complete assembly model or engineering drawing as completed until the files exist.

## 6. Recommended Repository Structure

```text
mechanical-design/
├── README.md
├── CAD/
│   ├── motor-bracket/
│   ├── roller-coupler/
│   └── conveyor-assembly/
├── engineering-drawings/
├── renders/
├── fabrication/
└── bill-of-materials.csv
```

Add files to these directories when they are available. Empty folders may not appear on GitHub, so use a README or `.gitkeep` file when you need to preserve an otherwise empty directory.

## 7. Future Improvements

Potential improvements include:

- Developing a complete 3D CAD assembly of the conveyor.
- Replacing temporary structural materials with a more rigid frame.
- Improving motor and roller alignment.
- Adding adjustable sensor and servo mounts.
- Checking mechanical clearances and belt tracking.
- Producing dimensioned engineering drawings for fabrication.
- Documenting assembly procedures and maintenance requirements.

## 8. Validation

Mechanical validation should include:

- Checking the stability of the conveyor frame.
- Confirming that the motor bracket remains securely mounted.
- Inspecting the coupler for slipping or excessive vibration.
- Verifying that the conveyor runs without obstruction.
- Confirming that the servo mechanism moves without binding.
- Checking that objects can pass through the intended sorting area.

Record observed test results and photographs rather than claiming unverified performance.

## 9. Project Status

The current prototype includes a cardboard conveyor structure, a DC motor drive and 3D-printed motor mounting and coupling components.

Further CAD documentation, engineering drawings and mechanical refinement can be added as the design develops.
