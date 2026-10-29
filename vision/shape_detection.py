import cv2
import numpy as np
from camera_config import CAMERA_URL

# ==================================================
# 1. CONFIGURATION — EDIT THESE VALUES IF NEEDED
# ==================================================

MIN_CONTOUR_AREA = 2500
CANNY_LOW = 50
CANNY_HIGH = 150
APPROX_EPSILON = 0.04

# ==================================================
# 2. SHAPE CLASSIFICATION
# ==================================================

def detect_shape(contour):
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
        else:
            return "RECTANGLE"

    else:
        area = cv2.contourArea(contour)

        circularity = (4 * np.pi * area) / (perimeter * perimeter)

        if circularity > 0.75:
            return "CIRCLE"
        else:
            return "OTHER"


# ==================================================
# 3. OPEN MOBILE PHONE IP CAMERA
# ==================================================

capture = cv2.VideoCapture(CAMERA_URL)

if not capture.isOpened():
    raise RuntimeError(
        "Camera could not be opened. Check CAMERA_URL "
        "and ensure the phone camera stream is running."
    )


# ==================================================
# 4. PROCESS LIVE VIDEO
# ==================================================

try:
    while True:
        success, frame = capture.read()

        if not success:
            print("Failed to read camera frame.")
            break

        # Reduce image noise
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        blurred = cv2.GaussianBlur(
            gray,
            (5, 5),
            0
        )

        # Find object edges
        edges = cv2.Canny(
            blurred,
            CANNY_LOW,
            CANNY_HIGH
        )

        # Join small gaps in edges
        kernel = np.ones((3, 3), dtype=np.uint8)

        edges = cv2.morphologyEx(
            edges,
            cv2.MORPH_CLOSE,
            kernel
        )

        # Extract external contours
        contours, _ = cv2.findContours(
            edges,
            cv2.RETR_EXTERNAL,
            cv2.CHAIN_APPROX_SIMPLE
        )

        for contour in contours:

            area = cv2.contourArea(contour)

            # Ignore small contours and background noise
            if area < MIN_CONTOUR_AREA:
                continue

            shape = detect_shape(contour)

            # Find object position
            x, y, width, height = cv2.boundingRect(contour)

            # Draw contour and bounding box
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

            # Display detected shape
            cv2.putText(
                frame,
                shape,
                (x, max(y - 10, 25)),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.7,
                (0, 255, 255),
                2
            )

            print(
                f"Shape: {shape} | "
                f"Area: {int(area)} pixels"
            )

        cv2.imshow("Shape Detection", frame)
        cv2.imshow("Detected Edges", edges)

        # Press q to quit
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

finally:
    capture.release()
    cv2.destroyAllWindows()
