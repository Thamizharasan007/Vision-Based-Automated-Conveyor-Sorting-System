"""ESP32-triggered vision controller for the conveyor sorting project.

Run from the repository root:
    python vision/wifi_communication.py

The program polls the ESP32 continuously, but opens/reads the camera and runs
classification only while an IR-triggered vision request is active.
"""

from __future__ import annotations

import json
import time
import urllib.error
import urllib.parse
import urllib.request
from collections import Counter
from typing import Optional

import cv2
import numpy as np

# ---------------------------------------------------------------------------
# USER CONFIGURATION
# ---------------------------------------------------------------------------
ESP32_BASE_URL = "http://192.168.4.1"

# IMPORTANT: when the phone joins the ESP32's Sorting-Conveyor Wi-Fi,
# its camera-stream IP may change. Update this URL to the address shown by
# your phone IP-camera app. Do not assume the old 192.168.1.4 address remains.
CAMERA_URL = "http://192.168.4.2:8080/video"

POLL_INTERVAL_SECONDS = 0.25
HTTP_TIMEOUT_SECONDS = 3.0
FRAME_COUNT = 9
FRAME_INTERVAL_SECONDS = 0.12
MIN_VALID_VOTES = 4
MIN_COLOUR_PIXELS = 5000
MIN_CONTOUR_AREA = 2500
SHOW_PREVIEW = True

# Keep only the colour classes the ESP32 currently accepts.
# To change a colour, edit its HSV lower/upper arrays here.
COLOUR_RANGES = {
    "GREEN": (np.array([35, 50, 40]), np.array([85, 255, 255])),
    "BLUE": (np.array([90, 50, 40]), np.array([130, 255, 255])),
    "ORANGE": (np.array([5, 100, 100]), np.array([15, 255, 255])),
    "YELLOW": (np.array([20, 100, 100]), np.array([30, 255, 255])),
}


class CameraUnavailable(RuntimeError):
    """Raised when the configured phone camera stream cannot be read."""


def http_get_json(path: str) -> dict:
    url = ESP32_BASE_URL.rstrip("/") + path
    request = urllib.request.Request(url, method="GET")
    with urllib.request.urlopen(request, timeout=HTTP_TIMEOUT_SECONDS) as response:
        return json.loads(response.read().decode("utf-8"))


def submit_result(request_id: int, mode: str, result: str) -> str:
    """Send a form-encoded result matching the ESP32 WebServer API."""
    url = ESP32_BASE_URL.rstrip("/") + "/classification"
    payload = urllib.parse.urlencode(
        {"requestId": str(request_id), "mode": mode, "result": result}
    ).encode("ascii")
    request = urllib.request.Request(
        url,
        data=payload,
        headers={"Content-Type": "application/x-www-form-urlencoded"},
        method="POST",
    )
    with urllib.request.urlopen(request, timeout=HTTP_TIMEOUT_SECONDS) as response:
        return response.read().decode("utf-8", errors="replace")


def open_camera() -> cv2.VideoCapture:
    print(f"[CAMERA] Connecting to {CAMERA_URL}")
    cap = cv2.VideoCapture(CAMERA_URL)
    try:
        cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
    except Exception:
        pass

    if not cap.isOpened():
        cap.release()
        raise CameraUnavailable(
            "Could not open the camera stream. Check CAMERA_URL and confirm the "
            "phone and PC are connected to the Sorting-Conveyor Wi-Fi."
        )

    # Discard a few initial frames so classification is less likely to use a
    # stale buffered image when the camera stream first opens.
    for _ in range(3):
        cap.read()
    return cap


def classify_colour(frame: np.ndarray) -> str:
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    counts: dict[str, int] = {}

    for colour, (lower, upper) in COLOUR_RANGES.items():
        mask = cv2.inRange(hsv, lower, upper)
        counts[colour] = int(cv2.countNonZero(mask))

    if not counts:
        return "UNKNOWN"

    best_colour = max(counts, key=counts.get)
    if counts[best_colour] < MIN_COLOUR_PIXELS:
        return "UNKNOWN"
    return best_colour


