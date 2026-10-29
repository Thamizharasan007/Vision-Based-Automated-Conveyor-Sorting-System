"""
Wi-Fi communication between the Python vision system and ESP32.

Protocol:
    HTTP POST /classification

    Request body examples:
        COLOR:GREEN
        COLOR:BLUE
        SHAPE:CIRCLE
        SHAPE:SQUARE
"""

from urllib.request import Request, urlopen
from urllib.error import URLError, HTTPError


# ==========================================
# CONFIGURATION — EDIT THESE SETTINGS
# ==========================================

ESP32_IP = "192.168.1.50"  # Replace with your ESP32's actual IP
ESP32_PORT = 80

ENDPOINT = f"http://{ESP32_IP}:{ESP32_PORT}/classification"
TIMEOUT = 2

VALID_CATEGORIES = {"COLOR", "SHAPE"}

VALID_VALUES = {
    "GREEN",
    "BLUE",
    "ORANGE",
    "YELLOW",
    "RED",
    "TRIANGLE",
    "SQUARE",
    "RECTANGLE",
    "CIRCLE",
    "OTHER",
    "UNKNOWN",
}


# ==========================================
# SEND CLASSIFICATION TO ESP32
# ==========================================

def send_classification(category, value):
    """
    Send a colour or shape result to the ESP32 over Wi-Fi.

    Returns True if the ESP32 responds with a successful
    HTTP status code; otherwise returns False.
    """

    category = category.strip().upper()
    value = value.strip().upper()

    if category not in VALID_CATEGORIES:
        print("Invalid category. Use COLOR or SHAPE.")
        return False

    if value not in VALID_VALUES:
        print("Invalid classification value.")
        return False

    # Prepare the message
    message = f"{category}:{value}"

    request = Request(
        ENDPOINT,
        data=message.encode("utf-8"),
        headers={"Content-Type": "text/plain"},
        method="POST",
    )

    try:
        with urlopen(request, timeout=TIMEOUT) as response:
            if 200 <= response.status < 300:
                print(f"Sent to ESP32: {message}")
                return True

            print(f"ESP32 returned HTTP {response.status}")
            return False

    except HTTPError as error:
        print(f"ESP32 HTTP error: {error.code}")
        return False

    except (URLError, TimeoutError, OSError) as error:
        print(f"Wi-Fi communication failed: {error}")
        return False


# ==========================================
# OPTIONAL MANUAL TEST
# ==========================================

if __name__ == "__main__":
    # Run this only after the ESP32 endpoint is implemented.
    send_classification("COLOR", "GREEN")
