#if defined(LILYGO_TWATCH_S3)

#include <Arduino.h>
#include "TWatchS3Board.h"

void TWatchS3Board::begin() {
  ESP32Board::begin();   // sets startup_reason, battery pin, and Wire.begin(PIN_BOARD_SDA, PIN_BOARD_SCL)

  power_init();

#if defined(TWATCH_INHIBIT_SLEEP) && (TWATCH_INHIBIT_SLEEP)
  // Never enter light sleep: keeps the CPU -- and therefore every AXP2101 rail,
  // especially ALDO4 (radio) -- continuously powered, so the SX1280 is never put
  // through a sleep/wake cycle that can corrupt its state or disrupt begin(). Trades
  // battery for radio stability; clear the TWATCH_INHIBIT_SLEEP flag to re-enable
  // MeshCore's periodic light sleep once SX1280 sleep/wake is sorted.
  setInhibitSleep(true);
#endif

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

  // --- Peripheral rails (LilyGo factory rail table) ---
  // ALDO4 (radio) MUST stay on for the whole session: cutting it drops the
  // SX1262/SX1280 and corrupts radio state. Light sleep does not touch these rails,
  // and begin() additionally inhibits sleep when TWATCH_INHIBIT_SLEEP is set.
  PMU->setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
  PMU->enablePowerOutput(XPOWERS_ALDO2);   // TFT backlight (LCD_VDD)
  PMU->setPowerChannelVoltage(XPOWERS_ALDO3, 3300);
  PMU->enablePowerOutput(XPOWERS_ALDO3);   // ST7789 display + FT6336 touch
  PMU->setPowerChannelVoltage(XPOWERS_ALDO4, 3300);
  PMU->enablePowerOutput(XPOWERS_ALDO4);   // LoRa radio (SX1262 / SX1280) -- KEEP ON

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
