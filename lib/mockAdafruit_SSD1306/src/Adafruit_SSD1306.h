#ifndef cartreader_Adafruit_SSD1306_h
#define cartreader_Adafruit_SSD1306_h

#include <inttypes.h>
#include <Wire.h>

class Adafruit_SSD1306 {
  public:
  Adafruit_SSD1306(uint8_t w, uint8_t h, TwoWire *twi, int8_t rst_pin = -1,
                   uint32_t clkDuring = 400000UL, uint32_t clkAfter = 100000UL);
  private:
  uint8_t m_width;
  uint8_t m_height;
};

#endif
