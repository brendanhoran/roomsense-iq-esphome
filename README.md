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

**Not included:** the carbon monoxide (CO) gas sensor - a separate analog sensor,
not to be confused with the carbon dioxide (CO2) above - which is wired to
ADC2/GPIO20, unreadable while Wi-Fi is on for the ESP32-S3; and the analog PIR
(the LD2410 supersedes it).

## Hardware

Pin map (recovered from the original firmware and the Rev8 schematic in
[`hardware/`](hardware/)):

- I2C: SDA=GPIO12, SCL=GPIO13 (SCD4x, SGP41, MPM10 share this bus)
- LD2410 UART: RX=GPIO8, TX=GPIO9
- RGB LED (LEDC): R=GPIO46, G=GPIO10, B=GPIO3
- Light (ADC1): GPIO5

The SCD4x uses a large `temperature_offset` (10.7 C) to compensate for
self-heating inside the enclosure; tune it to your case against a trusted
thermometer once the enclosure has reached thermal equilibrium. The offset only
shifts the reported temperature and humidity, not the CO2 reading. To align the
CO2 value, see [CO2 calibration](#co2-calibration) below.

## CO2 calibration

The SCD4x has a factory accuracy of about +-(50 ppm + 5% of reading), so out of
the box it can read ~150 ppm away from another sensor and still be within spec.
Two Home Assistant controls (both in the device's **Configuration** section) let
you align it to a reference you trust:

- **CO2 recalibration target** (number, ppm): the known CO2 of the air the
  device is currently in.
- **CO2 forced recalibration** (button): writes that value to the sensor's
  baseline. This is a Sensirion forced recalibration (FRC); it is stored in the
  sensor, so it survives reboots.

Steps:

1. Put the device in air of a known CO2 level. Fresh outdoor air (~420 ppm) is
   the gold-standard reference; alternatively sit it beside a trusted sensor in
   still, well-mixed indoor air.
2. Leave it running there for at least ~5 minutes. Sensirion requires more than
   3 minutes of stable measurement before an FRC.
3. Set **CO2 recalibration target** to the reference value. Indoors that is the
   trusted sensor's current reading, not 420.
4. Press **CO2 forced recalibration**.

After the FRC the sensor restarts measurement, so give it a few minutes: CO2,
temperature and humidity all drift for a couple of minutes before they settle to
their final values, so do not judge the result from the first reading or two. A
one-off `scd4x.sensor took a long time for an operation (405 ms)` log line right
after pressing the button is expected (the FRC path blocks briefly by design)
and does not recur in normal operation.

Automatic self-calibration (ASC) is disabled here
(`automatic_self_calibration: false`). ASC assumes the sensor regularly sees
fresh ~400 ppm air and nudges its baseline to match; in a room that stays closed
(for example over winter) that assumption fails and ASC slowly drags the reading
low, undoing a manual calibration. With ASC off nothing auto-corrects, so re-run
the forced recalibration every few months to catch slow sensor aging. If the
room is well ventilated and regularly reaches outdoor air, set it back to `true`
and the manual step is not needed.

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
make setup                          # create .venv and install the pinned ESPHome
# ESPHome needs Python 3.12-3.14; override if your default python3 is outside
# that range, e.g. `make setup PYTHON=python3.12`.

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
- The SCD4x needs a few minutes after any restart (a reboot or a forced
  recalibration) for CO2, temperature and humidity to settle to steady state.

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
