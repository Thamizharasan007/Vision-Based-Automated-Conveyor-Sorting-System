# Wi-Fi Communication Protocol

## 1. Overview

The vision-based conveyor sorting system uses Python and OpenCV on a computer to process video from a mobile phone acting as an IP camera. The ESP32 controls the conveyor motor, sensors, sorting servos and dashboard.

The planned communication link sends classification results from Python to the ESP32 using HTTP requests over Wi-Fi.

**Development status:** The ESP32 currently hosts a manual-control dashboard. The `/classification` endpoint described below is a proposed interface and is not implemented in the current firmware.

## 2. System Architecture

1. The mobile phone streams live video to the computer.
2. Python captures frames and detects object properties.
3. Python classifies an object's colour or shape.
4. Python sends the classification to the ESP32 using an HTTP POST request.
5. The ESP32 validates the message and selects the appropriate sorting action.
6. The conveyor and servos execute the action according to the firmware's control logic.

Steps 4–6 require implementation and end-to-end testing before automatic vision-driven sorting can be considered operational.

## 3. Network Configuration

The current ESP32 firmware creates its own Wi-Fi access point.

| Setting | Current value |
|---|---|
| Wi-Fi network | `Sorting-Conveyor` |
| ESP32 dashboard address | `http://192.168.4.1` |
| HTTP port | `80` |
| Proposed classification endpoint | `/classification` |

The mobile phone's camera stream uses a separate address, currently configured as `http://192.168.1.4:8080/video`.

The camera address and ESP32 address serve different purposes.

The computer must be able to access both the phone's video stream and the ESP32 endpoint. Depending on the network setup, this may require a shared router, multiple network interfaces or a different Wi-Fi configuration.

## 4. Proposed HTTP Interface

### Request

- **Method:** `POST`
- **Path:** `/classification`
- **Content-Type:** `text/plain`
- **Body:** One classification message

Example request body:

```text
COLOR:GREEN
```

Another example:

```text
SHAPE:CIRCLE
```

Each request should contain one classification result. The ESP32 firmware must implement the corresponding HTTP handler before these messages can be received.

### Message Format

| Message | Meaning |
|---|---|
| `COLOR:GREEN` | Green object detected |
| `COLOR:BLUE` | Blue object detected |
| `COLOR:ORANGE` | Orange object detected |
| `COLOR:YELLOW` | Yellow object detected |
| `COLOR:RED` | Red object detected, if configured |
| `SHAPE:TRIANGLE` | Triangle detected |
| `SHAPE:SQUARE` | Square detected |
| `SHAPE:RECTANGLE` | Rectangle detected |
| `SHAPE:CIRCLE` | Circle detected |
| `SHAPE:OTHER` | Shape is not one of the supported basic classes |

Only send categories that the vision module is configured to detect.

## 5. Proposed Response Handling

The ESP32 endpoint should return a suitable HTTP response after validating a request.

| HTTP status | Proposed meaning |
|---|---|
| `200 OK` | Classification accepted |
| `400 Bad Request` | Message is missing or invalid |
| `404 Not Found` | Endpoint does not exist |
| `409 Conflict` | System cannot accept a classification in its current state |
| `500 Internal Server Error` | Unexpected server-side failure |

These are proposed response conventions. The existing firmware does not yet return these responses for `/classification`.

Python should treat timeouts, connection failures and unsuccessful HTTP responses as communication failures. A failed request must not be treated as a confirmed sorting action.

## 6. Python Configuration

The proposed Python communication module is `vision/wifi_communication.py`.

Its ESP32 address must match the actual network configuration:

```python
ESP32_IP = "192.168.4.1"
ESP32_PORT = 80

ENDPOINT = (
    f"http://{ESP32_IP}:{ESP32_PORT}/classification"
)
```

`192.168.4.1` is the default access-point address used by the current firmware. Confirm the address from the ESP32 startup output before testing.

## 7. Safety and Control Requirements

Before connecting vision classification to physical sorting, the ESP32 firmware should:

- Validate every incoming classification.
- Reject unknown categories and unsupported values.
- Accept classifications only in an appropriate system state.
- Avoid repeated sorting actions caused by duplicate messages.
- Handle connection timeouts and malformed requests safely.
- Keep motor and servo control within the existing state-machine logic.
- Avoid moving the conveyor unexpectedly after an invalid request.

The ESP32 should remain responsible for safety-critical motor and servo control. Python should send classification results rather than directly controlling hardware pins.

## 8. Testing Plan

1. Confirm the ESP32 dashboard is reachable.
2. Implement the `/classification` endpoint separately from the existing dashboard routes.
3. Test valid and invalid messages without running the conveyor.
4. Confirm that Python can reach the ESP32 over the selected network.
5. Verify that communication failures are handled safely.
6. Test classification-driven sorting with the conveyor secured and clear of obstructions.
7. Record results before marking the integration complete.

## 9. Current Limitations

The current firmware supports manual product selection through the dashboard, not automatic classification requests.

The HTTP interface in this document is a design specification for future integration. The interface, supported categories, response behaviour and sorting-state checks must be implemented and verified in the firmware before the vision system can control the conveyor automatically.
