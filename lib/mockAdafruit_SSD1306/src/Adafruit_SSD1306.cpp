#include "Adafruit_SSD1306.h"
#include <Wire.h>

Adafruit_SSD1306::Adafruit_SSD1306(uint8_t w, uint8_t h, TwoWire *twi, int8_t rst_pin, uint32_t clkDuring, uint32_t clkAfter) {
  m_width = w;
  m_height = h;
}
