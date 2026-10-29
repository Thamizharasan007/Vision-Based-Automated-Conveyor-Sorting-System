import cv2
import numpy as np

# ==================================================
# 1. COLOUR CONFIGURATION — EDIT THIS SECTION
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

    # Red needs two ranges because its HSV hue
    # wraps around the beginning and end of the scale.
    "RED": [
        (np.array([0, 100, 100]), np.array([10, 255, 255])),
        (np.array([170, 100, 100]), np.array([179, 255, 255]))
    ]
}

# Choose the FOUR colours to detect.
# Replace "BLUE" with "RED" when needed.
ACTIVE_COLORS = ["GREEN", "BLUE", "ORANGE", "YELLOW"]

# Minimum number of matching pixels required for detection
MIN_PIXELS = 5000

# Camera stream URL
CAMERA_URL = "http://192.168.1.4:8080/video"


# ==================================================
# 2. OPEN CAMERA
# ==================================================

capture = cv2.VideoCapture(CAMERA_URL)

if not capture.isOpened():
    raise RuntimeError(
        "Camera could not be opened. Check the camera URL "
        "and make sure the phone and computer are connected."
    )

last_color = None


# ==================================================
# 3. MAIN DETECTION LOOP
# ==================================================

try:
    while True:
        success, frame = capture.read()

        if not success:
            print("Failed to read camera frame.")
            break

        # Convert BGR image to HSV colour space
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        pixel_counts = {}

        # Detect each active colour
        for color in ACTIVE_COLORS:
            combined_mask = np.zeros(
                hsv.shape[:2], dtype=np.uint8
            )

            # Some colours, such as red, need multiple HSV ranges
            for lower, upper in COLOR_RANGES[color]:
                mask = cv2.inRange(hsv, lower, upper)
                combined_mask = cv2.bitwise_or(
                    combined_mask, mask
                )

            pixel_counts[color] = cv2.countNonZero(
                combined_mask
            )

        # Find the colour with the highest pixel count
        detected_color = max(
            pixel_counts, key=pixel_counts.get
        )

        # Reject detections below the pixel threshold
        if pixel_counts[detected_color] < MIN_PIXELS:
            detected_color = "UNKNOWN"

        # Print only when the detected colour changes
        if detected_color != last_color:
            print("Detected colour:", detected_color)
            print("Pixel counts:", pixel_counts)
            last_color = detected_color

        # Show the detection result on the video
        cv2.putText(
            frame,
            f"Colour: {detected_color}",
            (20, 40),
            cv2.FONT_HERSHEY_SIMPLEX,
            1,
            (255, 255, 255),
            2
        )

        cv2.imshow("Colour Detection", frame)

        # Press q to exit
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

finally:
    capture.release()
    cv2.destroyAllWindows()