def analyze_shape(frame: np.ndarray):
    """Find the main foreground contour and classify its geometry.

    Uses Otsu thresholding in both polarities instead of relying only on Canny
    edge contours. This is generally more reliable for solid objects placed on
    a contrasting conveyor background.

    Returns (label, annotated_frame, selected_binary_mask, diagnostic_text).
    """
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    frame_h, frame_w = gray.shape[:2]
    frame_area = float(frame_h * frame_w)
    min_area = max(float(MIN_CONTOUR_AREA), frame_area * 0.003)
    max_area = frame_area * 0.65

    kernel = np.ones((5, 5), dtype=np.uint8)
    candidates = []
    masks = []

    # Try both bright-object and dark-object foreground polarity.
    for threshold_type in (cv2.THRESH_BINARY, cv2.THRESH_BINARY_INV):
        _, mask = cv2.threshold(
            blurred, 0, 255, threshold_type + cv2.THRESH_OTSU
        )
        mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel, iterations=1)
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel, iterations=2)
        masks.append(mask)

        contours, _ = cv2.findContours(
            mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
        )
        for contour in contours:
            area = cv2.contourArea(contour)
            if area < min_area or area > max_area:
                continue

            x, y, w, h = cv2.boundingRect(contour)
            # Ignore objects touching the image boundary: these are often
            # conveyor/background regions rather than the item being sorted.
            margin = 2
            if x <= margin or y <= margin or x + w >= frame_w - margin or y + h >= frame_h - margin:
                continue

            perimeter = cv2.arcLength(contour, True)
            if perimeter <= 0:
                continue

            approximation = cv2.approxPolyDP(contour, 0.025 * perimeter, True)
            corners = len(approximation)
            circularity = (4.0 * np.pi * area) / (perimeter * perimeter)

            if corners == 3:
                label = "TRIANGLE"
                ratio = 0.0
            elif corners == 4:
                rect = cv2.minAreaRect(contour)
                side_a, side_b = rect[1]
                if min(side_a, side_b) <= 0:
                    continue
                ratio = max(side_a, side_b) / min(side_a, side_b)
                label = "SQUARE" if ratio <= 1.15 else "RECTANGLE"
            elif circularity >= 0.75:
                label = "CIRCLE"
                ratio = 0.0
            elif corners >= 5:
                label = "OTHER"
                ratio = 0.0
            else:
                label = "UNKNOWN"
                ratio = 0.0

            candidates.append({
                "area": area,
                "contour": contour,
                "label": label,
                "corners": corners,
                "circularity": circularity,
                "ratio": ratio,
                "mask": mask,
            })

    annotated = frame.copy()
    if not candidates:
        return "UNKNOWN", annotated, np.zeros_like(gray), "no suitable foreground contour"

    # The largest non-border foreground object is assumed to be the item on
    # the conveyor. If the camera sees several objects, keep only the largest.
    best = max(candidates, key=lambda item: item["area"])
    cv2.drawContours(annotated, [best["contour"]], -1, (0, 255, 0), 3)
    x, y, w, h = cv2.boundingRect(best["contour"])
    cv2.rectangle(annotated, (x, y), (x + w, y + h), (255, 180, 0), 2)
    diagnostic = (
        f"area={best['area']:.0f}, corners={best['corners']}, "
        f"circularity={best['circularity']:.2f}, ratio={best['ratio']:.2f}"
    )
    cv2.putText(
        annotated, f"{best['label']} | {diagnostic}", (12, 28),
        cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 255, 0), 2, cv2.LINE_AA
    )
    return best["label"], annotated, best["mask"], diagnostic


def classify_shape(frame: np.ndarray) -> str:
    """Compatibility wrapper for shape-only callers."""
    label, _annotated, _mask, _diagnostic = analyze_shape(frame)
    return label


