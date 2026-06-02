#if defined(LILYGO_TWATCH_S3)

#include <Arduino.h>
#include "TWatchS3Board.h"

void TWatchS3Board::begin() {
  ESP32Board::begin();   // sets startup_reason, battery pin, and Wire.begin(PIN_BOARD_SDA, PIN_BOARD_SCL)

  power_init();

#ifdef PIN_USER_BTN
  pinMode(PIN_USER_BTN, INPUT);
#endif
}

bool TWatchS3Board::power_init() {
  if (!PMU) {
    PMU = new XPowersAXP2101(Wire, PIN_BOARD_SDA, PIN_BOARD_SCL, I2C_PMU_ADD);
    if (!PMU->init()) {
      MESH_DEBUG_PRINTLN("Warning: Failed to find AXP2101 power management");
      delete PMU;
      PMU = NULL;
    } else {
      MESH_DEBUG_PRINTLN("AXP2101 PMU init succeeded");
    }
  }
  if (!PMU) {
    return false;
  }

  PMU->setChargingLedMode(XPOWERS_CHG_LED_CTRL_CHG);

  // --- Radio / display / RTC rails (enabled on both S3 and S3 Plus wiring) ---
  PMU->setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
  PMU->enablePowerOutput(XPOWERS_ALDO2);   // screen / sensors / PCF8563 RTC
  PMU->setPowerChannelVoltage(XPOWERS_ALDO3, 3300);
  PMU->enablePowerOutput(XPOWERS_ALDO3);   // LoRa radio (standard S3) / display+touch (Plus)
  PMU->setPowerChannelVoltage(XPOWERS_ALDO4, 3300);
  PMU->enablePowerOutput(XPOWERS_ALDO4);   // LoRa radio (Plus) / GPS (standard S3)

#if defined(ENV_INCLUDE_GPS) && (ENV_INCLUDE_GPS)
  // --- GPS rails (T-Watch S3 Plus) ---
  PMU->setPowerChannelVoltage(XPOWERS_BLDO1, 3300);
  PMU->enablePowerOutput(XPOWERS_BLDO1);   // GPS (newer Plus board)
  PMU->setPowerChannelVoltage(XPOWERS_DCDC3, 3300);
  PMU->enablePowerOutput(XPOWERS_DCDC3);   // GPS (older Plus board)
  PMU->setPowerChannelVoltage(XPOWERS_BLDO2, 3300);
  PMU->enablePowerOutput(XPOWERS_BLDO2);   // DRV2605 haptic enable
#else
  PMU->disablePowerOutput(XPOWERS_DCDC3);
  PMU->disablePowerOutput(XPOWERS_BLDO1);
  PMU->disablePowerOutput(XPOWERS_BLDO2);
#endif

  // --- Disable rails we never use ---
  PMU->disablePowerOutput(XPOWERS_DCDC2);
  PMU->disablePowerOutput(XPOWERS_DCDC5);
  PMU->disablePowerOutput(XPOWERS_DLDO1);
  PMU->disablePowerOutput(XPOWERS_DLDO2);
  PMU->disablePowerOutput(XPOWERS_ALDO1);
  // NOTE: DCDC1 powers the ESP32 itself and is auto-managed - never touch it.

  PMU->disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
  PMU->clearIrqStatus();

  PMU->setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_500MA);
  PMU->setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V2);

  PMU->disableTSPinMeasure();
  PMU->enableSystemVoltageMeasure();
  PMU->enableVbusVoltageMeasure();
  PMU->enableBattVoltageMeasure();

  return true;
}

#endif
