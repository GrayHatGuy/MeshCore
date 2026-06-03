#pragma once

// BMA423 accelerometer driver for the LilyGo T-Watch S3 family.
//
// Screen-wake = a clear movement of the watch (raw-accelerometer magnitude change).
// This matches a generic lift/raise gesture. The Bosch built-in wrist-tilt + double-tap
// feature engines are ALSO enabled (they fire on a worn-wrist glance rotation), so a
// worn glance wakes cleanly too -- but the raw-motion path is what catches a plain lift.
// INT1 on GPIO14. API: lewisxhe/SensorLib (SensorBMA423.hpp).

#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <SensorBMA423.hpp>

#ifndef MOTION_BMA423_ADDR
  #define MOTION_BMA423_ADDR 0x19      // BMA423 on the T-Watch main I2C bus (SDO low)
#endif
#ifndef MOTION_POLL_INTERVAL_MS
  #define MOTION_POLL_INTERVAL_MS 50   // accel / feature-status poll period
#endif
#ifndef MOTION_WAKE_FRAC
  #define MOTION_WAKE_FRAC 0.18f       // |accel| deviation from rest that counts as a "move" (tune)
#endif

class MotionBMA423 {
  SensorBMA423 _sensor;
  int _int_pin = -1;
  bool _ok = false;
  unsigned long _next_poll = 0;
  bool _tilt = false;
  bool _double_tap = false;
  bool _moved = false;
  bool _mag_init = false;
  float _mag_ema = 0;

public:
  bool begin(TwoWire& wire, int int_pin, uint8_t addr = MOTION_BMA423_ADDR) {
    _int_pin = int_pin;
    if (!_sensor.begin(wire, addr)) { _ok = false; return false; }
    _sensor.configAccelerometer(_sensor.RANGE_2G, _sensor.ODR_100HZ,
                                _sensor.BW_NORMAL_AVG4, _sensor.PERF_CONTINUOUS_MODE);
    _sensor.enableAccelerometer();
    _sensor.configInterrupt();
    if (_int_pin >= 0) pinMode((uint8_t)_int_pin, INPUT);

    // T-Watch S3 mounting orientation (same as LilyGo/Meshtastic).
    _sensor.setRemapAxes(_sensor.REMAP_TOP_LAYER_RIGHT_CORNER);
    _sensor.enableFeature(_sensor.FEATURE_TILT, true);     // Bosch wrist-tilt (worn glance)
    _sensor.enableFeature(_sensor.FEATURE_WAKEUP, true);   // double-tap
    _sensor.enableTiltIRQ();
    _sensor.enableWakeupIRQ();

    _ok = true;
    return true;
  }

  bool isReady() const { return _ok; }
  int  intPin()  const { return _int_pin; }

  void loop() {
    if (!_ok) return;
    unsigned long now = millis();
    if ((long)(now - _next_poll) < 0) return;
    _next_poll = now + MOTION_POLL_INTERVAL_MS;

    // Bosch feature interrupts (worn-wrist glance / double-tap).
    if (_sensor.readIrqStatus()) {
      if (_sensor.isTilt())      _tilt = true;
      if (_sensor.isDoubleTap()) _double_tap = true;
    }

    // Raw-motion wake: a clear change in acceleration magnitude = the watch moved.
    int16_t x, y, z;
    if (_sensor.getAccelerometer(x, y, z)) {
      float mag = sqrtf((float)x*x + (float)y*y + (float)z*z);
      if (!_mag_init) { _mag_ema = mag; _mag_init = true; }
      float ref = (_mag_ema > 1.0f) ? _mag_ema : 1.0f;
      if (fabsf(mag - _mag_ema) / ref > MOTION_WAKE_FRAC) _moved = true;
      _mag_ema += (mag - _mag_ema) * 0.3f;   // track the resting magnitude slowly
    }
  }

  bool hasWakeGesture() const { return _tilt || _double_tap || _moved; }
  bool consumeTilt()      { bool v = _tilt; _tilt = false; return v; }
  bool consumeDoubleTap() { bool v = _double_tap; _double_tap = false; return v; }
  void clear() { _tilt = _double_tap = _moved = false; }
};
