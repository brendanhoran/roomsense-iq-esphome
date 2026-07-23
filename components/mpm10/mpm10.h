#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace mpm10 {

/// Driver for the MFrontier MPM10 laser particulate-matter sensor (I2C mode,
/// address 0x4D) fitted to RoomSense IQ boards.
///
/// Every value is a big-endian uint16: high byte at the base register, low byte
/// at base + 1. The register map matches the official MFrontier MPM10 datasheet
/// ("IIC register address and data format", hardware/MPM10-protocol-datasheet.pdf):
///   Standard (CF=1) mass ug/m3:   PM1.0 0x20, PM2.5 0x22, PM10 0x24
///   Atmospheric mass ug/m3:       PM1.0 0x26, PM2.5 0x28, PM10 0x2A
///   Particle counts (per 0.1 L):  >0.3um 0x2C, >0.5um 0x2E, >1.0um 0x30,
///                                 >2.5um 0x32, >5.0um 0x34, >10um 0x36
/// All sensors are optional; only configured ones are read and published.
class MPM10Component : public PollingComponent, public i2c::I2CDevice {
 public:
  // Standard (CF=1) mass concentration
  void set_pm_1_0_sensor(sensor::Sensor *s) { this->pm_1_0_sensor_ = s; }
  void set_pm_2_5_sensor(sensor::Sensor *s) { this->pm_2_5_sensor_ = s; }
  void set_pm_10_0_sensor(sensor::Sensor *s) { this->pm_10_0_sensor_ = s; }
  // Atmospheric-environment mass concentration
  void set_pm_1_0_atm_sensor(sensor::Sensor *s) { this->pm_1_0_atm_sensor_ = s; }
  void set_pm_2_5_atm_sensor(sensor::Sensor *s) { this->pm_2_5_atm_sensor_ = s; }
  void set_pm_10_0_atm_sensor(sensor::Sensor *s) { this->pm_10_0_atm_sensor_ = s; }
  // Particle-number counts (per 0.1 L)
  void set_pmc_0_3um_sensor(sensor::Sensor *s) { this->pmc_0_3um_sensor_ = s; }
  void set_pmc_0_5um_sensor(sensor::Sensor *s) { this->pmc_0_5um_sensor_ = s; }
  void set_pmc_1_0um_sensor(sensor::Sensor *s) { this->pmc_1_0um_sensor_ = s; }
  void set_pmc_2_5um_sensor(sensor::Sensor *s) { this->pmc_2_5um_sensor_ = s; }
  void set_pmc_5_0um_sensor(sensor::Sensor *s) { this->pmc_5_0um_sensor_ = s; }
  void set_pmc_10_0um_sensor(sensor::Sensor *s) { this->pmc_10_0um_sensor_ = s; }

  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  /// Read one big-endian uint16 that spans base_reg (high) and base_reg + 1
  /// (low). Returns false on any I2C error.
  bool read_pm_value_(uint8_t base_reg, uint16_t *out);

  /// If sensor is set, read base_reg and publish it; clear *ok on read failure.
  void publish_reg_(uint8_t base_reg, sensor::Sensor *sensor, bool *ok);

  sensor::Sensor *pm_1_0_sensor_{nullptr};
  sensor::Sensor *pm_2_5_sensor_{nullptr};
  sensor::Sensor *pm_10_0_sensor_{nullptr};
  sensor::Sensor *pm_1_0_atm_sensor_{nullptr};
  sensor::Sensor *pm_2_5_atm_sensor_{nullptr};
  sensor::Sensor *pm_10_0_atm_sensor_{nullptr};
  sensor::Sensor *pmc_0_3um_sensor_{nullptr};
  sensor::Sensor *pmc_0_5um_sensor_{nullptr};
  sensor::Sensor *pmc_1_0um_sensor_{nullptr};
  sensor::Sensor *pmc_2_5um_sensor_{nullptr};
  sensor::Sensor *pmc_5_0um_sensor_{nullptr};
  sensor::Sensor *pmc_10_0um_sensor_{nullptr};
};

}  // namespace mpm10
}  // namespace esphome
