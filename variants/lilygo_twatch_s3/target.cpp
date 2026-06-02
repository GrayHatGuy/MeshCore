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

#ifndef LORA_CR
  #define LORA_CR      5
#endif

bool radio_init() {
  fallback_clock.begin();
  // PMU rails and the main I2C bus (Wire on PIN_BOARD_SDA/SCL) are brought up in
  // board.begin(); the PCF8563 RTC at 0x51 is auto-detected here.
  rtc_clock.begin(Wire);

  return radio.std_init(&spi);
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
