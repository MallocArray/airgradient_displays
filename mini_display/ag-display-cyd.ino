/*
AirGradient Display for ESP32 Cheap Yellow Display (CYD)

This code adapts the AirGradient DIY Mini Display to run on ESP32-based CYD hardware.
It displays indoor and outdoor air quality data from the AirGradient platform.

Hardware: ESP32 Cheap Yellow Display (CYD)
- 320x240 ILI9341 or ST7789 display
- XPT2046 touch controller
- ESP32 micro-controller

For build instructions visit: https://www.airgradient.com/resources/airgradient-diy-display/
For CYD info: https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display

Required libraries:
- "WiFiManager by tzapu, tablatronix" tested with Version 2.0.17
- "TFT_eSPI" by Bodmer tested with Version 2.5.43
- "ArduinoJSON" by Benoit Blanchon tested with Version 7.4.2
- "XPT2046_Touchscreen" by Paul Stoffregen with Version 1.4.0+sha.f956c5d

Configuration:
Set inUSaqi and inF below to display PM2.5 in US AQI and temperature in Fahrenheit.

MIT License
*/

// ----------------------------
// Standard Libraries
// ----------------------------
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>

// ----------------------------
// Additional Libraries
// ----------------------------
#include <WiFiManager.h>
// WiFiManager for easy WiFi configuration
// https://github.com/tzapu/WiFiManager

#include <TFT_eSPI.h>
// TFT display library
// https://github.com/Bodmer/TFT_eSPI

#include <XPT2046_Touchscreen.h>
// Touch screen library
// https://github.com/PaulStoffregen/XPT2046_Touchscreen

// ----------------------------
// Touch Screen pins
// ----------------------------
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// ----------------------------
// Display and Touch objects
// ----------------------------
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// ----------------------------
// AirGradient Data Variables
// ----------------------------
String locNameInside;
String locNameOutside;
String place_timezone;
String location;
bool outdoor_offline;
bool indoor_offline;
String outdoor_policy;
String outdoor_date;
String indoor_date;

String deviceID;
String timex;
int pm02;
int pi02;
int pi02_outside;
int rco2;
float atmp;
float atmp_outside;
int rhum_outside;
int rhum;
int heat;

String pi02_color;
String pi02_color_outside;
String pi02_category;
String pm02_color;
String pm02_category;
String rco2_color;
String rco2_category;
String heat_color;
String heat_color_outside;
String heat_category;

// ----------------------------
// Configuration
// ----------------------------
#define API_ROOT "http://hw.airgradient.com/displays/"
boolean inUSaqi = false;  // Set to true for US AQI instead of µg/m³
boolean inF = false;       // Set to true for Fahrenheit instead of Celsius

// ----------------------------
// Color Definitions for TFT_eSPI
// Note: TFT_PURPLE and TFT_BROWN are already defined in TFT_eSPI.h
// We only need to define colors that aren't in the library
// ----------------------------
// TFT_PURPLE and TFT_BROWN are already defined in TFT_eSPI
// #define TFT_PURPLE 0xF81F  // Already defined as 0x780F
// #define TFT_BROWN 0x7800   // Already defined as 0x9A60
#define TFT_DARKGREY 0x7BEF

// ----------------------------
// Function Declarations
// ----------------------------
String getDeviceId();
void welcomeMessage();
void connectToWifi();
void payloadToDataInside(String payload);
void updateDisplay();
uint16_t getColorFromString(String colorStr);

