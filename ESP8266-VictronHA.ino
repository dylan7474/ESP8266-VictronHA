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

void flashStatusCode(int code) {
    static int currentCode = -1;
    static int flashes = 0;
    static bool ledOn = false;
    static unsigned long lastChange = 0;
    unsigned long now = millis();

    if (code != currentCode) {
        currentCode = code;
        flashes = 0;
        ledOn = false;
        digitalWrite(LED_PIN, HIGH);
        lastChange = now;
    }

    if (code <= 0) {
        digitalWrite(LED_PIN, HIGH);
        return;
    }

    if (flashes >= currentCode) {
        if (now - lastChange >= 1000) {
            flashes = 0;
        }
        return;
    }

    if (now - lastChange >= 200) {
        lastChange = now;
        if (ledOn) {
            digitalWrite(LED_PIN, HIGH);
            ledOn = false;
            flashes++;
        } else {
            digitalWrite(LED_PIN, LOW);
            ledOn = true;
        }
    }
}

void updateLED() {
    int code = 0;
    if (WiFi.status() != WL_CONNECTED) {
        code = 1; // WiFi disconnected
    } else if (!mqttClient.connected()) {
        code = 2; // MQTT disconnected
    } else if (millis() - lastDataSent < 5000) {
        code = 4; // Data recently sent
    } else {
        code = 3; // Connected and idle
    }
    flashStatusCode(code);
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
        String topic = String(MQTT_TOPIC) + "/" + key;
        mqttClient.publish(topic.c_str(), value.c_str());
        lastDataSent = millis();
    }
}

void startupBlink() {
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, LOW);
        delay(200);
        digitalWrite(LED_PIN, HIGH);
        delay(200);
    }
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, LOW);
        delay(600);
        digitalWrite(LED_PIN, HIGH);
        delay(200);
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
    startupBlink();
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
