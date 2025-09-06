.DEFAULT_GOAL := build
FQBN=esp8266:esp8266:nodemcuv2
SKETCH=ESP8266-VictronHA.ino
BUILD_DIR=build
BIN=$(BUILD_DIR)/$(notdir $(SKETCH)).bin

.PHONY: setup build clean

setup:
	bash CodexEnvironment.txt

build:
	arduino-cli compile --fqbn $(FQBN) --output-dir $(BUILD_DIR) $(SKETCH)
	@echo "Firmware binary: $(BIN)"

clean:
	rm -rf $(BUILD_DIR)