// ----------------------------
// Setup
// ----------------------------
void setup() {
    Serial.begin(115200);
    Serial.println("AirGradient Display for CYD");
    Serial.print("Chip ID: ");
    Serial.println(getDeviceId());

    // Initialize touch screen
    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(touchSPI);
    ts.setRotation(1);

    // Initialize TFT display
    tft.init();
    tft.setRotation(1); // Landscape mode
    tft.fillScreen(TFT_BLACK);

    // Show welcome message
    welcomeMessage();

    // Connect to WiFi
    connectToWifi();

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());

    tft.fillScreen(TFT_BLACK);
    delay(1000);

    // Fetch initial data on startup
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 100);
    tft.println("Loading data...");
    Serial.println("Fetching initial data...");

    HTTPClient http;
    http.begin(API_ROOT + getDeviceId());
    http.setTimeout(10000);
    http.setReuse(false);

    int httpCode = http.GET();
    if (httpCode == 200) {
        String airData = http.getString();
        payloadToDataInside(airData);
        Serial.println("Initial data loaded successfully");
        updateDisplay();
    } else {
        Serial.print("Initial fetch failed with HTTP error: ");
        Serial.println(httpCode);
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextSize(2);
        tft.setCursor(10, 100);
        tft.println("Data load failed");
        tft.setTextSize(1);
        tft.setCursor(10, 130);
        tft.println("Will retry in 5 minutes");
        tft.setCursor(10, 145);
        tft.println("Or tap screen to refresh");
    }
    http.end();
}

// ----------------------------
// Main Loop
// ----------------------------
void loop() {
    // Touch-to-refresh: check for touch to trigger immediate refresh
    static unsigned long lastUpdate = 0;
    static const unsigned long updateInterval = 300000; // 5 minutes (300000 ms)
    bool manualRefresh = false;

    // Check for touch
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        // Simple debounce: only allow refresh if last update > 2s ago
        if (millis() - lastUpdate > 2000) {
            manualRefresh = true;
            Serial.println("Touch detected - manual refresh triggered");

            // Visual feedback for touch
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_CYAN, TFT_BLACK);
            tft.setTextSize(3);
            tft.setCursor(40, 90);
            tft.println("REFRESHING");
            tft.setTextSize(2);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setCursor(60, 130);
            tft.println("Please wait...");
            delay(800); // Show feedback
        }
    }

    // Timed update or manual refresh
    if (manualRefresh || millis() - lastUpdate > updateInterval) {
        lastUpdate = millis();

        // Check if WiFi is connected
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected, reconnecting...");
            connectToWifi();
            return;
        }

        // Show loading message only for manual refresh
        if (manualRefresh) {
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            tft.setTextSize(3);
            tft.setCursor(50, 80);
            tft.println("UPDATING");

            tft.setTextSize(2);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setCursor(50, 130);
            tft.println("Fetching data...");
        }

        // Fetch data from AirGradient API
        HTTPClient http;
        http.begin(API_ROOT + getDeviceId());
        http.setTimeout(10000);  // 10 second timeout
        http.setReuse(false);    // Prevent memory leaks

        int httpCode = http.GET();
        if (httpCode == 200) {
            if (manualRefresh) {
                tft.setCursor(50, 130);
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.println("Processing...    ");
            }

            String airData = http.getString();
            payloadToDataInside(airData);
            Serial.println("Data received successfully");
        } else {
            Serial.print("HTTP error: ");
            Serial.println(httpCode);

            // Show error only for manual refresh
            if (manualRefresh) {
                tft.fillScreen(TFT_BLACK);
                tft.setTextColor(TFT_RED, TFT_BLACK);
                tft.setTextSize(2);
                tft.setCursor(10, 100);
                tft.print("HTTP Error: ");
                tft.println(httpCode);
                delay(3000);
            }
        }
        http.end();

        delay(500);

        // Update the display with fetched data
        updateDisplay();
    }

    // Small delay to avoid busy loop
    delay(50);
}

// ----------------------------
// Get unique device ID
// ----------------------------
String getDeviceId() {
    uint64_t chipid = ESP.getEfuseMac();
    return String((uint32_t)(chipid >> 32), HEX) + String((uint32_t)chipid, HEX);
}

