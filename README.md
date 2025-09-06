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

1. Run the configuration script to verify required tools and libraries:

   ```bash
   ./configure
   ```

   The script reports any missing dependencies and hints on how to install them.

2. (Optional) Install the toolchain and libraries using the provided setup target:

   ```bash
   make setup
   ```

   The script configures a local Arduino toolchain. It now detects
   both `/etc/apt/sources.list` and the newer `/etc/apt/sources.list.d/ubuntu.sources`
   and will skip mirror configuration if neither file exists.

3. Compile the sketch for NodeMCU v2 (default make target):

   ```bash
   make       # or: make build
   ```

   The build artifacts will be placed in the `build/` directory.

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

## WiFi resilience

The sketch monitors the wireless connection and enables automatic
reconnection. If WiFi drops, the main loop triggers a disconnect and
retries `connectWiFi()` for up to 10 seconds per attempt until the
network is restored, allowing MQTT publishing to resume without manual
intervention.

## License

This project is licensed under the [MIT License](LICENSE).