def collect_classification(mode: str) -> str:
    """Classify several frames and return a majority-vote label."""
    cap = open_camera()
    votes: list[str] = []
    window_name = "Conveyor vision - " + mode

    try:
        for frame_index in range(FRAME_COUNT):
            ok, frame = cap.read()
            if not ok or frame is None:
                time.sleep(FRAME_INTERVAL_SECONDS)
                continue

            shape_preview = None
            shape_mask = None
            if mode == "COLOUR":
                label = classify_colour(frame)
            elif mode == "SHAPE":
                label, shape_preview, shape_mask, diagnostic = analyze_shape(frame)
                print(f"[SHAPE] Frame {frame_index + 1}: {label} ({diagnostic})")
            else:
                raise ValueError(f"Unsupported ESP32 mode: {mode}")

            votes.append(label)

            if SHOW_PREVIEW:
                preview = shape_preview if mode == "SHAPE" else frame.copy()
                cv2.putText(
                    preview,
                    f"{mode}: {label} ({frame_index + 1}/{FRAME_COUNT})",
                    (15, 30),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.75,
                    (0, 255, 0),
                    2,
                    cv2.LINE_AA,
                )
                cv2.imshow(window_name, preview)
                if mode == "SHAPE" and shape_mask is not None:
                    cv2.imshow("Shape threshold mask", shape_mask)
                cv2.waitKey(1)

            time.sleep(FRAME_INTERVAL_SECONDS)
    finally:
        cap.release()
        if SHOW_PREVIEW:
            try:
                cv2.destroyWindow(window_name)
                if mode == "SHAPE":
                    cv2.destroyWindow("Shape threshold mask")
                cv2.waitKey(1)
            except cv2.error:
                pass

    valid_votes = [vote for vote in votes if vote != "UNKNOWN"]
    if not valid_votes:
        return "UNKNOWN"

    counts = Counter(valid_votes)
    result, number_of_votes = counts.most_common(1)[0]
    if number_of_votes < MIN_VALID_VOTES:
        print(f"[VISION] Weak/noisy detection: {dict(counts)} -> UNKNOWN")
        return "UNKNOWN"

    print(f"[VISION] Votes: {dict(counts)} -> {result}")
    return result


def main() -> None:
    print("ESP32-triggered conveyor vision controller")
    print(f"ESP32: {ESP32_BASE_URL}")
    print("Waiting for an IR-triggered vision request...\n")

    last_seen_request_id: Optional[int] = None

    while True:
        try:
            request_data = http_get_json("/vision-request")
            active = bool(request_data.get("active", False))
            request_id = int(request_data.get("requestId", 0))
            mode = str(request_data.get("mode", "")).strip().upper()

            if not active:
                time.sleep(POLL_INTERVAL_SECONDS)
                continue

            if request_id == last_seen_request_id:
                # Avoid running the same request repeatedly if the ESP32 has
                # not yet changed state after a response or network delay.
                time.sleep(POLL_INTERVAL_SECONDS)
                continue

            if mode not in {"COLOUR", "SHAPE"}:
                print(f"[ERROR] ESP32 sent an unknown mode: {mode!r}")
                time.sleep(1.0)
                continue

            print(f"\n[REQUEST] ID={request_id} | selected mode={mode}")
            result = collect_classification(mode)
            print(f"[RESULT] {result}; sending to ESP32...")

            try:
                response_text = submit_result(request_id, mode, result)
                print(f"[ESP32] {response_text}")
                last_seen_request_id = request_id
            except urllib.error.HTTPError as exc:
                detail = exc.read().decode("utf-8", errors="replace")
                if exc.code == 409:
                    # A 409 may mean the ESP32 has already accepted this
                    # request (for example, if the reply was lost).
                    print(f"[ESP32] Request no longer active: {detail}")
                    last_seen_request_id = request_id
                else:
                    print(f"[HTTP ERROR] {exc.code}: {detail}")
                    time.sleep(1.0)
            except (urllib.error.URLError, TimeoutError) as exc:
                print(f"[NETWORK ERROR] Could not submit result: {exc}")
                time.sleep(1.0)

        except CameraUnavailable as exc:
            # Do not submit a false UNKNOWN result just because the camera is
            # unreachable. The ESP32 stays safely stopped awaiting a result.
            print(f"[CAMERA ERROR] {exc}")
            time.sleep(2.0)
        except (urllib.error.URLError, TimeoutError, json.JSONDecodeError) as exc:
            print(f"[ESP32 CONNECTION ERROR] {exc}")
            time.sleep(2.0)
        except KeyboardInterrupt:
            print("\nStopping vision controller.")
            break
        except Exception as exc:
            print(f"[ERROR] {type(exc).__name__}: {exc}")
            time.sleep(1.0)

    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
