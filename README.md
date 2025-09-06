# ESP8266-VictronHA
A Victron SmartShunt to Home Assistant interface (over wifi)

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
