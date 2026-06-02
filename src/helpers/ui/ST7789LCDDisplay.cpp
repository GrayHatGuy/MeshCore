#include "ST7789LCDDisplay.h"

#ifndef DISPLAY_ROTATION
  #define DISPLAY_ROTATION 3
#endif

#ifndef DISPLAY_SCALE_X
  #define DISPLAY_SCALE_X 2.5f // 320 / 128
#endif

#ifndef DISPLAY_SCALE_Y
  #define DISPLAY_SCALE_Y 3.75f // 240 / 64
#endif

// Font multiplier, decoupled from the layout scale so small/square panels (e.g. the
// 240x240 T-Watch, where SCALE_X must be 1.875 to fit/centre) can still use a larger,
// readable integer text size. Defaults to DISPLAY_SCALE_X -> unchanged for T-Deck/etc.
#ifndef DISPLAY_TEXT_SCALE
  #define DISPLAY_TEXT_SCALE DISPLAY_SCALE_X
#endif

#ifndef DISPLAY_WIDTH
  #define DISPLAY_WIDTH 240
#endif
#ifndef DISPLAY_HEIGHT
  #define DISPLAY_HEIGHT 320
#endif

#ifdef DISPLAY_USE_GFX_FONT
  // Proportional GFX font: readable AND fits more chars than the integer-scaled built-in
  // 6x8 font on small/dense screens (e.g. the 240x240 T-Watch 2-column data screens).
  // Gated so other ST7789 boards (T-Deck, Heltec_v4) keep the built-in font.
  #include <Fonts/FreeSans9pt7b.h>
  #define DISPLAY_GFX_FONT (&FreeSans9pt7b)
  #ifndef DISPLAY_FONT_BASELINE
    #define DISPLAY_FONT_BASELINE 13   // px from logical top to baseline at text size 1 (tune on hw)
  #endif
#endif

bool ST7789LCDDisplay::i2c_probe(TwoWire& wire, uint8_t addr) {
  return true;
}

bool ST7789LCDDisplay::begin() {
  if (!_isOn) {
    if (_peripher_power) _peripher_power->claim();

    if (PIN_TFT_LEDA_CTL != -1) {
      pinMode(PIN_TFT_LEDA_CTL, OUTPUT);
      digitalWrite(PIN_TFT_LEDA_CTL, HIGH);
    }
    if (PIN_TFT_RST != -1) {
      pinMode(PIN_TFT_RST, OUTPUT);
      digitalWrite(PIN_TFT_RST, LOW); 
      delay(10);
      digitalWrite(PIN_TFT_RST, HIGH);
    }

    // Im not sure if this is just a t-deck problem or not, if your display is slow try this.
    #if defined(LILYGO_TDECK) || defined(HELTEC_LORA_V4_TFT) || defined(LILYGO_TWATCH_S3)
      displaySPI.begin(PIN_TFT_SCL, -1, PIN_TFT_SDA, PIN_TFT_CS);
    #endif

    display.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    display.setRotation(DISPLAY_ROTATION);

    display.setSPISpeed(40e6);

    display.fillScreen(ST77XX_BLACK);
    display.setTextColor(ST77XX_WHITE);
#ifdef DISPLAY_USE_GFX_FONT
    display.setFont(DISPLAY_GFX_FONT);
    _textsize = 2;
    display.setTextSize(2);
#else
    display.setTextSize(2 * DISPLAY_TEXT_SCALE);
    display.cp437(true); // Use full 256 char 'Code Page 437' font
#endif

    _isOn = true;
  }

  return true;
}

void ST7789LCDDisplay::turnOn() {
  ST7789LCDDisplay::begin();
}

void ST7789LCDDisplay::turnOff() {
  if (_isOn) {
    if (PIN_TFT_LEDA_CTL != -1) {
      digitalWrite(PIN_TFT_LEDA_CTL, HIGH);
    }
    if (PIN_TFT_RST != -1) {
      digitalWrite(PIN_TFT_RST, LOW);
    }
    if (PIN_TFT_LEDA_CTL != -1) {
      digitalWrite(PIN_TFT_LEDA_CTL, LOW);
    }
    _isOn = false;

    if (_peripher_power) _peripher_power->release();
  }
}

