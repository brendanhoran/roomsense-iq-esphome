"""MPM10 I2C particulate-matter sensor used on RoomSense IQ boards.

The module is not a Plantower PMS. It is an I2C device at address 0x4D that
exposes PM1.0 / PM2.5 / PM10 as big-endian uint16 values, each split across two
consecutive 8-bit registers. The protocol was recovered from the original
ESP-IDF firmware (firmware/components/mpm10/mpm10.c) and reimplemented here.
"""

import esphome.codegen as cg

mpm10_ns = cg.esphome_ns.namespace("mpm10")
