#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "MomentaryButton.h"   // reuse the BUTTON_EVENT_* constants

// Minimal FT6336 / FT6x36 capacitive-touch reader (e.g. LilyGo T-Watch S3).
//
// It deliberately exposes a MomentaryButton-compatible check() that returns
// BUTTON_EVENT_CLICK / BUTTON_EVENT_LONG_PRESS, so the ui-new input loop can treat a
// tap like a button click and a touch-and-hold like a long press -- mapped to the
// same key codes as the physical button (tap = next page / wake; hold = enter).
//
// Only the TD_STATUS register (touch present / absent) is read; X/Y coordinates are
// not needed for tap / long-press page navigation. The panel sits on its own I2C bus
// (Wire1 on the T-Watch S3: SDA 39 / SCL 40, addr 0x38), separate from the AXP2101 /
// RTC bus, so begin() initialises that bus itself.
class TouchFT6X36 {
  TwoWire* _wire = nullptr;
  uint8_t  _addr = 0x38;
  int8_t   _int_pin = -1;
  uint16_t _long_ms = 1200;
  bool     _down = false;
  bool     _long_fired = false;
  unsigned long _down_at = 0;
  unsigned long _last_read = 0;

  bool isTouchedRaw() {
    // FT6x36 reg 0x02 (TD_STATUS): low nibble = number of active touch points
    _wire->beginTransmission(_addr);
    _wire->write((uint8_t)0x02);
    if (_wire->endTransmission(false) != 0) return false;   // NACK / bus error
    if (_wire->requestFrom((int)_addr, 1) != 1) return false;
    return (_wire->read() & 0x0F) > 0;
  }

public:
  void begin(TwoWire& wire, int sda, int scl, uint8_t addr = 0x38,
             int int_pin = -1, uint16_t long_press_ms = 1200) {
    _wire = &wire;
    _addr = addr;
    _int_pin = (int8_t)int_pin;
    _long_ms = long_press_ms;
    _wire->begin(sda, scl, 400000);
    if (_int_pin >= 0) pinMode(_int_pin, INPUT_PULLUP);
  }

  // Returns one of BUTTON_EVENT_* (NONE / CLICK / LONG_PRESS). Throttled to ~15 ms so
  // it can be polled every UITask::loop() iteration without flooding the I2C bus.
  int check() {
    if (_wire == nullptr) return BUTTON_EVENT_NONE;
    unsigned long now = millis();
    if (now - _last_read < 15) return BUTTON_EVENT_NONE;
    _last_read = now;

    bool touched = isTouchedRaw();
    if (touched) {
      if (!_down) {
        _down = true;
        _long_fired = false;
        _down_at = now;
      } else if (!_long_fired && (now - _down_at) >= _long_ms) {
        _long_fired = true;
        return BUTTON_EVENT_LONG_PRESS;   // fire once while still held
      }
    } else if (_down) {
      _down = false;
      if (!_long_fired) return BUTTON_EVENT_CLICK;   // short tap on release
    }
    return BUTTON_EVENT_NONE;
  }

  bool isTouched() const { return _down; }
};
