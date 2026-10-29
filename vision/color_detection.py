import cv2
import numpy as np

from camera_config import CAMERA_URL
from utils import (
    open_camera,
    convert_to_hsv,
    create_color_mask,
    classify_dominant_color,
    draw_label,
)

# ==================================================
# 1. COLOUR CONFIGURATION
# ==================================================

COLOR_RANGES = {
    "GREEN": [
        (np.array([35, 50, 40]), np.array([85, 255, 255]))
    ],

    "BLUE": [
        (np.array([90, 50, 40]), np.array([130, 255, 255]))
    ],

    "ORANGE": [
        (np.array([5, 100, 100]), np.array([15, 255, 255]))
    ],

    "YELLOW": [
        (np.array([20, 100, 100]), np.array([30, 255, 255]))
    ],

    # Red needs two HSV ranges.
    "RED": [
        (np.array([0, 100, 100]), np.array([10, 255, 255])),
        (np.array([170, 100, 100]), np.array([179, 255, 255]))
    ],
}

# Select the four colours to detect.
ACTIVE_COLORS = ["GREEN", "BLUE", "ORANGE", "YELLOW"]

# Minimum matching pixels required for detection
MIN_PIXELS = 5000


# ==================================================
# 2. OPEN MOBILE PHONE IP CAMERA
# ==================================================

capture = open_camera(CAMERA_URL)
last_color = None

print("Colour detection started. Press 'q' to exit.")

# ==================================================
# 3. MAIN DETECTION LOOP
# ==================================================

try:
    while True:
        success, frame = capture.read()

        if not success:
            print("Failed to read camera frame.")
            break

        # Convert the frame from BGR to HSV
        hsv = convert_to_hsv(frame)

        # Count matching pixels for every active colour
        pixel_counts = {}

        for color in ACTIVE_COLORS:
            mask = create_color_mask(
                hsv,
                COLOR_RANGES[color]
            )

            pixel_counts[color] = cv2.countNonZero(mask)

        # Identify the colour with the highest pixel count
        detected_color = classify_dominant_color(
            pixel_counts,
            MIN_PIXELS
        )

        # Print only when the detected colour changes
        if detected_color != last_color:
            print("Detected colour:", detected_color)
            print("Pixel counts:", pixel_counts)
            last_color = detected_color

        # Display the detected colour on the video
        draw_label(
            frame,
            f"Colour: {detected_color}",
            position=(20, 40)
        )

        cv2.imshow("Mobile IP Camera - Colour Detection", frame)

        # Press q to quit
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

finally:
    capture.release()
    cv2.destroyAllWindows()
    print("Colour detection stopped.")
