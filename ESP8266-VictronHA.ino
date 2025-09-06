#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "config.h"

#define BAUD_RATE 19200
#define i2c_Address 0x3C

const char* MQTT_TOPIC = "homeassistant/sensor/victron";

WiFiClient espClient;
PubSubClient mqttClient(espClient);
Adafruit_SH1106G display(128, 64, &Wire, -1);

String voltage = "0.0V";
String current = "0.0A";
String soc = "0.0%";

bool activityIndicator = false;
int scrollOffset = 128;
String scrollText = "";
int disconnectCount = 0;
unsigned long connectStartTime = 0;
float connectionTime = 0;
bool wifiConnectedOnce = false;

const int LED_PIN = LED_BUILTIN;
unsigned long lastLED = 0;
bool ledState = false;
unsigned long lastDataSent = 0;

void connectWiFi() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("Connecting to WiFi...");
    display.display();

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    connectStartTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - connectStartTime < 10000) {
        delay(500);
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Connecting to WiFi...");
        display.print("Time: ");
        display.print((millis() - connectStartTime) / 1000);
        display.println("s");
        display.display();
    }
    if (WiFi.status() == WL_CONNECTED) {
        connectionTime = (millis() - connectStartTime) / 1000.0;
        wifiConnectedOnce = true;
    }
}

void connectMQTT() {
    while (!mqttClient.connected()) {
        if (mqttClient.connect("ESP8266-Victron")) {
            return;
        }
        delay(5000);
    }
}

void updateLED() {
    bool connected = (WiFi.status() == WL_CONNECTED) && mqttClient.connected();
    unsigned long interval;
    unsigned long now = millis();

    if (!connected) {
        interval = 200; // Fast blink for connectivity issues
    } else if (now - lastDataSent < 5000) {
        interval = 1000; // Slow blink when data recently sent
    } else {
        digitalWrite(LED_PIN, HIGH); // Off when idle
        return;
    }

    if (now - lastLED >= interval) {
        lastLED = now;
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? LOW : HIGH); // LED is active LOW
    }
}

void updateDisplay() {
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(scrollOffset, 20);
    display.println(scrollText);
    
    if (activityIndicator) {
        display.fillRect(120, 0, 5, 5, SH110X_WHITE);
    }
    
    display.setTextSize(1);
    display.setCursor(0, 55);
    display.print("WiFi Time: ");
    display.print(connectionTime, 1);
    display.print("s");
    display.print(" DC:");
    display.print(disconnectCount);
    
    display.display();
    
    scrollOffset -= 2;
    if (scrollOffset < -((int)scrollText.length() * 18)) {
        scrollOffset = 128;
    }
}

void processVictronData(String key, String value) {
    if (key == "V") {
        voltage = "V: " + String(value.toFloat() / 1000, 2) + "V";
    } else if (key == "I") {
        current = "I: " + String(value.toFloat() / 1000, 2) + "A";
    } else if (key == "SOC") {
        soc = "SOC: " + String(value.toFloat() / 10, 1) + "%";
    }
    
    scrollText = voltage + "  |  " + current + "  |  " + soc;
    activityIndicator = !activityIndicator;

    if (mqttClient.connected()) {
        String payload = voltage + "," + current + "," + soc;
        mqttClient.publish(MQTT_TOPIC, payload.c_str());
        lastDataSent = millis();
    }
}

void setup() {
    display.begin(i2c_Address, true);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connecting...");
    display.display();
    
    Serial.begin(115200);
    Serial.setRxBufferSize(256);
    Serial.begin(BAUD_RATE, SERIAL_8N1);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);

    connectWiFi();
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    connectMQTT();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        disconnectCount++;
        WiFi.disconnect();
        connectWiFi();
    }
    
    if (!mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop();

    if (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();

        int separator = line.indexOf('\t');
        if (separator != -1) {
            String key = line.substring(0, separator);
            String value = line.substring(separator + 1);
            processVictronData(key, value);
        }
    }

    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate > 100) {
        updateDisplay();
        lastDisplayUpdate = millis();
    }

    updateLED();
}
