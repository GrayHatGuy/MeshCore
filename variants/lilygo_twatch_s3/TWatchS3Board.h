#pragma once

#include <Wire.h>
#include <Arduino.h>
#include "XPowersLib.h"
#include <helpers/ESP32Board.h>
#include <driver/rtc_io.h>

// Forward declaration; the full BMA423 driver header (and SensorLib) is pulled into
// the .cpp only, so it doesn't leak into every TU that includes this board header.
class MotionBMA423;

// I2C address of the AXP2101 PMU (on the main Wire bus, PIN_BOARD_SDA/SCL)
#ifndef I2C_PMU_ADD
#define I2C_PMU_ADD  0x34
#endif

// The LilyGo T-Watch S3 family (T-Watch S3 and T-Watch S3 Plus) routes the LoRa
// radio, display and (on the Plus) GPS through an AXP2101 PMU. The radio will not
// power up until the correct rails are enabled, so this board class brings them up
// in begin() before radio_init() runs.
//
// Rail map (LilyGo factory LilyGoWatchS3.cpp; confirmed on a T-Watch S3 Plus):
//   ALDO2 -> TFT backlight (LCD_VDD)
//   ALDO3 -> ST7789 display + FT6336 touch panel
//   ALDO4 -> LoRa radio (SX1262 / SX1280)   <-- must stay ON at all times
//   BLDO1 -> GPS (newer Plus)   DCDC3 -> GPS (older Plus)   BLDO2 -> haptic
// We enable ALDO2+ALDO3+ALDO4 for the whole session and add the GPS rails only when
// GPS is built in. ALDO4 (radio) is never disabled while running: cutting it drops
// the SX1262/SX1280 and corrupts radio state (begin() would then fail/misbehave).
class TWatchS3Board : public ESP32Board {
  XPowersLibInterface *PMU = NULL;
#if defined(MOTION_BMA423) && (MOTION_BMA423)
  MotionBMA423* _motion = NULL;   // wrist-tilt + double-tap wake gestures (task #18)
#endif

  bool power_init();

public:
  void begin();

#if defined(MOTION_BMA423) && (MOTION_BMA423)
  void loop() override;                                  // polls the BMA423 each cycle
  void* getMotionSensor() override { return _motion; }   // MotionBMA423* (for the UI calibration page)
#endif

  uint16_t getBattMilliVolts() override {
    return PMU ? PMU->getBattVoltage() : 0;
  }

  const char* getManufacturerName() const override {
    return "LilyGo T-Watch S3";
  }
};
