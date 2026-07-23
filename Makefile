# RoomSense IQ - ESPHome firmware
#
# Usage: `make help`. Override the binary or target device on the command line,
# e.g. `make upload DEVICE=192.168.200.213` or `make logs DEVICE=/dev/ttyUSB0`.

ESPHOME ?= esphome
CONFIG  ?= roomsense-iq.yaml
DEVICE  ?=

.PHONY: help validate compile upload logs clean

help:  ## Show this help
	@grep -E '^[a-zA-Z0-9_-]+:.*## ' $(MAKEFILE_LIST) \
		| awk 'BEGIN{FS=":.*## "}{printf "  %-10s %s\n", $$1, $$2}'

validate:  ## Validate config + custom component (fast, no toolchain download)
	$(ESPHOME) config $(CONFIG)

compile:  ## Compile the firmware (full build)
	$(ESPHOME) compile $(CONFIG)

upload:  ## Compile + flash. Set DEVICE=<ip|serial>; omit to auto-discover (OTA)
	$(ESPHOME) upload $(CONFIG) $(if $(DEVICE),--device $(DEVICE),)

logs:  ## Stream logs. Set DEVICE=<ip|serial>
	$(ESPHOME) logs $(CONFIG) $(if $(DEVICE),--device $(DEVICE),)

clean:  ## Remove the ESPHome build cache
	rm -rf .esphome