void ST7789LCDDisplay::clear() {
  display.fillScreen(ST77XX_BLACK);
}

void ST7789LCDDisplay::startFrame(Color bkg) {
  display.fillScreen(ST77XX_BLACK);
  display.setTextColor(ST77XX_WHITE);
#ifdef DISPLAY_USE_GFX_FONT
  display.setFont(DISPLAY_GFX_FONT);
  _textsize = 1;
  display.setTextSize(1);
#else
  display.setTextSize(1 * DISPLAY_TEXT_SCALE); // base text size (Please wait... etc.)
  display.cp437(true); // Use full 256 char 'Code Page 437' font
#endif
}

void ST7789LCDDisplay::setTextSize(int sz) {
  _textsize = sz;
#ifdef DISPLAY_USE_GFX_FONT
  display.setTextSize(sz);                    // proportional font, native size x sz
#else
  display.setTextSize(sz * DISPLAY_TEXT_SCALE);
#endif
}

void ST7789LCDDisplay::setColor(Color c) {
  switch (c) {
    case DisplayDriver::DARK :
      _color = ST77XX_BLACK;
      break;
    case DisplayDriver::LIGHT : 
      _color = ST77XX_WHITE;
      break;
    case DisplayDriver::RED : 
      _color = ST77XX_RED;
      break;
    case DisplayDriver::GREEN : 
      _color = ST77XX_GREEN;
      break;
    case DisplayDriver::BLUE : 
      _color = ST77XX_BLUE;
      break;
    case DisplayDriver::YELLOW : 
      _color = ST77XX_YELLOW;
      break;
    case DisplayDriver::ORANGE : 
      _color = ST77XX_ORANGE;
      break;
    default:
      _color = ST77XX_WHITE;
      break;
  }
  display.setTextColor(_color);
}

void ST7789LCDDisplay::setCursor(int x, int y) {
#ifdef DISPLAY_USE_GFX_FONT
  // GFX custom fonts anchor text at its baseline; shift down by the ascent so the UI's
  // top-left (x, y) lands where the built-in-font layout expects.
  display.setCursor(x * DISPLAY_SCALE_X, y * DISPLAY_SCALE_Y + DISPLAY_FONT_BASELINE * _textsize);
#else
  display.setCursor(x * DISPLAY_SCALE_X, y * DISPLAY_SCALE_Y);
#endif
}

void ST7789LCDDisplay::print(const char* str) {
  display.print(str);
}

void ST7789LCDDisplay::fillRect(int x, int y, int w, int h) {
  display.fillRect(x * DISPLAY_SCALE_X, y * DISPLAY_SCALE_Y, w * DISPLAY_SCALE_X, h * DISPLAY_SCALE_Y, _color);
}

void ST7789LCDDisplay::drawRect(int x, int y, int w, int h) {
  display.drawRect(x * DISPLAY_SCALE_X, y * DISPLAY_SCALE_Y, w * DISPLAY_SCALE_X, h * DISPLAY_SCALE_Y, _color);
}

void ST7789LCDDisplay::drawXbm(int x, int y, const uint8_t* bits, int w, int h) {
  uint8_t byteWidth = (w + 7) / 8;

  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      uint8_t byte = bits[j * byteWidth + i / 8];
      bool pixelOn = byte & (0x80 >> (i & 7));

      if (pixelOn) {
        for (int dy = 0; dy < DISPLAY_SCALE_X; dy++) {
          for (int dx = 0; dx < DISPLAY_SCALE_X; dx++) {
            display.drawPixel(x * DISPLAY_SCALE_X + i * DISPLAY_SCALE_X + dx, y * DISPLAY_SCALE_Y + j * DISPLAY_SCALE_X + dy, _color);
          }
        }
      }
    }
  }
}

uint16_t ST7789LCDDisplay::getTextWidth(const char* str) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);

  // Divide by the LAYOUT scale (not the text scale): the UI lays out in logical units
  // where 1 unit = DISPLAY_SCALE_X px, so the string's logical footprint must use the
  // same scale or centered/positioned text overruns the panel edge.
  return w / DISPLAY_SCALE_X;
}

void ST7789LCDDisplay::endFrame() {
  // display.display();
}