// ----------------------------
// Parse JSON payload from API
// ----------------------------
void payloadToDataInside(String payload) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Place information
    location = doc["place"]["name"].as<const char*>();
    place_timezone = doc["place"]["timezone"].as<const char*>();

    // Outdoor data
    JsonObject outdoor = doc["outdoor"];
    locNameOutside = outdoor["name"].as<const char*>();
    outdoor_offline = outdoor["offline"];
    outdoor_policy = outdoor["guidelines"][0]["title"].as<const char*>();

    JsonObject outdoor_current = outdoor["current"];
    atmp_outside = outdoor_current["atmp"];
    rhum_outside = outdoor_current["rhum"];
    outdoor_date = outdoor_current["date"].as<const char*>();

    // Indoor data
    JsonObject indoor = doc["indoor"];
    locNameInside = indoor["name"].as<const char*>();
    indoor_offline = indoor["offline"];

    JsonObject indoor_current = indoor["current"];
    atmp = indoor_current["atmp"];
    rhum = indoor_current["rhum"];
    rco2 = indoor_current["rco2"];
    indoor_date = indoor_current["date"].as<const char*>();
    rco2_color = indoor_current["rco2_clr"].as<const char*>();
    rco2_category = indoor_current["rco2_lbl"].as<const char*>();

    // PM2.5 data (US AQI or µg/m³)
    if (inUSaqi) {
        pi02_outside = outdoor_current["pi02"];
        pi02_color_outside = outdoor_current["pi02_clr"].as<const char*>();
        pi02_category = outdoor_current["pi02_lbl"].as<const char*>();
        pi02 = indoor_current["pi02"];
        pi02_color = indoor_current["pi02_clr"].as<const char*>();
        pi02_category = indoor_current["pi02_lbl"].as<const char*>();
    } else {
        pi02_outside = outdoor_current["pm02"];
        pi02_color_outside = outdoor_current["pm02_clr"].as<const char*>();
        pi02_category = outdoor_current["pm02_lbl"].as<const char*>();
        pi02 = indoor_current["pm02"];
        pi02_color = indoor_current["pm02_clr"].as<const char*>();
        pi02_category = indoor_current["pm02_lbl"].as<const char*>();
    }
}

// ----------------------------
// Convert color string to RGB565
// ----------------------------
uint16_t getColorFromString(String colorStr) {
    if (colorStr == "green") return TFT_GREEN;
    else if (colorStr == "yellow") return TFT_YELLOW;
    else if (colorStr == "orange") return TFT_ORANGE;
    else if (colorStr == "red") return TFT_RED;
    else if (colorStr == "purple") return TFT_PURPLE;
    else if (colorStr == "brown") return TFT_BROWN;
    return TFT_LIGHTGREY;
}

// ----------------------------
// Update display with air quality data
// ----------------------------
void updateDisplay() {
    int y = 20;
    int boxHeight = 60;
    int boxWidth = 140;
    int radius = 8;
    int spacing = 10;

    tft.fillScreen(TFT_BLACK);

    // Display location name
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(5, y);
    tft.println(location);

    // Draw separator line
    tft.drawLine(0, 35, 320, 35, TFT_WHITE);

    y = 50;

    // ------ OUTDOOR SECTION ------
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(5, y);
    tft.println(locNameOutside);

    y += 15;

    // PM2.5 box
    tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, getColorFromString(pi02_color_outside));

    // Temperature/Humidity box
    tft.fillRoundRect(5 + boxWidth + spacing, y, boxWidth, boxHeight, radius, TFT_DARKGREY);

    // PM2.5 value
    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK, getColorFromString(pi02_color_outside));
    tft.setCursor(20, y + 15);
    tft.println(String(pi02_outside));

    // PM2.5 label
    tft.setTextSize(1);
    tft.setCursor(15, y + 45);
    if (inUSaqi) {
        tft.println("US AQI");
    } else {
        tft.println("ug/m3");
    }

    // Temperature
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setCursor(15 + boxWidth + spacing, y + 10);
    if (inF) {
        tft.println(String((atmp_outside * 9 / 5) + 32, 1) + "F");
    } else {
        tft.println(String(atmp_outside, 1) + "C");
    }

    // Humidity
    tft.setTextSize(2);
    tft.setCursor(15 + boxWidth + spacing, y + 35);
    tft.println(String(rhum_outside) + "%");

    y += boxHeight + 20;

    // ------ INDOOR SECTION ------
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(5, y);
    tft.println(locNameInside);

    y += 15;

    // PM2.5 box
    tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, getColorFromString(pi02_color));

    // CO2 box
    tft.fillRoundRect(5 + boxWidth + spacing, y, boxWidth, boxHeight, radius, getColorFromString(rco2_color));

    // PM2.5 value
    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK, getColorFromString(pi02_color));
    tft.setCursor(20, y + 15);
    tft.println(String(pi02));

    // PM2.5 label
    tft.setTextSize(1);
    tft.setCursor(15, y + 45);
    if (inUSaqi) {
        tft.println("US AQI");
    } else {
        tft.println("ug/m3");
    }

    // CO2 value
    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK, getColorFromString(rco2_color));
    tft.setCursor(20 + boxWidth + spacing, y + 15);
    tft.println(String(rco2));

    // CO2 label
    tft.setTextSize(1);
    tft.setCursor(15 + boxWidth + spacing, y + 45);
    tft.println("CO2 ppm");

    // Date/time stamp
    y += boxHeight + 10;
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setCursor(5, y);
    tft.println("Updated: ");
    tft.println(indoor_date);
}

