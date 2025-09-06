# ESP8266-VictronHA
A Victron SmartShunt to Home Assistant interface (over wifi).

This sketch runs on an ESP8266, reads voltage, current, and state of charge
values from a Victron SmartShunt over the serial interface, displays the
information on an OLED screen, and publishes the measurements to an MQTT
broker for consumption by Home Assistant.

## Configuration

Update the network credentials and MQTT broker settings in `config.h` to match
your environment before compiling the sketch.

## Build

This project uses [arduino-cli](https://arduino.github.io/arduino-cli/latest/) to compile for the ESP8266.

1. Install dependencies and required libraries by running the setup script:

   ```bash
   bash CodexEnvironment.txt
   ```

2. Compile the sketch for NodeMCU v2:

   ```bash
   arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 ESP8266-VictronHA.ino
   ```

The sketch file `ESP8266-VictronHA.ino` must reside in a folder with the same name to compile correctly.

## LED status codes

The built-in LED provides a repeating flash sequence to indicate the current
state of the device:

| Flashes | Meaning                  |
| ------- | ------------------------ |
| 1       | WiFi disconnected        |
| 2       | MQTT disconnected        |
| 3       | Connected and idle       |
| 4       | Data recently published  |

Each sequence repeats with a short pause between groups.
