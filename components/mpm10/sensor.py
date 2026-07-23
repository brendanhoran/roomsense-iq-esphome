"""Sensor platform for the MFrontier MPM10 I2C particulate-matter sensor.

Exposes, all optional:
  - standard (CF=1) mass concentration:  pm1_0 / pm2_5 / pm10_0        (ug/m3)
  - atmospheric-environment mass:        pm1_0_atm / pm2_5_atm / ...   (ug/m3)
  - particle-number counts per 0.1 L:    pm_0_3um ... pm_10_0um        (count)

Register map is per the official MFrontier MPM10 datasheet
(hardware/MPM10-protocol-datasheet.pdf, "IIC register address and data format").
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_PM1,
    DEVICE_CLASS_PM10,
    DEVICE_CLASS_PM25,
    STATE_CLASS_MEASUREMENT,
    UNIT_MICROGRAMS_PER_CUBIC_METER,
)

from . import mpm10_ns

DEPENDENCIES = ["i2c"]

# Standard (CF=1) mass concentration
CONF_PM_1_0 = "pm1_0"
CONF_PM_2_5 = "pm2_5"
CONF_PM_10_0 = "pm10_0"
# Atmospheric-environment mass concentration
CONF_PM_1_0_ATM = "pm1_0_atm"
CONF_PM_2_5_ATM = "pm2_5_atm"
CONF_PM_10_0_ATM = "pm10_0_atm"
# Particle-number counts (per 0.1 L of air), by minimum particle diameter
CONF_PMC_0_3UM = "pm_0_3um"
CONF_PMC_0_5UM = "pm_0_5um"
CONF_PMC_1_0UM = "pm_1_0um"
CONF_PMC_2_5UM = "pm_2_5um"
CONF_PMC_5_0UM = "pm_5_0um"
CONF_PMC_10_0UM = "pm_10_0um"

# 1 dL = 0.1 L, the sample volume these counts are reported over.
UNIT_COUNTS_PER_DECILITRE = "particles/dL"
ICON_COUNTER = "mdi:counter"

MPM10Component = mpm10_ns.class_("MPM10Component", cg.PollingComponent, i2c.I2CDevice)


def _mass_schema(device_class):
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
        accuracy_decimals=0,
        device_class=device_class,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def _count_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_COUNTS_PER_DECILITRE,
        icon=ICON_COUNTER,
        accuracy_decimals=0,
        state_class=STATE_CLASS_MEASUREMENT,
    )


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MPM10Component),
            cv.Optional(CONF_PM_1_0): _mass_schema(DEVICE_CLASS_PM1),
            cv.Optional(CONF_PM_2_5): _mass_schema(DEVICE_CLASS_PM25),
            cv.Optional(CONF_PM_10_0): _mass_schema(DEVICE_CLASS_PM10),
            cv.Optional(CONF_PM_1_0_ATM): _mass_schema(DEVICE_CLASS_PM1),
            cv.Optional(CONF_PM_2_5_ATM): _mass_schema(DEVICE_CLASS_PM25),
            cv.Optional(CONF_PM_10_0_ATM): _mass_schema(DEVICE_CLASS_PM10),
            cv.Optional(CONF_PMC_0_3UM): _count_schema(),
            cv.Optional(CONF_PMC_0_5UM): _count_schema(),
            cv.Optional(CONF_PMC_1_0UM): _count_schema(),
            cv.Optional(CONF_PMC_2_5UM): _count_schema(),
            cv.Optional(CONF_PMC_5_0UM): _count_schema(),
            cv.Optional(CONF_PMC_10_0UM): _count_schema(),
        }
    )
    .extend(cv.polling_component_schema("30s"))
    .extend(i2c.i2c_device_schema(0x4D))
)

# config key -> C++ setter method name
_SETTERS = {
    CONF_PM_1_0: "set_pm_1_0_sensor",
    CONF_PM_2_5: "set_pm_2_5_sensor",
    CONF_PM_10_0: "set_pm_10_0_sensor",
    CONF_PM_1_0_ATM: "set_pm_1_0_atm_sensor",
    CONF_PM_2_5_ATM: "set_pm_2_5_atm_sensor",
    CONF_PM_10_0_ATM: "set_pm_10_0_atm_sensor",
    CONF_PMC_0_3UM: "set_pmc_0_3um_sensor",
    CONF_PMC_0_5UM: "set_pmc_0_5um_sensor",
    CONF_PMC_1_0UM: "set_pmc_1_0um_sensor",
    CONF_PMC_2_5UM: "set_pmc_2_5um_sensor",
    CONF_PMC_5_0UM: "set_pmc_5_0um_sensor",
    CONF_PMC_10_0UM: "set_pmc_10_0um_sensor",
}


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    for key, setter in _SETTERS.items():
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(var, setter)(sens))
