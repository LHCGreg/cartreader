#include "RGB_LED.h"
#include <avr/io.h>

// Set pins of red, green and blue
RGBTools rgb(12, 11, 10);

void rgbLed(byte color) {
  switch (color % 7) {
    case blue_color:
      rgb.setColor(0, 0, 255);
      break;
    case red_color:
      rgb.setColor(255, 0, 0);
      break;
    case purple_color:
      rgb.setColor(255, 0, 255);
      break;
    case green_color:
      rgb.setColor(0, 255, 0);
      break;
    case turquoise_color:
      rgb.setColor(0, 255, 255);
      break;
    case yellow_color:
      rgb.setColor(255, 255, 0);
      break;
    case white_color:
      rgb.setColor(255, 255, 255);
      break;
  }
}

void ledRedOff() {
  PORTB |= (1 << 6);
}

void ledRedOn() {
  PORTB &= ~(1 << 6);
}

void ledGreenOff() {
  PORTB |= (1 << 5);
}

void ledGreenOn() {
  PORTB &= ~(1 << 5);
}

void ledBlueOff() {
  PORTB |= (1 << 4);
}

void ledBlueOn() {
  PORTB &= ~(1 << 4);
}
