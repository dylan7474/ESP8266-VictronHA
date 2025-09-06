# ESP8266-VictronHA

## Overview
ESP8266-VictronHA is a firmware project for the ESP8266 that bridges a Victron SmartShunt battery monitor to Home Assistant over Wi-Fi and MQTT. The device reads SmartShunt data via the serial port, shows key values on an OLED display, and publishes measurements to Home Assistant.

## Building
Before building, run the provided `configure` script to verify that the required toolchain and libraries are available.

### Linux
```sh
make
```

### Windows
```sh
make -f Makefile.win
```

## Basic Controls
- On startup the device connects to the configured Wi-Fi network and MQTT broker.
- Live voltage, current and state of charge values scroll across the OLED screen.
- Pressing the reset button restarts the device and re-establishes all connections.

## Roadmap
- Add a web interface for configuring Wi-Fi and MQTT credentials.
- Support over-the-air firmware updates.
- Expose additional data from the SmartShunt.
