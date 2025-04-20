#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TaskScheduler.h>
#include <TimeLib.h>

#include "icons.h"

const char* ssid = "ssid";
const char* password = "password";

const String apiKey = "apikey";
const String city = "city";
const String weatherURL = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&units=metric&appid=" + apiKey;

const String lastfmApiKey = "lastfmapikey";
const String username = "username";
const String lastfmURL = "http://ws.audioscrobbler.com/2.0/?method=user.getrecenttracks&user=" + username + "&api_key=" + lastfmApiKey + "&format=json";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

String temp = "--C", hum = "--%", feelsLike = "--C", weather = "Loading...";
String lastfmSong = "No song", lastfmArtist = "Unknown", timeStr, dateStr;
String weatherIcon = "01d";
String weatherDescription = "Clear";
int slideIndex = 0;
unsigned long lastScreenUpdate = 0;
int screenUpdateInterval = 30;
bool showNotification = false;
String notificationText = "";
unsigned long notificationStartTime = 0;
const unsigned long NOTIFICATION_DURATION = 3000;

bool wifiConnected = false;
int signalStrength = 0;
int batteryLevel = 100;

Scheduler ts;

void updateWeatherCallback();
void updateLastFMCallback();
void updateTimeCallback();
void updateDisplayCallback();
void nextSlideCallback();

Task updateWeatherTask(300000, TASK_FOREVER, &updateWeatherCallback);
Task updateLastFMTask(30000, TASK_FOREVER, &updateLastFMCallback);
Task updateTimeTask(1000, TASK_FOREVER, &updateTimeCallback);
Task updateDisplayTask(50, TASK_FOREVER, &updateDisplayCallback);
Task nextSlideTask(8000, TASK_FOREVER, &nextSlideCallback);

void getWeatherIconForCode(String weatherCode, String& iconType) {
    String condition = weatherCode.substring(0, 2);
    
    if (condition == "01") {
        iconType = "CLEAR";
    } else if (condition == "02" || condition == "03" || condition == "04") {
        iconType = "CLOUDY";
    } else if (condition == "09" || condition == "10") {
        iconType = "RAIN";
    } else if (condition == "11") {
        iconType = "THUNDER";
    } else if (condition == "13") {
        iconType = "SNOW";
    } else if (condition == "50") {
        iconType = "MIST";
    } else {
        iconType = "CLEAR";
    }
}

void drawWiFiIcon() {
    if (WiFi.status() == WL_CONNECTED) {
        int strength = map(WiFi.RSSI(), -100, -40, 0, 3);
        
        switch(strength) {
            case 3:
                drawIcon(display, WIFI_FULL_ICON, SCREEN_WIDTH - 18, 0);
                break;
            case 2:
                drawIcon(display, WIFI_MED_ICON, SCREEN_WIDTH - 18, 0);
                break;
            case 1:
                drawIcon(display, WIFI_LOW_ICON, SCREEN_WIDTH - 18, 0);
                break;
            default:
                drawIcon(display, WIFI_LOW_ICON, SCREEN_WIDTH - 18, 0);
        }
    } else {
        drawIcon(display, WIFI_NONE_ICON, SCREEN_WIDTH - 18, 0);
    }
}

void fetchWeather() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    http.begin(weatherURL);
    int httpCode = http.GET();
    
    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            temp = String(doc["main"]["temp"].as<float>(), 1) + "C";
            hum = String(doc["main"]["humidity"].as<int>()) + "%";
            feelsLike = String(doc["main"]["feels_like"].as<float>(), 1) + "C";
            weather = doc["weather"][0]["main"].as<String>();
            weatherDescription = doc["weather"][0]["description"].as<String>();
            weatherIcon = doc["weather"][0]["icon"].as<String>();
            
            if (weatherDescription.indexOf("rain") >= 0 || 
                weatherDescription.indexOf("snow") >= 0 || 
                weatherDescription.indexOf("storm") >= 0) {
                showNotification = true;
                notificationText = "Weather Alert: " + weatherDescription;
                notificationStartTime = millis();
            }
        }
    }
    http.end();
}

