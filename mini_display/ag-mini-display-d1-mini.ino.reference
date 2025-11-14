/*
This is the code for the AirGradient DIY Mini Display with an ESP8266 Microcontroller.
It can be configures to show the outside air quality as well as one indoor location from the AirGradient platform.
For build instructions please visit https://www.airgradient.com/resources/airgradient-diy-display/
The codes needs the following libraries installed:
"WifiManager by tzapu, tablatronix" tested with Version 2.0.5-alpha
"Adafruit_ILI9341" tested with Version 1.5.10
"Adafruit GFX library" tested with Version 1.10.12 (often automatically installed with above ILI9341 library)
"ArduinoJSON" by Benoit Blanchon tested with Version 6.8.5


Configuration:
Please set in the code below if you want to display the PM2.5 values in US AQI and temperature in F.


If you are a school or university contact us for a free trial on the AirGradient platform.
https://www.airgradient.com/schools/


MIT License
*/

// #include <Arduino.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>

// #include <ArduinoOTA.h>
// #include <ESP8266httpUpdate.h>

#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1
#define TS_CS D3

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

const char *locNameInside;
const char *locNameOutside;

const char *place_timezone;
const char *location;
bool outdoor_offline;
bool indoor_offline;
const char *outdoor_policy;
const char *outdoor_date;
const char *indoor_date;
boolean prodMode = true;

String deviceID;
const char *timex;
int pm02;
int pi02;
int pi02_outside;
int rco2;
float atmp;
float atmp_outside;
int rhum_outside;
int rhum;
int heat;

const char *pi02_color;
const char *pi02_color_outside;
const char *pi02_category;
const char *pm02_color;
const char *pm02_category;
const char *rco2_color;
const char *rco2_category;
const char *heat_color;
const char *heat_color_outside;
const char *heat_category;

// Configuration
#define API_ROOT "http://hw.airgradient.com/displays/"
boolean inUSaqi = false;
boolean inF = false;

String getDeviceId()
{
    return String(ESP.getChipId(), HEX);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Chip ID");
    Serial.println(String(ESP.getChipId(), HEX));

    tft.begin();
    tft.setRotation(2);
    while (!Serial && (millis() <= 1000))
        ;
    welcomeMessage();
    connectToWifi();

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    tft.fillScreen(ILI9341_BLACK);
    delay(2000);
}

void loop()
{

    WiFiClient client;
    HTTPClient http;
    http.begin(client, API_ROOT + getDeviceId());

    int httpCode = http.GET();
    if (httpCode == 200)
    {
        String airData = http.getString();
        payloadToDataInside(airData);
        Serial.print("airData1 : ");
        Serial.println(airData);
    }
    else
    {
        Serial.println("error");
        Serial.println(httpCode);
    }
    http.end();

    delay(1000);
    updateDisplay();

    delay(120000);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);
    tft.setFont(&FreeSans12pt7b);
    tft.setCursor(5, 20);
    tft.println("requesting data…");
}

void payloadToDataInside(String payload)
{
    const size_t capacity = JSON_ARRAY_SIZE(1) + 2JSON_OBJECT_SIZE(2) + 2JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(10) + JSON_OBJECT_SIZE(13) + 530;
    DynamicJsonBuffer jsonBuffer(capacity);
    JsonObject &root = jsonBuffer.parseObject(payload);
    location = root["place"]["name"];
    place_timezone = root["place"]["timezone"];
    JsonObject &outdoor = root["outdoor"];
    locNameOutside = outdoor["name"];
    outdoor_offline = outdoor["offline"];
    outdoor_policy = outdoor["guidelines"][0]["title"];
    JsonObject &outdoor_current = outdoor["current"];

    atmp_outside = outdoor_current[&quot; atmp & quot;];
    rhum_outside = outdoor_current[&quot; rhum & quot;];
    outdoor_date = outdoor_current[&quot; date & quot;];
    JsonObject & amp;
    indoor = root[&quot; indoor & quot;];
    locNameInside = indoor[&quot; name & quot;];
    indoor_offline = indoor[&quot; offline & quot;];
    JsonObject & amp;
    indoor_current = indoor[&quot; current & quot;];

    atmp = indoor_current[&quot; atmp & quot;];
    rhum = indoor_current[&quot; rhum & quot;];
    rco2 = indoor_current[&quot; rco2 & quot;];
    indoor_date = indoor_current[&quot; date & quot;];
    rco2_color = indoor_current[&quot; rco2_clr & quot;];
    rco2_category = indoor_current[&quot; rco2_lbl & quot;];

    if (inUSaqi)
    {
        pi02_outside = outdoor_current[&quot; pi02 & quot;];
        pi02_color_outside = outdoor_current[&quot; pi02_clr & quot;];
        pi02_category = outdoor_current[&quot; pi02_lbl & quot;];
        pi02 = indoor_current[&quot; pi02 & quot;];
        pi02_color = indoor_current[&quot; pi02_clr & quot;];
        pi02_category = indoor_current[&quot; pi02_lbl & quot;];
    }
    else
    {
        pi02_outside = outdoor_current[&quot; pm02 & quot;];
        pi02_color_outside = outdoor_current[&quot; pm02_clr & quot;];
        pi02_category = outdoor_current[&quot; pm02_lbl & quot;];
        pi02 = indoor_current[&quot; pm02 & quot;];
        pi02_color = indoor_current[&quot; pm02_clr & quot;];
        pi02_category = indoor_current[&quot; pm02_lbl & quot;];
    }
}

