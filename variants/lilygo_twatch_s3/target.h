#pragma once

#define RADIOLIB_STATIC_ONLY 1
#include <RadioLib.h>
#include <helpers/radiolib/RadioLibWrappers.h>
#if defined(USE_SX1280)
  #include <helpers/radiolib/CustomSX1280Wrapper.h>
#else
  #include <helpers/radiolib/CustomSX1262Wrapper.h>
#endif
#include "TWatchS3Board.h"
#include <helpers/AutoDiscoverRTCClock.h>
#include <helpers/SensorManager.h>
#if defined(ENV_INCLUDE_GPS) && (ENV_INCLUDE_GPS)
  #include "helpers/sensors/EnvironmentSensorManager.h"
  #include "helpers/sensors/MicroNMEALocationProvider.h"
#endif
#ifdef DISPLAY_CLASS
  #include <helpers/ui/ST7789LCDDisplay.h>
  #include <helpers/ui/MomentaryButton.h>
#endif

extern TWatchS3Board board;
extern WRAPPER_CLASS radio_driver;
extern AutoDiscoverRTCClock rtc_clock;
#if defined(ENV_INCLUDE_GPS) && (ENV_INCLUDE_GPS)
  extern EnvironmentSensorManager sensors;
#else
  extern SensorManager sensors;
#endif

#ifdef DISPLAY_CLASS
  extern DISPLAY_CLASS display;
  extern MomentaryButton user_btn;
#endif

bool radio_init();
uint32_t radio_get_rng_seed();
void radio_set_params(float freq, float bw, uint8_t sf, uint8_t cr);
void radio_set_tx_power(int8_t dbm);
mesh::LocalIdentity radio_new_identity();