void fetchLastFM() {
    if (WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    http.begin(lastfmURL);

    int httpCode = http.GET();
    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            if (doc["recenttracks"]["track"].size() > 0) {
                String prevSong = lastfmSong;
                String fullSong = doc["recenttracks"]["track"][0]["name"] | "No song";
                String prevArtist = lastfmArtist;
                lastfmArtist = doc["recenttracks"]["track"][0]["artist"]["#text"] | "Unknown";
                
                bool isPlaying = false;
                if (doc["recenttracks"]["track"][0].containsKey("@attr")) {
                    isPlaying = doc["recenttracks"]["track"][0]["@attr"].containsKey("nowplaying");
                }
                
                if (fullSong.length() > 18) {
                    fullSong = fullSong.substring(0, 16) + "..";
                }
                
                lastfmSong = fullSong;
                
                if ((isPlaying && prevSong != fullSong) || (isPlaying && prevArtist != lastfmArtist)) {
                    showNotification = true;
                    notificationText = "Now Playing: " + fullSong;
                    notificationStartTime = millis();
                }
            }
        }
    }
    http.end();
}

void updateTime() {
    timeClient.update();
    
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    int currentYear = ptm->tm_year + 1900;
    
    char dateBuffer[12];
    sprintf(dateBuffer, "%02d/%02d/%04d", monthDay, currentMonth, currentYear);
    dateStr = String(dateBuffer);
    
    int rawHour = timeClient.getHours();
    int hour12 = rawHour % 12;
    if (hour12 == 0) hour12 = 12;
    String ampm = (rawHour >= 12) ? "PM" : "AM";
    
    char timeBuffer[12];
    sprintf(timeBuffer, "%02d:%02d %s", hour12, timeClient.getMinutes(), ampm.c_str());
    timeStr = String(timeBuffer);
}

void drawWeatherIcon(int x, int y) {
    String iconType;
    getWeatherIconForCode(weatherIcon, iconType);
    
    if (iconType == "CLEAR") {
        drawIcon(display, CLEAR_ICON, x, y);
    } else if (iconType == "CLOUDY") {
        drawIcon(display, CLOUDY_ICON, x, y);
    } else if (iconType == "RAIN") {
        drawIcon(display, RAIN_ICON, x, y);
    } else if (iconType == "THUNDER") {
        drawIcon(display, THUNDER_ICON, x, y);
    } else if (iconType == "SNOW") {
        drawIcon(display, SNOW_ICON, x, y);
    } else if (iconType == "MIST") {
        drawIcon(display, MIST_ICON, x, y);
    }
}