void updateDisplay()
{
    int y = 25;
    int boxHeight = 75;
    int boxWidth = 110;
    int radius = 8;
    tft.fillScreen(ILI9341_BLACK);

    tft.setFont(&amp; FreeSans9pt7b);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setCursor(5, y);
    tft.println(location);

    tft.drawLine(0, 35, 250, 35, ILI9341_WHITE);

    y = y + 50;

    tft.setFont(&amp; FreeSans9pt7b);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setCursor(5, y);
    tft.println(locNameOutside);
    tft.setFont(&amp; FreeSans12pt7b);

    y = y + 12;

    if (String(pi02_color_outside) == &quot; green & quot;)
    {
        tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, ILI9341_GREEN);
    }
    else if (String(pi02_color_outside) == &quot; yellow & quot;)
    {
        tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, ILI9341_YELLOW);
    }
    else if (String(pi02_color_outside) == &quot; orange & quot;)
    {
        tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, ILI9341_ORANGE);
    }
    else if (String(pi02_color_outside) == &quot; red & quot;)
    {
        tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, ILI9341_RED);
    }
    else if (String(pi02_color_outside) == &quot; purple & quot;)
    {
        tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, ILI9341_PURPLE);
    }
    else if (String(pi02_color_outside) == &quot; brown & quot;)
    {
        tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, ILI9341_MAROON);
    }

    if (String(heat_color_outside) == &quot; green & quot;)
    {
        tft.fillRoundRect(5 + boxWidth + 10, y, boxWidth, boxHeight, radius, ILI9341_GREEN);
    }
    else if (String(heat_color_outside) == &quot; yellow & quot;)
    {
        tft.fillRoundRect(5 + boxWidth + 10, y, boxWidth, boxHeight, radius, ILI9341_YELLOW);
    }
    else if (String(heat_color_outside) == &quot; orange & quot;)
    {
        tft.fillRoundRect(5 + boxWidth + 10, y, boxWidth, boxHeight, radius, ILI9341_ORANGE);
    }
    else if (String(heat_color_outside) == &quot; red & quot;)
    {
        tft.fillRoundRect(5 + boxWidth + 10, y, boxWidth, boxHeight, radius, ILI9341_RED);
    }
    else if (String(heat_color_outside) == &quot; purple & quot;)
    {
        tft.fillRoundRect(5 + boxWidth + 10, y, boxWidth, boxHeight, radius, ILI9341_PURPLE);
    }
    else if (String(heat_color_outside) == &quot; brown & quot;)
    {
        tft.fillRoundRect(5 + boxWidth + 10, y, boxWidth, boxHeight, radius, ILI9341_MAROON);
    }

    tft.setFont(&amp; FreeSans9pt7b);
    tft.setTextColor(ILI9341_BLACK, ILI9341_BLACK);
    tft.setCursor(20, y + boxHeight - 10);

    if (inUSaqi)
    {
        tft.println(&quot; US AQI & quot;);
    }
    else
    {
        tft.println(&quot; ug / m3 & quot;);
    }

    tft.setFont(&amp; FreeSans18pt7b);
    tft.setTextColor(ILI9341_BLACK, ILI9341_BLACK);
    tft.setCursor(20, y + 40);
    tft.println(String(pi02_outside));

    tft.setFont(&amp; FreeSans9pt7b);

    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

    tft.setCursor(20 + boxWidth + 10, y + 20);

    if (inF)
    {
        tft.println(String((atmp_outside * 9 / 5) + 32) + &quot; F & quot;);
    }
    else
    {
        tft.println(String(atmp_outside) + &quot; C & quot;);
    }

    tft.setCursor(20 + boxWidth + 10, y + 40);
    tft.println(String(rhum_outside) + &quot; % &quot;);

    tft.setTextColor(ILI9341_DARKGREY, ILI9341_BLACK);
    tft.setCursor(20 + boxWidth + 10, y + 60);
    tft.println(String(outdoor_date));

    // inside

    y = y + 110;

    tft.setFont(&amp; FreeSans9pt7b);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setCursor(5, y);
    tft.println(locNameInside);
    tft.setFont(&amp; FreeSans12pt7b);

    y = y + 12;

    if (String(pi02_color) == &quot; green & quot;)
    {
        tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, ILI9341_GREEN);
    }
    else if (String(pi02_color) == &quot; yellow & quot;)
    {
        tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, ILI9341_YELLOW);
    }
    else if (String(pi02_color) == &quot; orange & quot;)
    {
        tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, ILI9341_ORANGE);
    }
    else if (String(pi02_color) == &quot; red & quot;)
    {
        tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, ILI9341_RED);
    }
    else if (String(pi02_color) == &quot; purple & quot;)
    {
        tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, ILI9341_PURPLE);
    }
    else if (String(pi02_color) == &quot; brown & quot;)
    {
        tft.fillRoundRect(5, y, boxWidth, boxHeight, radius, ILI9341_MAROON);
    }

    if (String(rco2_color) == &quot; green & quot;)
    {
        tft.fillRoundRect(5 + boxWidth + 10, y, boxWidth, boxHeight, radius, ILI9341_GREEN);
    }
    else if (String(rco2_color) == &quot; yellow & quot;)
    {
        tft.fillRoundRect(5 + boxWidth + 10, y, boxWidth, boxHeight, radius, ILI9341_YELLOW);
    }
    else if (String(rco2_color) == &quot; orange & quot;)
    {
        tft.fillRoundRect(5 + boxWidth + 10, y, boxWidth, boxHeight, radius, ILI9341_ORANGE);
    }
    else if (String(rco2_color) == &quot; red & quot;)
    {
        tft.fillRoundRect(5 + boxWidth + 10, y, boxWidth, boxHeight, radius, ILI9341_RED);
    }
    else if (String(rco2_color) == &quot; purple & quot;)
    {
        tft.fillRoundRect(5 + boxWidth + 10, y, boxWidth, boxHeight, radius, ILI9341_PURPLE);
    }
    else if (String(rco2_color) == &quot; brown & quot;)
    {
        tft.fillRoundRect(5 + boxWidth + 10, y, boxWidth, boxHeight, radius, ILI9341_MAROON);
    }

    tft.setFont(&amp; FreeSans9pt7b);
    tft.setTextColor(ILI9341_BLACK, ILI9341_BLACK);
    tft.setCursor(20, y + boxHeight - 10);

    if (inUSaqi)
    {
        tft.println(&quot; US AQI & quot;);
    }
    else
    {
        tft.println(&quot; ug / m3 & quot;);
    }

    tft.setCursor(20 + boxWidth + 10, y + boxHeight - 10);
    tft.println(&quot; CO2 ppm & quot;);

    tft.setFont(&amp; FreeSans18pt7b);
    tft.setTextColor(ILI9341_BLACK, ILI9341_BLACK);
    tft.setCursor(20, y + 40);
    tft.println(String(pi02));
    tft.setCursor(20 + boxWidth + 10, y + 40);
    tft.println(String(rco2));

    y = y + 100;

    tft.setFont(&amp; FreeSans9pt7b);
    tft.setTextColor(ILI9341_DARKGREY, ILI9341_BLACK);
    tft.setCursor(boxWidth - 30, y);
    tft.println(String(indoor_date));
}

void welcomeMessage()
{
    Serial.println("Welcome Message 2");
    tft.setFont(&FreeSans9pt7b);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);

    tft.setCursor(40, 24);
    tft.setFont(&FreeSans12pt7b);
    tft.setCursor(5, 20);
    tft.println("AirGradient");

    tft.setFont(&FreeSans9pt7b);
    tft.setCursor(5, 100);
    tft.println("id: " + String(ESP.getChipId(), HEX));

    tft.setCursor(5, 140);
    tft.println("connecting …");

    delay(2000);
}

void connectToWifi()
{
    delay(2000);

    WiFiManager wifiManager;
    // chWiFi.disconnect(); //to delete previous saved hotspot
    String HOTSPOT = "AIRGRADIENT-DISPLAY-" + String(ESP.getChipId(), HEX);
    wifiManager.setTimeout(120);
    if (!wifiManager.autoConnect((const char *)HOTSPOT.c_str()))
    {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
        delay(5000);
    }
}
