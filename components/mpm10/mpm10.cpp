#include "mpm10.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mpm10 {

static const char *const TAG = "mpm10";

// IIC register map (base / high-byte register; low byte is base + 1).
// Standard (CF=1) mass concentration, ug/m3:
static const uint8_t REG_PM_1_0 = 0x20;
static const uint8_t REG_PM_2_5 = 0x22;
static const uint8_t REG_PM_10_0 = 0x24;
// Atmospheric-environment mass concentration, ug/m3:
static const uint8_t REG_PM_1_0_ATM = 0x26;
static const uint8_t REG_PM_2_5_ATM = 0x28;
static const uint8_t REG_PM_10_0_ATM = 0x2A;
// Particle-number counts. The datasheet's per-row text is garbled (it prints
// the diameter thresholds 0.3/0.5/1.0/2.5/5.0/10 in the sample-volume column),
// but the register order and the Plantower-family protocol make the meaning
// unambiguous: the count of particles larger than the given diameter per 0.1 L.
static const uint8_t REG_PMC_0_3UM = 0x2C;
static const uint8_t REG_PMC_0_5UM = 0x2E;
static const uint8_t REG_PMC_1_0UM = 0x30;
static const uint8_t REG_PMC_2_5UM = 0x32;
static const uint8_t REG_PMC_5_0UM = 0x34;
static const uint8_t REG_PMC_10_0UM = 0x36;

bool MPM10Component::read_pm_value_(uint8_t base_reg, uint16_t *out) {
  uint8_t high = 0;
  uint8_t low = 0;
  // The module returns a single byte per register, so read the two bytes
  // separately rather than as a 16-bit block.
  if (!this->read_byte(base_reg, &high) || !this->read_byte(static_cast<uint8_t>(base_reg + 1), &low)) {
    return false;
  }
  *out = (static_cast<uint16_t>(high) << 8) | low;
  return true;
}

void MPM10Component::publish_reg_(uint8_t base_reg, sensor::Sensor *sensor, bool *ok) {
  if (sensor == nullptr) {
    return;
  }
  uint16_t value = 0;
  if (this->read_pm_value_(base_reg, &value)) {
    sensor->publish_state(value);
  } else {
    *ok = false;
  }
}

void MPM10Component::update() {
  bool ok = true;

  this->publish_reg_(REG_PM_1_0, this->pm_1_0_sensor_, &ok);
  this->publish_reg_(REG_PM_2_5, this->pm_2_5_sensor_, &ok);
  this->publish_reg_(REG_PM_10_0, this->pm_10_0_sensor_, &ok);

  this->publish_reg_(REG_PM_1_0_ATM, this->pm_1_0_atm_sensor_, &ok);
  this->publish_reg_(REG_PM_2_5_ATM, this->pm_2_5_atm_sensor_, &ok);
  this->publish_reg_(REG_PM_10_0_ATM, this->pm_10_0_atm_sensor_, &ok);

  this->publish_reg_(REG_PMC_0_3UM, this->pmc_0_3um_sensor_, &ok);
  this->publish_reg_(REG_PMC_0_5UM, this->pmc_0_5um_sensor_, &ok);
  this->publish_reg_(REG_PMC_1_0UM, this->pmc_1_0um_sensor_, &ok);
  this->publish_reg_(REG_PMC_2_5UM, this->pmc_2_5um_sensor_, &ok);
  this->publish_reg_(REG_PMC_5_0UM, this->pmc_5_0um_sensor_, &ok);
  this->publish_reg_(REG_PMC_10_0UM, this->pmc_10_0um_sensor_, &ok);

  if (ok) {
    this->status_clear_warning();
  } else {
    ESP_LOGW(TAG, "Failed to read from MPM10 at address 0x%02X", this->address_);
    this->status_set_warning();
  }
}

void MPM10Component::dump_config() {
  ESP_LOGCONFIG(TAG, "MPM10 particulate-matter sensor:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "PM1.0", this->pm_1_0_sensor_);
  LOG_SENSOR("  ", "PM2.5", this->pm_2_5_sensor_);
  LOG_SENSOR("  ", "PM10", this->pm_10_0_sensor_);
  LOG_SENSOR("  ", "PM1.0 (atmospheric)", this->pm_1_0_atm_sensor_);
  LOG_SENSOR("  ", "PM2.5 (atmospheric)", this->pm_2_5_atm_sensor_);
  LOG_SENSOR("  ", "PM10 (atmospheric)", this->pm_10_0_atm_sensor_);
  LOG_SENSOR("  ", "Particles >0.3um", this->pmc_0_3um_sensor_);
  LOG_SENSOR("  ", "Particles >0.5um", this->pmc_0_5um_sensor_);
  LOG_SENSOR("  ", "Particles >1.0um", this->pmc_1_0um_sensor_);
  LOG_SENSOR("  ", "Particles >2.5um", this->pmc_2_5um_sensor_);
  LOG_SENSOR("  ", "Particles >5.0um", this->pmc_5_0um_sensor_);
  LOG_SENSOR("  ", "Particles >10um", this->pmc_10_0um_sensor_);
}

}  // namespace mpm10
}  // namespace esphome