void drawNotification() {
    if (showNotification && (millis() - notificationStartTime < NOTIFICATION_DURATION)) {
        display.fillRect(0, 0, SCREEN_WIDTH, 8, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setTextSize(1);
        
        int textWidth = notificationText.length() * 6;
        if (textWidth <= SCREEN_WIDTH) {
            display.setCursor((SCREEN_WIDTH - textWidth) / 2, 0);
        } else {
            display.setCursor(0, 0);
            notificationText = notificationText.substring(0, 20) + "...";
        }
        display.print(notificationText);
        display.setTextColor(SSD1306_WHITE);
    } else {
        showNotification = false;
    }
}

void drawStatusBar() {
    if (!showNotification) {
        display.setTextSize(1);
        display.setCursor(2, 0);
        display.print(timeStr);
        
        drawWiFiIcon();
        
        display.drawLine(0, 8, SCREEN_WIDTH, 8, SSD1306_WHITE);
    }
}

void displayTime() {
    display.drawRoundRect(14, 16, SCREEN_WIDTH-28, 30, 3, SSD1306_WHITE);
    
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    
    int hour = timeClient.getHours() % 12;
    if (hour == 0) hour = 12;
    int minute = timeClient.getMinutes();
    
    String hourStr = hour < 10 ? " " + String(hour) : String(hour);
    String minuteStr = minute < 10 ? "0" + String(minute) : String(minute);
    String timeDisplay = hourStr + ":" + minuteStr;
    
    int timeWidth = timeDisplay.length() * 12;
    display.setCursor((SCREEN_WIDTH - timeWidth) / 2, 24);
    display.print(timeDisplay);
    
    display.setTextSize(1);
    display.setCursor(90, 32);
    display.print(timeClient.getHours() >= 12 ? "PM" : "AM");
    
    char dayOfWeek[10];
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    
    switch (ptm->tm_wday) {
        case 0: strcpy(dayOfWeek, "Sunday"); break;
        case 1: strcpy(dayOfWeek, "Monday"); break;
        case 2: strcpy(dayOfWeek, "Tuesday"); break;
        case 3: strcpy(dayOfWeek, "Wednesday"); break;
        case 4: strcpy(dayOfWeek, "Thursday"); break;
        case 5: strcpy(dayOfWeek, "Friday"); break;
        case 6: strcpy(dayOfWeek, "Saturday"); break;
    }
    
    int dateWidth = strlen(dayOfWeek) * 6;
    display.setCursor((SCREEN_WIDTH - dateWidth) / 2, 48);
    display.print(dayOfWeek);
    
    char dateText[12];
    sprintf(dateText, "%d/%d/%d", ptm->tm_mday, ptm->tm_mon + 1, ptm->tm_year % 100);
    dateWidth = strlen(dateText) * 6;
    display.setCursor((SCREEN_WIDTH - dateWidth) / 2, 56);
    display.print(dateText);
}

void displayWeather() {
    display.setTextSize(1);
    int cityWidth = city.length() * 6;
    display.setCursor((SCREEN_WIDTH - cityWidth) / 2, 10);
    display.print(city);
    display.drawLine(15, 18, SCREEN_WIDTH-15, 18, SSD1306_WHITE);
    
    drawWeatherIcon(24, 28);
    
    display.setTextSize(2);
    display.setCursor(46, 28);
    display.print(temp);
    
    display.setTextSize(1);
    int descWidth = weatherDescription.length() * 6;
    String displayDesc = weatherDescription;
    if (descWidth > SCREEN_WIDTH - 10) {
        displayDesc = weatherDescription.substring(0, 16) + "..";
        descWidth = displayDesc.length() * 6;
    }
    display.setCursor((SCREEN_WIDTH - descWidth) / 2, 48);
    display.print(displayDesc);
    
    String humAndFeels = "H:" + hum + " F:" + feelsLike;
    int infoWidth = humAndFeels.length() * 6;
    display.setCursor((SCREEN_WIDTH - infoWidth) / 2, 56);
    display.print(humAndFeels);
}

void displayLastFM() {
    drawIcon(display, MUSIC_ICON, 16, 24);
    
    display.setTextSize(1);
    display.setCursor(10, 10);
    display.print("Now Playing");
    display.drawLine(10, 18, SCREEN_WIDTH-10, 18, SSD1306_WHITE);
    
    display.setCursor(38, 24);
    String displaySong = lastfmSong;
    if (displaySong.length() > 15) {
        displaySong = displaySong.substring(0, 13) + "..";
    }
    display.print(displaySong);
    
    display.setCursor(38, 34);
    display.print("by");
    
    display.setCursor(38, 44);
    String displayArtist = lastfmArtist;
    if (displayArtist.length() > 15) {
        displayArtist = displayArtist.substring(0, 13) + "..";
    }
    display.print(displayArtist);
}

void updateWeatherCallback() {
    fetchWeather();
}

void updateLastFMCallback() {
    fetchLastFM();
}

void updateTimeCallback() {
    updateTime();
}

void updateDisplayCallback() {
    display.clearDisplay();
    
    switch (slideIndex) {
        case 0: displayTime(); break;
        case 1: displayWeather(); break;
        case 2: displayLastFM(); break;
    }
    
    drawStatusBar();
    drawNotification();
    
    display.display();
}

void nextSlideCallback() {
    slideIndex = (slideIndex + 1) % 3;
}

bool connectWiFi(int timeout) {
    WiFi.begin(ssid, password);
    int attempts = 0;
    int maxAttempts = timeout / 500;
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(10, 10);
    display.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        display.fillRect(0, 30, SCREEN_WIDTH, 10, SSD1306_BLACK);
        
        display.drawRect(10, 30, SCREEN_WIDTH-20, 10, SSD1306_WHITE);
        int progressWidth = map(attempts, 0, maxAttempts, 0, SCREEN_WIDTH-22);
        display.fillRect(11, 31, progressWidth, 8, SSD1306_WHITE);
        
        display.display();
        delay(500);
        attempts++;
    }
    
    return WiFi.status() == WL_CONNECTED;
}

void setup() {
    Serial.begin(115200);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.display();
    
    if (connectWiFi(10000)) {
        timeClient.begin();
        updateTime();
        
        fetchWeather();
        fetchLastFM();
        
        showNotification = true;
        notificationText = "Connected to " + String(ssid);
        notificationStartTime = millis();
    } else {
        display.clearDisplay();
        display.setCursor(5, 20);
        display.print("WiFi connection failed!");
        display.setCursor(5, 35);
        display.print("Check credentials.");
        display.display();
        delay(3000);
    }
    
    ts.addTask(updateWeatherTask);
    ts.addTask(updateLastFMTask);
    ts.addTask(updateTimeTask);
    ts.addTask(updateDisplayTask);
    ts.addTask(nextSlideTask);
    
    updateWeatherTask.enable();
    updateLastFMTask.enable();
    updateTimeTask.enable();
    updateDisplayTask.enable();
    nextSlideTask.enable();
}

void loop() {
    ts.execute();
}