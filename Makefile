FQBN=esp8266:esp8266:nodemcuv2
SKETCH=ESP8266-VictronHA.ino

.PHONY: setup build clean

setup:
	bash CodexEnvironment.txt

build:
	arduino-cli compile --fqbn $(FQBN) $(SKETCH)

clean:
	rm -rf build
