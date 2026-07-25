# RoomSense IQ - ESPHome firmware
#
# Usage: `make help`. First run `make setup` (creates .venv and installs the
# pinned ESPHome), then validate/compile/upload/logs. Override the interpreter,
# binary, config, or device on the command line, e.g.
# `make setup PYTHON=python3.12` or `make upload DEVICE=roomsense-iq.local`.

PYTHON  ?= python3
VENV    ?= .venv
ESPHOME ?= $(VENV)/bin/esphome
CONFIG  ?= roomsense-iq.yaml
DEVICE  ?=

.PHONY: help setup validate compile upload logs clean check-esphome

help:  ## Show this help
	@grep -E '^[a-zA-Z0-9_-]+:.*## ' $(MAKEFILE_LIST) | awk 'BEGIN{FS=":.*## "}{printf "  %-12s %s\n", $$1, $$2}'

setup:  ## Create the .venv and install the pinned ESPHome (run once)
	$(PYTHON) -m venv $(VENV)
	$(VENV)/bin/pip install --upgrade pip
	$(VENV)/bin/pip install -r requirements.txt

validate: check-esphome  ## Validate config + custom component (fast, no toolchain download)
	$(ESPHOME) config $(CONFIG)

compile: check-esphome  ## Compile the firmware (full build)
	$(ESPHOME) compile $(CONFIG)

upload: check-esphome  ## Compile + flash. Set DEVICE=<ip|serial>; omit to auto-discover (OTA)
	$(ESPHOME) upload $(CONFIG) $(if $(DEVICE),--device $(DEVICE),)

logs: check-esphome  ## Stream logs. Set DEVICE=<ip|serial>
	$(ESPHOME) logs $(CONFIG) $(if $(DEVICE),--device $(DEVICE),)

clean:  ## Remove the ESPHome build cache
	rm -rf .esphome

# Internal guard: fail with guidance if ESPHome isn't installed yet.
check-esphome:
	@command -v $(ESPHOME) >/dev/null 2>&1 || { \
		echo "ESPHome not found at '$(ESPHOME)'." >&2; \
		echo "Run 'make setup' first, or set ESPHOME to an existing install" >&2; \
		echo "(e.g. 'make compile ESPHOME=esphome')." >&2; \
		exit 1; }
