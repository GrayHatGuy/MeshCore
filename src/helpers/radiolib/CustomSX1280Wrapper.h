#pragma once

#include "CustomSX1280.h"
#include "RadioLibWrappers.h"

#ifndef USE_SX1280
#define USE_SX1280
#endif

class CustomSX1280Wrapper : public RadioLibWrapper {
public:
  CustomSX1280Wrapper(CustomSX1280& radio, mesh::MainBoard& board) : RadioLibWrapper(radio, board) { }
  bool isReceivingPacket() override {
    return ((CustomSX1280 *)_radio)->isReceiving();
  }
  float getCurrentRSSI() override {
    return ((CustomSX1280 *)_radio)->getRSSI(false);   // instantaneous RSSI
  }
  float getLastRSSI() const override { return ((CustomSX1280 *)_radio)->getRSSI(); }
  float getLastSNR() const override { return ((CustomSX1280 *)_radio)->getSNR(); }

  virtual void powerOff() override {
    ((CustomSX1280 *)_radio)->sleep();
  }

  // SX128x has no SX126x-style AGC reset register; returning to standby is sufficient.
  void doResetAGC() override { ((CustomSX1280 *)_radio)->standby(); }
};
