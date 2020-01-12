#ifndef cartreader_RGB_LED_h
#define cartreader_RGB_LED_h

#include <Arduino.h>
#include <RGBTools.h>

typedef enum COLOR_T {
  blue_color,
  red_color,
  purple_color,
  green_color,
  turquoise_color,
  yellow_color,
  white_color,
} color_t;

extern RGBTools rgb;

void rgbLed(byte Color);

#endif
