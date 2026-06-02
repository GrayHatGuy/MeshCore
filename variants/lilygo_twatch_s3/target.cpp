#include <Arduino.h>
#include "target.h"

TWatchS3Board board;

static SPIClass spi;
RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, spi);
WRAPPER_CLASS radio_driver(radio, board);

ESP32RTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);

#if defined(ENV_INCLUDE_GPS) && (ENV_INCLUDE_GPS)
  MicroNMEALocationProvider gps(Serial1, &rtc_clock);
  EnvironmentSensorManager sensors(gps);
#else
  SensorManager sensors;
#endif

#ifdef DISPLAY_CLASS
  DISPLAY_CLASS display;
  MomentaryButton user_btn(PIN_USER_BTN, 1000, true);
#endif
#ifdef TOUCH_FT6X36
  TouchFT6X36 touch;
#endif

#ifndef LORA_CR
  #define LORA_CR      5
#endif

bool radio_init() {
  fallback_clock.begin();
  // PMU rails and the main I2C bus (Wire on PIN_BOARD_SDA/SCL) are brought up in
  // board.begin(); the PCF8563 RTC at 0x51 is auto-detected here.
  rtc_clock.begin(Wire);

#ifdef TOUCH_FT6X36
  // FT6336 capacitive touch on its own I2C bus (Wire1). ALDO3 (display+touch) is
  // already powered by board.begin(); this owns Wire1 (the env sensor manager only
  // claims Wire1 when ENV_PIN_SDA/SCL are defined, which the T-Watch does not).
  touch.begin(Wire1, PIN_TOUCH_SDA, PIN_TOUCH_SCL, TOUCH_I2C_ADDR, PIN_TOUCH_INT);
#endif

  // First boot right after flashing can run radio begin() before the SX1280's ALDO4
  // rail has fully settled (or with the reset line in a post-flash state), so
  // std_init() fails and main.cpp's halt() leaves the screen stuck on "Loading..."
  // until a manual RST. Retry a few times with a short settle/reset delay so the
  // cold boot self-heals without needing the reset button.
  for (int attempt = 0; attempt < 3; attempt++) {
    if (radio.std_init(&spi)) return true;
    delay(250);
  }
  return false;
}

uint32_t radio_get_rng_seed() {
  return radio.random(0x7FFFFFFF);
}

void radio_set_params(float freq, float bw, uint8_t sf, uint8_t cr) {
  radio.setFrequency(freq);
  radio.setSpreadingFactor(sf);
  radio.setBandwidth(bw);
  radio.setCodingRate(cr);
}

void radio_set_tx_power(int8_t dbm) {
  radio.setOutputPower(dbm);
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng); // create new random identity
}
