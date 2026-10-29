```python
"""
Shared utility functions for the vision-based conveyor sorting system.

Used by the colour and shape detection modules.
"""

import cv2
import numpy as np


# ==================================================
# 1. OPEN MOBILE PHONE IP CAMERA
# ==================================================

def open_camera(camera_url):
    """Open the camera stream and return the VideoCapture object."""

    capture = cv2.VideoCapture(camera_url)

    if not capture.isOpened():
        capture.release()
        raise RuntimeError(
            f"Could not open camera stream: {camera_url}. "
            "Check the IP camera application and stream URL."
        )

    return capture


# ==================================================
# 2. CONVERT FRAME TO HSV
# ==================================================

def convert_to_hsv(frame):
    """Convert an image from BGR format to HSV."""

    return cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)


# ==================================================
# 3. CREATE A MASK FOR ONE OR MORE HSV RANGES
# ==================================================

def create_color_mask(hsv_frame, hsv_ranges):
    """
    Create a combined mask from one or more HSV ranges.

    Example:
        hsv_ranges = [
            (lower_blue, upper_blue)
        ]

    Multiple ranges can be supplied for colours such as red.
    """

    combined_mask = np.zeros(
        hsv_frame.shape[:2],
        dtype=np.uint8
    )

    for lower, upper in hsv_ranges:
        mask = cv2.inRange(hsv_frame, lower, upper)

        combined_mask = cv2.bitwise_or(
            combined_mask,
            mask
        )

    return combined_mask


# ==================================================
# 4. CLASSIFY THE DOMINANT COLOUR
# ==================================================

def classify_dominant_color(pixel_counts, min_pixels=5000):
    """
    Return the colour with the highest pixel count.

    Example:
        {"GREEN": 8000, "BLUE": 1200}
        returns "GREEN" when min_pixels is 5000.

    Returns UNKNOWN if the highest count is below the threshold.
    """

    if not pixel_counts:
        return "UNKNOWN"

    detected_color = max(
        pixel_counts,
        key=pixel_counts.get
    )

    if pixel_counts[detected_color] < min_pixels:
        return "UNKNOWN"

    return detected_color


# ==================================================
# 5. DISPLAY A TEXT LABEL ON A FRAME
# ==================================================

def draw_label(
    frame,
    text,
    position=(20, 40),
    color=(255, 255, 255),
    font_scale=0.8
):
    """Draw a readable text label on an image."""

    cv2.putText(
        frame,
        text,
        position,
        cv2.FONT_HERSHEY_SIMPLEX,
        font_scale,
        color,
        2,
        cv2.LINE_AA
    )

    return frame
```
