from urllib.request import Request, urlopen

ESP32_URL = "http://192.168.1.50/classification"

def send_classification(category, value):
    message = f"{category.upper()}:{value.upper()}\n"

    request = Request(
        ESP32_URL,
        data=message.encode("utf-8"),
        headers={"Content-Type": "text/plain"},
        method="POST"
    )

    try:
        with urlopen(request, timeout=2) as response:
            print("ESP32 response:", response.status)
            return 200 <= response.status < 300
    except Exception as error:
        print("Wi-Fi communication failed:", error)
        return False
