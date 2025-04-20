# ESP32 SSD1306 Weather & Last.fm Display

A lightweight ESP32 project that displays **real-time weather**, **now playing info from Last.fm**, and **current date & time** on a 0.96" OLED screen using the SSD1306 driver.

---

## **Features**

- **Live Weather** (Temperature, Humidity, Feels Like, Description)
- **Now Playing** from **Last.fm** (Song & Artist)
- **12-Hour Clock** with Date
- **Weather Alerts** (Rain, Snow, etc.)
- **WiFi Signal Strength Indicator**
- **Startup Progress Bar**

---

## **Hardware Required**

- **ESP32 Board** (e.g. DOIT ESP32DEVKIT1)  
- **0.96" SSD1306 OLED Display** (I2C)  
- **Jumper Wires**

---

## **Wiring**

```
ESP32 Pin  |  OLED Pin
-------------------------
3.3V       |  VCC
GND        |  GND
21 (SDA)   |  SDA
22 (SCL)   |  SCL
```

---

## **Installation**

### 1. Clone Project

```bash
git clone https://github.com/onbot6/ESP32-SSD1306.git
cd ESP32-SSD1306
```

### 2. Install Required Libraries

Install these libraries in Arduino IDE (search in Library Manager or use links):

- [WiFi (ESP32)](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi)  
- [HTTPClient](https://github.com/espressif/arduino-esp32/tree/master/libraries/HTTPClient)  
- [ArduinoJson](https://arduinojson.org/?utm_source=meta&utm_medium=library.properties)  
- [NTPClient](https://github.com/arduino-libraries/NTPClient)  
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)  
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)  
- [TimeLib](https://github.com/PaulStoffregen/Time)  
- [TaskScheduler](https://github.com/arkhipenko/TaskScheduler)

---

## **Setup WiFi & API Keys**

Edit this section in the `.ino` file:

```cpp
const char* ssid = "your_wifi_name";
const char* password = "your_password";

const String apiKey = "openweathermap_api_key";
const String city = "your_city_name";

const String lastfmApiKey = "your_lastfm_api_key";
const String username = "your_lastfm_username";
```

---

## **Importing `icons.h`**
To import icons.h
1. Click on `Sketch`
2. Click on `Add File...`
---

## **Usage**

1. Open project in **Arduino IDE**
2. Select board: **DOITESP32DEVKITV1**
3. Connect ESP32 via USB
4. Click **Upload**

---

## **APIs Used**

- [OpenWeatherMap API](https://openweathermap.org/api)
- [Last.fm API](https://www.last.fm/api)

---

## **Notes**

- Auto trims long song titles.
- Alerts for conditions like rain/snow are shown on screen.
- Display size: **128x64**, optimized for **0.96" OLED**.

---