// ----------------------------
// Show welcome message on startup
// ----------------------------
void welcomeMessage() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.setTextSize(3);
    tft.setCursor(50, 50);
    tft.println("AirGradient");

    tft.setTextSize(2);
    tft.setCursor(30, 100);
    tft.println("Display for CYD");

    tft.setTextSize(1);
    tft.setCursor(30, 140);
    tft.print("ID: ");
    tft.println(getDeviceId());

    delay(2000);
}

// ----------------------------
// Callback when WiFiManager enters config mode
// ----------------------------
void configModeCallback(WiFiManager *myWiFiManager) {
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    Serial.println(myWiFiManager->getConfigPortalSSID());

    // Show WiFi setup required message
    tft.fillRect(0, 160, 320, 80, TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 165);
    tft.println("WiFi Setup Required!");
    tft.setCursor(10, 180);
    tft.println("Connect to WiFi hotspot:");
    tft.setCursor(10, 195);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.println(myWiFiManager->getConfigPortalSSID());
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 210);
    tft.println("Then open 192.168.4.1");
}

// ----------------------------
// Connect to WiFi using WiFiManager
// ----------------------------
void connectToWifi() {
    WiFiManager wifiManager;

    String HOTSPOT = "AIRGRADIENT-CYD-" + getDeviceId();
    wifiManager.setTimeout(120);

    // Set callback for when entering config mode (AP mode)
    wifiManager.setAPCallback(configModeCallback);

    // Show initial connection attempt message
    tft.fillRect(0, 160, 320, 80, TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 165);
    tft.println("Connecting to WiFi...");
    Serial.println("Attempting WiFi connection...");

    // autoConnect will:
    // 1. Try to connect with saved credentials
    // 2. If that fails, start config portal (triggers callback)
    // 3. Return true if connected, false if timeout
    if (!wifiManager.autoConnect(HOTSPOT.c_str())) {
        Serial.println("Failed to connect and hit timeout");
        tft.fillRect(0, 160, 320, 80, TFT_BLACK);
        tft.setCursor(10, 165);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.println("WiFi Connection Failed!");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(10, 180);
        tft.println("Restarting...");
        delay(3000);
        ESP.restart();
        delay(5000);
    }

    // Successfully connected
    tft.fillRect(0, 160, 320, 80, TFT_BLACK);
    tft.setCursor(10, 165);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("WiFi Connected!");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 180);
    tft.print("IP: ");
    tft.println(WiFi.localIP());
    Serial.println("Connected! IP: " + WiFi.localIP().toString());
    delay(2000);
}
