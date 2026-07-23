# RoomSense IQ - ESPHome firmware

Maintainable [ESPHome](https://esphome.io) firmware for the **RoomSense IQ**
(ESP32-S3-WROOM-1) air-quality and presence sensor. It replaces the original
ESP-IDF firmware after that project went dormant, and reports every usable
sensor to Home Assistant over the encrypted native API.

> **Have an MFrontier MPM10 PM sensor?** The [`mpm10`](components/mpm10/) component
> here is a standalone ESPHome driver for it - a part that had no existing ESPHome
> support, written from scratch for this project. It is not tied to the RoomSense
> IQ: anyone with an MPM10 on any ESP32/ESP8266 board can drop it into their own
> config. See [The MPM10 component](#the-mpm10-component) for how.

## Sensors

| Home Assistant entity | Device | Bus | ESPHome component |
|-----------------------|--------|-----|-------------------|
| Carbon dioxide, Temperature, Humidity | Sensirion SCD4x (0x62) | I2C | `scd4x` |
| VOC index, NOx index | Sensirion SGP41 (0x59) | I2C | `sgp4x` |
| PM1 / PM2.5 / PM10 | MFrontier MPM10 (0x4D) | I2C | `mpm10` (custom, in `components/`) |
| Presence + distances | HLK-LD2410 | UART @ 256000 | `ld2410` |
| Light Level | CdS photocell | ADC1 (GPIO5) | `adc` |
| Status LED | EAST1616RGBB4 | LEDC | `light` (rgb) |

**Not included:** the CO gas sensor (wired to ADC2/GPIO20, which the ESP32-S3
cannot read while Wi-Fi is on) and the analog PIR (the LD2410 supersedes it).

## Hardware

Pin map (recovered from the original firmware and the Rev8 schematic in
[`hardware/`](hardware/)):

- I2C: SDA=GPIO12, SCL=GPIO13 (SCD4x, SGP41, MPM10 share this bus)
- LD2410 UART: RX=GPIO8, TX=GPIO9
- RGB LED (LEDC): R=GPIO46, G=GPIO10, B=GPIO3
- Light (ADC1): GPIO5

The SCD4x uses a large `temperature_offset` (12.1 C) to compensate for
self-heating inside the enclosure; tune it to your case.

## The MPM10 component

The particulate sensor is an **MFrontier MPM10** (Shenzhen MemsFrontier
Electronics) laser-scattering PM sensor, which supports UART/I2C/PWM; this board
wires it over I2C at address 0x4D. It reports PM1/PM2.5/PM10 as big-endian uint16
values split across consecutive registers. ESPHome has no stock component for the
MPM10, so this project provides one: [`components/mpm10/`](components/mpm10/) is a
from-scratch ESPHome driver and the main piece of original code in this repo.

The register map (address 0x4D; PM1/PM2.5/PM10 as big-endian uint16 at registers
0x20/0x22/0x24) was first recovered from the original RoomSense ESP-IDF firmware,
then confirmed byte-for-byte against the official MFrontier protocol datasheet.
Both datasheets are in [`hardware/`](hardware/): `MPM10-datasheet.pdf` (2-page
overview) and `MPM10-protocol-datasheet.pdf` (11-page, with the full IIC register
map, UART frame format, and PWM output).

The component reads all of the sensor's I2C data. Beyond the standard PM1/PM2.5/PM10
mass (0x20/0x22/0x24), it optionally exposes the six particle-number count bins
(>0.3/0.5/1.0/2.5/5.0/10 um per 0.1 L, at 0x2C-0x36) and the atmospheric-mode PM
mass (0x26/0x28/0x2A). The default config enables the mass values and the count
bins; add `pm1_0_atm` / `pm2_5_atm` / `pm10_0_atm` for the atmospheric set.

Note: the datasheet's particle-count rows are internally garbled (the diameter
thresholds are printed in the sample-volume column). The register order and the
Plantower-family protocol make the meaning unambiguous, and it is borne out live:
the counts come back as a clean monotonically-decreasing size distribution.

### Reusing the component on your own board

The `mpm10` component is self-contained - it depends only on ESPHome's `i2c` and
`sensor` platforms, nothing RoomSense-specific - so anyone with an MPM10 can use
it independently of the rest of this firmware. Point `external_components` at
this repo and add the platform (all sensors are optional; list only what you
want):

```yaml
external_components:
  - source: github://brendanhoran/roomsense-iq-esphome
    components: [mpm10]

i2c:
  sda: GPIO12      # your board's pins (these are the RoomSense IQ's)
  scl: GPIO13

sensor:
  - platform: mpm10
    # address defaults to 0x4D
    pm1_0:
      name: "PM1"
    pm2_5:
      name: "PM2.5"
    pm10_0:
      name: "PM10"
    # optional: pm1_0_atm / pm2_5_atm / pm10_0_atm (atmospheric-mode mass)
    # optional: pm_0_3um / pm_0_5um / pm_1_0um / pm_2_5um / pm_5_0um / pm_10_0um
    #           (particle-number counts, per 0.1 L)
```

Alternatively, copy `components/mpm10/` into a local `components/` directory and
use `source: {type: local, path: components}`. Contributions to fold this into
[ESPHome upstream](https://esphome.io) would be welcome.

## Quick start

```sh
python3 -m venv .venv
.venv/bin/pip install esphome

cp secrets.yaml.example secrets.yaml
# edit secrets.yaml: Wi-Fi credentials + an API key (openssl rand -base64 32)

make validate                       # check the config
make upload DEVICE=/dev/ttyUSB0     # first flash over USB
make upload DEVICE=<device-ip>      # later updates over the air
make logs   DEVICE=<device-ip>      # stream logs
```

Then add the device in Home Assistant (Settings -> Devices & Services ->
ESPHome) and paste the API key.

## Notes

- This board does not break out the ESP32-S3 native USB, so `logger` is pinned
  to `hardware_uart: UART0` to get serial logs over the USB-C cable.
- The SGP41 needs ~90 s to warm up after each boot before VOC/NOx are valid.

## Acknowledgements

The ESPHome configuration and the `mpm10` component here are original work, but
the hardware knowledge behind them - the pin map, the I2C addresses, the MPM10
register protocol, and the SCD4x self-heating offset - was recovered by reading
the original RoomSense ESP-IDF firmware. Credit and thanks to:

- [roomsense/roomsense-iq](https://github.com/roomsense/roomsense-iq) - the
  original (public-domain) RoomSense Labs firmware. Source of the MPM10 I2C
  register map (`components/mpm10/mpm10.c`), the sensor set, and the pin map.
- [tofurky/roomsense-iq](https://github.com/tofurky/roomsense-iq) - Matt Merhar's
  actively-maintained fork of that firmware (CI, bug fixes, cleanups), the
  version this project was read against.

No source code was copied from either repo; the datasheets in [`hardware/`](hardware/)
are the independent confirmation of the protocol details.

## Licence

Public domain (CC0 1.0) for this project's own files. See [`LICENSE`](LICENSE).

The vendor datasheets bundled in [`hardware/`](hardware/) are the copyright of
their respective manufacturers and are **not** covered by CC0 - they are included
for reference only. See [`hardware/README.md`](hardware/README.md#copyright).
