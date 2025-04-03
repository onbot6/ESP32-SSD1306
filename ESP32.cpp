// thanks chatgpt for helping a bit
// todo
// add a working music player
// telegram bot integration
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi Credentials
const char* ssid = "ssid";
const char* password = "password";

// APIs
const String apiKey = "apikey";
const String city = "city";
const String weatherURL = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&units=metric&appid=" + apiKey;

const String lastfmApiKey = "lastfmapikey";
const String username = "username";
const String lastfmURL = "http://ws.audioscrobbler.com/2.0/?method=user.getrecenttracks&user=" + username + "&api_key=" + lastfmApiKey + "&format=json";

// Time Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// Variables
String temp = "--C", hum = "--%", feelsLike = "--°C", weather = "Loading..."; //some esps (ex, mine) does not support °, so continue without using it
String lastfmSong = "No song", lastfmArtist = "Unknown", timeStr;
int slideIndex = 0;

// Fetch Weather
void fetchWeather() {
    if (WiFi.status() != WL_CONNECTED) return;
    HTTPClient http;
    http.begin(weatherURL);
    int httpCode = http.GET();
    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        temp = String(doc["main"]["temp"].as<float>(), 1) + "°C";
        hum = String(doc["main"]["humidity"].as<int>()) + "%";
        feelsLike = String(doc["main"]["feels_like"].as<float>(), 1) + "°C";
        weather = doc["weather"][0]["main"].as<String>();
    }
    http.end();
}

// Fetch Last.fm (PURE HTTP)
void fetchLastFM() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    http.begin(lastfmURL);
    
    int httpCode = http.GET();
    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);
        
        String fullSong = doc["recenttracks"]["track"][0]["name"] | "No song";
        lastfmArtist = doc["recenttracks"]["track"][0]["artist"]["#text"] | "Unknown";

        // Truncate song name if too long
        if (fullSong.length() > 14) {
            fullSong = fullSong.substring(0, 12) + "...";  // Shorten to 12 chars + "..."
        }

        lastfmSong = fullSong;
    }
    
    http.end();
}

// get the time 
void updateTime() {
    timeClient.update();
    int rawHour = timeClient.getHours();
    int hour12 = rawHour % 12;
    if (hour12 == 0) hour12 = 12;  // Convert 0 to 12 for 12-hour format
    String ampm = (rawHour >= 12) ? "PM" : "AM";

    timeStr = String(hour12) + ":" + (timeClient.getMinutes() < 10 ? "0" : "") + String(timeClient.getMinutes()) + " " + ampm;
}

// Smooth Slide Transition (isnt smooth, it sucks)
void smoothSlide(void (*displayFunc)()) {
    for (int i = 0; i < SCREEN_WIDTH; i += 8) {
        display.clearDisplay();
        displayFunc();
        display.fillRect(i, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
        display.display();
        delay(10);
    }
}

// Display Time (Centered)
void displayTime() {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    int x = (SCREEN_WIDTH - (timeStr.length() * 12)) / 2; // Centering text
    display.setCursor(x, 28);
    display.println(timeStr);
}

// Display Weather
void displayWeather() {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 5);
    display.println(city);

    display.setTextSize(2);
    display.setCursor(5, 20);
    display.println(weather);

    display.setTextSize(1);
    display.setCursor(5, 42);
    display.println("Hum: " + hum);
    
    display.setCursor(5, 54);
    display.println("Feels: " + feelsLike);
}

// Display Last.fm
void displayLastFM() {
    display.setTextSize(1); // Smaller text size for song title
    display.setCursor(5, 5);
    display.println("Now Playing:");
    
    display.setTextSize(1); // Smaller text size for song name
    int x = (SCREEN_WIDTH - (lastfmSong.length() * 6)) / 2; // Centering text
    display.setCursor(x, 25);
    display.println(lastfmSong);
    
    display.setTextSize(1); // Author name, fit in the remaining space
    display.setCursor(5, 45);
    display.println(lastfmArtist);
}

// Setup
void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting...");
    }
    Serial.println("WiFi Connected");

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed");
        for (;;);
    }

    display.setRotation(0); // Normal orientation

    timeClient.begin();
    fetchWeather();
    fetchLastFM();
    updateTime();
}

// Loop
void loop() {
    updateTime();

    fetchWeather();
    fetchLastFM();

    switch (slideIndex) {
        case 0: smoothSlide(displayTime); break;
        case 1: smoothSlide(displayWeather); break;
        case 2: smoothSlide(displayLastFM); break;
    }

    display.display();

    slideIndex = (slideIndex + 1) % 3;
    delay(20000); //20 seconds is a hassle but i love it, change it to 10 seconds if you want to