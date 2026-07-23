# Hardware reference

Datasheets and the board schematic for the RoomSense IQ (ESP32-S3-WROOM-1), so
the repo is self-contained. Source URLs are listed for provenance.

## Board

| File | What it is |
|------|------------|
| `Roomsense-IQ--Schematic-Rev8.pdf` | Board schematic (RoomSense Labs, Rev 8). Source of the pin map. |

## Sensors

| File | Sensor | Role | Source |
|------|--------|------|--------|
| `MPM10-datasheet.pdf` | MFrontier MPM10 | PM1/PM2.5/PM10 (I2C 0x4D) | 2-page overview (specs, features) |
| `MPM10-protocol-datasheet.pdf` | MFrontier MPM10 | as above | 11-page datasheet: full IIC register map, UART frame, PWM. What the custom `mpm10` component implements. |
| `SCD4x-datasheet.pdf` | Sensirion SCD4x | CO2, temperature, humidity (I2C 0x62) | https://sensirion.com/media/documents/48C4B7FB/67FE0194/CD_DS_SCD4x_Datasheet_D1.pdf |
| `SGP41-datasheet.pdf` | Sensirion SGP41 | VOC + NOx index (I2C 0x59) | https://sensirion.com/media/documents/5FE8673C/61E96F50/Sensirion_Gas_Sensors_Datasheet_SGP41.pdf |
| `LD2410C-datasheet.pdf` | Hi-Link HLK-LD2410 | 24 GHz mmWave presence radar (UART) | https://d.hlktech.net/download/HLK-LD2410C/1/HLK%20LD2410C%20Human%20%20Presence%20Sensor%20Module%20Data%20Sheet%20V1.00.pdf |
| `LD2410C-serial-protocol.pdf` | Hi-Link HLK-LD2410 | serial protocol | https://d.hlktech.net/download/HLK-LD2410C/1/HLK-LD2410C%20Serial%20communication%20protocol%20V1.07.pdf |

## Notes

- The SCD4x, SGP41 and LD2410 have native ESPHome components, so their protocols
  are already implemented upstream; their datasheets are here for reference only.
  The MPM10 is the one this repo implements directly (`components/mpm10/`).
- The LD2410 documents are for the LD2410C variant (Hi-Link only hosts direct
  PDFs for that one); the serial protocol is common across LD2410 / B / C.
- The MPM10 protocol datasheet confirms our register map: 7-bit I2C address
  `0x4d`; PM1.0/PM2.5/PM10 as big-endian uint16 at `0x20`/`0x22`/`0x24`.
