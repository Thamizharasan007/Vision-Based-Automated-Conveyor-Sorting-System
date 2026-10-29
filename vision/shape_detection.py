```python
import cv2
import numpy as np

from camera_config import CAMERA_URL
from utils import open_camera, draw_label


# ==================================================
# 1. CONFIGURATION — CHANGE THESE VALUES IF NEEDED
# ==================================================

MIN_CONTOUR_AREA = 2500

CANNY_LOW = 50
CANNY_HIGH = 150

APPROX_EPSILON = 0.04


# ==================================================
# 2. SHAPE CLASSIFICATION
# ==================================================

def detect_shape(contour):
    """Classify a contour as a basic geometric shape."""

    perimeter = cv2.arcLength(contour, True)

    if perimeter == 0:
        return "UNKNOWN"

    approximation = cv2.approxPolyDP(
        contour,
        APPROX_EPSILON * perimeter,
        True
    )

    corners = len(approximation)

    if corners == 3:
        return "TRIANGLE"

    elif corners == 4:
        x, y, width, height = cv2.boundingRect(approximation)

        ratio = width / float(height)

        if 0.90 <= ratio <= 1.10:
            return "SQUARE"

        return "RECTANGLE"

    else:
        area = cv2.contourArea(contour)

        circularity = (
            4 * np.pi * area
        ) / (perimeter * perimeter)

        if circularity > 0.75:
            return "CIRCLE"

        return "OTHER"


# ==================================================
# 3. OPEN MOBILE PHONE IP CAMERA
# ==================================================

capture = open_camera(CAMERA_URL)

print("Shape detection started. Press 'q' to exit.")


# ==================================================
# 4. LIVE SHAPE DETECTION
# ==================================================

try:
    while True:
        success, frame = capture.read()

        if not success:
            print("Failed to read camera frame.")
            break

        # Convert the image to grayscale
        gray = cv2.cvtColor(
            frame,
            cv2.COLOR_BGR2GRAY
        )

        # Reduce noise
        blurred = cv2.GaussianBlur(
            gray,
            (5, 5),
            0
        )

        # Detect edges
        edges = cv2.Canny(
            blurred,
            CANNY_LOW,
            CANNY_HIGH
        )

        # Close small gaps in object edges
        kernel = np.ones(
            (3, 3),
            dtype=np.uint8
        )

        edges = cv2.morphologyEx(
            edges,
            cv2.MORPH_CLOSE,
            kernel
        )

        # Find external contours
        contours, _ = cv2.findContours(
            edges,
            cv2.RETR_EXTERNAL,
            cv2.CHAIN_APPROX_SIMPLE
        )

        detected_shapes = []

        for contour in contours:

            area = cv2.contourArea(contour)

            # Ignore small contours
            if area < MIN_CONTOUR_AREA:
                continue

            shape = detect_shape(contour)
            detected_shapes.append(shape)

            # Find object position
            x, y, width, height = cv2.boundingRect(
                contour
            )

            # Draw the contour and bounding box
            cv2.drawContours(
                frame,
                [contour],
                -1,
                (0, 255, 0),
                2
            )

            cv2.rectangle(
                frame,
                (x, y),
                (x + width, y + height),
                (255, 0, 0),
                2
            )

            # Display shape and contour area
            draw_label(
                frame,
                f"{shape} | Area: {int(area)}",
                position=(x, max(y - 10, 25)),
                color=(0, 255, 255),
                font_scale=0.6
            )

        # Display the current result
        if detected_shapes:
            status = ", ".join(detected_shapes)
        else:
            status = "NO SHAPE DETECTED"

        draw_label(
            frame,
            f"Detected: {status}",
            position=(20, 40),
            color=(255, 255, 255)
        )

        cv2.imshow(
            "Mobile IP Camera - Shape Detection",
            frame
        )

        cv2.imshow(
            "Detected Edges",
            edges
        )

        # Press q to quit
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

finally:
    capture.release()
    cv2.destroyAllWindows()
    print("Shape detection stopped.")
```
