#pragma once

#include <Wire.h>
#include <Arduino.h>
#include "XPowersLib.h"
#include <helpers/ESP32Board.h>
#include <driver/rtc_io.h>

// I2C address of the AXP2101 PMU (on the main Wire bus, PIN_BOARD_SDA/SCL)
#ifndef I2C_PMU_ADD
#define I2C_PMU_ADD  0x34
#endif

// The LilyGo T-Watch S3 family (T-Watch S3 and T-Watch S3 Plus) routes the LoRa
// radio, display and (on the Plus) GPS through an AXP2101 PMU. The radio will not
// power up until the correct rails are enabled, so this board class brings them up
// in begin() before radio_init() runs.
//
// Rail map (from LilyGo factory code, cross-checked against the user's Meshtastic
// T-Watch S3 Plus port):
//   ALDO2 -> screen / sensors / PCF8563 RTC  (standard S3: "must not be turned off")
//   ALDO3 -> LoRa radio (standard S3) / display+touch (Plus)
//   ALDO4 -> LoRa radio (Plus) / GPS (standard S3)
//   BLDO1 -> GPS (newer Plus)        DCDC3 -> GPS (older Plus)        BLDO2 -> haptic
// We enable ALDO2+ALDO3+ALDO4 unconditionally so the radio is powered on either the
// S3 (ALDO3) or Plus (ALDO4) wiring, and add the GPS rails only when GPS is built in.
class TWatchS3Board : public ESP32Board {
  XPowersLibInterface *PMU = NULL;

  bool power_init();

public:
  void begin();

  uint16_t getBattMilliVolts() override {
    return PMU ? PMU->getBattVoltage() : 0;
  }

  const char* getManufacturerName() const override {
    return "LilyGo T-Watch S3";
  }
};
