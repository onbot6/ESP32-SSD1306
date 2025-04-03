# ESP32 SSD1306 Weather & Last.fm Display  

A lightweight ESP32 project that displays real-time **weather updates, time, and the latest song from Last.fm** on an SSD1306 OLED display. The display **automatically cycles through different screens every 20 seconds**.  

## Features  
- **Live Weather Data** (Temperature, Humidity, Feels Like)  
- **Last.fm Now Playing** (Latest song & artist)

## Hardware Requirements  
- **ESP32** (e.g., DOIT ESP32DEVKIT1)  
- **0.96" SSD1306 OLED Display** (I2C)  
- **Jumper wires**

## Wiring (ESP32 → OLED)  
```plaintext
ESP32 Pin  |  OLED Pin  
-------------------------
3.3V       |  VCC  
GND        |  GND  
21 (SDA)   |  SDA  
22 (SCL)   |  SCL  
```

## Installation & Setup  
### 1. Clone the Repository  
```sh
git clone https://github.com/onbot6/ESP32-SSD1306.git
cd ESP32-SSD1306
```

### 2. Install Dependencies  
Ensure you have these libraries installed in the **Arduino IDE**:  
- [WiFi](https://github.com/espressif/arduino-esp32)  
- [HTTPClient](https://github.com/espressif/arduino-esp32)  
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)  
- [NTPClient](https://github.com/arduino-libraries/NTPClient)  
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)  
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)  

### 3. Update Credentials  
Edit in the code:  
```cpp
const char* ssid = "wifi";
const char* password = "password";
const String apiKey = "apikey";
const String city = "city";
const String lastfmApiKey = "apikey";
const String username = "username";
```

### 4. Compile & Upload  
- Select **ESP32 Board** in the **Arduino IDE**  
- Connect your ESP32  
- Click **Upload**  

## TODO  
- Add some more features 
- Telegram integration
