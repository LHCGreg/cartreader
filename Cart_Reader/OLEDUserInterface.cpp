#include "options.h"

#if defined(enable_OLED) && !defined(UNIT_TEST)

#include "OLEDUserInterface.h"
#include "RGB_LED.h"
#include "globals.h"
#include "utils.h"

/******************************************
  OLED Menu Module
*****************************************/

static const uint8_t PROGMEM icon[] = {
  0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF,
  0xFF, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0xF8, 0x00, 0x00, 0x0F, 0xFF,
  0x00, 0x00, 0x0F, 0xFF, 0xF8, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0xF8, 0x00, 0x00,
  0x0F, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0xF8, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0xF8,
  0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0xF8, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00,
  0x00, 0xFF, 0x80, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0xFF, 0x80, 0x0F, 0x00, 0x0F, 0xFF,
  0xFF, 0xFF, 0xF0, 0x0F, 0x80, 0x0F, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0x80, 0x0F, 0x00,
  0x0F, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0x80, 0x0F, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0x80,
  0xFF, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x80, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x0F, 0xF0, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0xF0, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0x0F, 0xF0, 0xF0, 0x0F, 0xFF, 0xF0, 0x00, 0xFF, 0xFF, 0x00, 0xF0, 0xF0, 0x0F, 0xFF,
  0xF0, 0x00, 0xFF, 0xFF, 0x00, 0xF0, 0xF0, 0x0F, 0xFF, 0xF0, 0x00, 0xFF, 0xFF, 0x00, 0xF0, 0xF0,
  0x0F, 0xFF, 0xF0, 0x00, 0xFF, 0xFF, 0x00, 0xF0, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00,
  0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF,
  0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0,
  0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00,
  0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F,
  0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0,
  0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00,
  0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0xF0, 0x0F, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0xF0, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0xF0,
  0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x1F, 0xF0, 0xFF, 0xF0,
  0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x1F, 0xF0, 0xFF, 0xF0, 0xFF, 0x0F, 0xFF, 0x0F, 0xF0, 0x1F, 0xF0,
  0xFF, 0xF0, 0xFF, 0x0F, 0xFF, 0x0F, 0xF0, 0x1F, 0xF0, 0xFF, 0xF0, 0xFF, 0x0F, 0xFF, 0x0F, 0xF0,
  0x01, 0xF0, 0xFF, 0xF0, 0xFF, 0x0F, 0xFF, 0x0F, 0x80, 0x01, 0xF0, 0xFF, 0xF0, 0xFF, 0x0F, 0xFF,
  0x0F, 0x80, 0x01, 0xF0, 0xFF, 0xF0, 0xFF, 0x0F, 0xFF, 0x0F, 0x80, 0x01, 0xF0, 0xFF, 0xF0, 0xFF,
  0x0F, 0xFF, 0x0F, 0x80, 0x01, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x80, 0x01, 0xF0, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x80, 0x01, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x80, 0x01,
  0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x80, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F,
  0x80, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0F, 0x80, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x01, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x01, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80
};

void OLEDUserInterface::initialize() {
  // GLCD
  m_display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  m_display.setTextSize(1);
  m_display.setTextColor(WHITE);

  // Clear the screen buffer.
  clearOutput();

#if !defined(fast_start) && !defined(UNIT_TEST)
  delay(100);

  // Draw line
  display.drawLine(0, 32, 127, 32, WHITE);
  flushOutput();
  delay(100);

  // Initialize LED
  rgb.setColor(0, 0, 0);

  // Clear the screen.
  clearOutput();
  flushOutput();
  delay(25);

  // Draw the Logo
  display.drawBitmap(28, 0, icon, 72, 64, 1);
  for (int s = 1; s < 64; s += 2) {
    // Draw Scanlines
    display.drawLine(0, s, 127, s, BLACK);
  }
  flushOutput();
  delay(50);

  // Clear the screen.
  clearOutput();
  flushOutput();
  delay(25);

  // Draw the Logo
  display.drawBitmap(28, 0, icon, 72, 64, 1);
  for (int s = 1; s < 64; s += 2) {
    // Draw Scanlines
    display.drawLine(0, s, 127, s, BLACK);
  }
  display.setCursor(100, 55);
  display.println(ver);
  flushOutput();
  delay(200);
#endif
}

// Button timing
const uint16_t debounceMs = 20;       // ms debounce period to prevent flickering when pressing or releasing the button
const uint16_t DCgapMs = 250;         // max ms between clicks for a double click event
const uint16_t holdTimeMs = 2000;     // ms hold period: how long to wait for press+hold event
const uint16_t longHoldTimeMs = 5000; // ms long hold period: how long to wait for press+hold event

// Read button state
ButtonEvent OLEDUserInterface::waitForButtonEvent() {
  ButtonEvent event = ButtonEvent::NoEvent;
  while (event == ButtonEvent::NoEvent) {
    event = checkButtons();
  }

  return event;
}

ButtonEvent OLEDUserInterface::checkButtons() {
#ifdef enable_Button2
  if (checkButton2() != ButtonEvent::NoEvent)
    return ButtonEvent::Hold;
  else
    return checkButton1();
#else
  return checkButton1();
#endif
}

ButtonEvent OLEDUserInterface::checkButton1() {
  return checkButton(0);
}

ButtonEvent OLEDUserInterface::checkButton2() {
  return checkButton(1);
}

ButtonEvent OLEDUserInterface::checkButton(uint8_t button) {
  ButtonEvent event = ButtonEvent::NoEvent;

  bool buttonVal;
  if (button == 0) {
    buttonVal = (PIND & (1 << 7));
  }
  else if (button == 1) {
    buttonVal = (PING & (1 << 2));
  }
  else {
    return ButtonEvent::NoEvent;
  }

  ButtonState &buttonStateRef = buttonsState[button];
  if (buttonVal == LOW && buttonStateRef.buttonLast == HIGH && (millis() - buttonStateRef.upTime) > debounceMs) {
    buttonStateRef.downTime = millis();
    buttonStateRef.ignoreUp = false;
    buttonStateRef.waitForUp = false;
    buttonStateRef.singleOK = true;
    buttonStateRef.holdEventPast = false;
    buttonStateRef.longholdEventPast = false;
    if ((millis() - buttonStateRef.upTime) < DCgapMs && buttonStateRef.DConUp == false && buttonStateRef.DCwaiting == true) {
      buttonStateRef.DConUp = true;
    }
    else {
      buttonStateRef.DConUp = false;
    }
    buttonStateRef.DCwaiting = false;
  }
  // Button released
  else if (buttonVal == HIGH && buttonStateRef.buttonLast == LOW && (millis() - buttonStateRef.downTime) > debounceMs) {
    if (!buttonStateRef.ignoreUp) {
      buttonStateRef.upTime = millis();
      if (buttonStateRef.DConUp == false) {
        buttonStateRef.DCwaiting = true;
      }
      else {
        event = ButtonEvent::DoubleClick;
        buttonStateRef.DConUp = false;
        buttonStateRef.DCwaiting = false;
        buttonStateRef.singleOK = false;
      }
    }
  }
  // Test for normal click event: DCgap expired
  if (buttonVal == HIGH && (millis() - buttonStateRef.upTime) >= DCgapMs && buttonStateRef.DCwaiting == true && buttonStateRef.DConUp == false && buttonStateRef.singleOK == true) {
    event = ButtonEvent::Click;
    buttonStateRef.DCwaiting = false;
  }
  // Test for hold
  if (buttonVal == LOW && (millis() - buttonStateRef.downTime) >= holdTimeMs) {
    // Trigger "normal" hold
    if (!buttonStateRef.holdEventPast) {
      event = ButtonEvent::Hold;
      buttonStateRef.waitForUp = true;
      buttonStateRef.ignoreUp = true;
      buttonStateRef.DConUp = false;
      buttonStateRef.DCwaiting = false;
      //buttonStateRef.downTime = millis();
      buttonStateRef.holdEventPast = true;
    }
    // Trigger "long" hold
    if ((millis() - buttonStateRef.downTime) >= longHoldTimeMs) {
      if (!buttonStateRef.longholdEventPast) {
        event = ButtonEvent::LongHold;
        buttonStateRef.longholdEventPast = true;
      }
    }
  }
  buttonStateRef.buttonLast = buttonVal;
  return event;
}

// Wait for user to push button
void OLEDUserInterface::waitForUserInput() {
  // Change led to green
  if (errorLvl == 0)
    rgbLed(green_color);

  while (1) {
    // get input button
    ButtonEvent b = checkButtons();

    // Send some clock pulses to the Eeprom in case it locked up
    if ((mode == CartReaderMode::N64Cart) && ((saveType == 5) || (saveType == 6))) {
      pulseClock_N64(1);
    }
    // if the cart readers input button is pressed shortly
    if (b == ButtonEvent::Click) {
      errorLvl = 0;
      break;
    }

    // if the cart readers input button is pressed long
    if (b == ButtonEvent::Hold) {
      errorLvl = 0;
      break;
    }
  }
}

static const uint8_t PROGMEM sig[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xE0, 0xF0, 0x80, 0x40, 0x30, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x90, 0xCC, 0x4E, 0x10, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x90, 0x5C, 0x7B, 0x19, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0xB8, 0x56, 0x31, 0x09, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xE0, 0xA8, 0x72, 0x31, 0x0F, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xAC, 0x23, 0x21, 0x86, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xE7, 0xA1, 0x00, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void OLEDUserInterface::displayAbout(const String &aboutMessage) {
  clearOutput();
  // Draw the Logo
  m_display.drawBitmap(0, 0, sig, 128, 64, 1);
  printlnMsg(aboutMessage);
  printlnMsg(F(""));
  printlnMsg(F(""));
  printlnMsg(F(""));
  printlnMsg(F(""));
  printlnMsg(F("Press Button"));
  flushOutput();

  while (1) {
    // get input button
    ButtonEvent b = waitForButtonEvent();

    // if the cart readers input button is pressed shortly
    if (b == ButtonEvent::Click) {
      break;
    }

    // if the cart readers input button is pressed long
    if (b == ButtonEvent::Hold) {
      break;
    }

    // if the button is pressed super long
    if (b == ButtonEvent::LongHold) {
      clearOutput();
      printlnMsg(F("Resetting folder..."));
      flushOutput();
      delay(2000);
      saveFolderNumber(0);
      break;
    }
    rgb.setColor(random(0, 255), random(0, 255), random(0, 255));
    delay(random(50, 100));
  }
}

bool OLEDUserInterface::supportsSDInfoDisplay() {
  return false;
}

void OLEDUserInterface::displaySDInfo(uint32_t capacityGB, uint8_t FATType) {
  ;
}

void OLEDUserInterface::updateN64ButtonTest(const String &currentButton, char stickX, char stickY) {
  m_display.clearDisplay();
  printAtPosition(F("Button Test"), CENTER, 0);
  m_display.drawLine(22 + 0, 10, 22 + 84, 10, WHITE);

  // Print Button
  String buttonLine = String(F("       "));
  
  if (currentButton.length() == 0) {
    buttonLine.concat(F("Press a button"));
  }
  else {
    buttonLine.concat(currentButton);
  }

  buttonLine.concat(F("       "));

  printAtPosition(buttonLine, CENTER, 20);

  // Print Stick X Value
  String stickXString = F("X: ");
  stickXString.concat(static_cast<int16_t>(stickX));
  stickXString.concat(F("   "));
  printAtPosition(stickXString, 22 + 0, 38);

  // Print Stick Y Value
  String stickYString = F("Y: ");
  stickYString.concat(static_cast<int16_t>(stickY));
  stickYString.concat(F("   "));
  printAtPosition(stickYString, 22 + 60, 38);

  printAtPosition(F("(Continue with START)"), 0, 55);

  //Update LCD
  m_display.display();
}

void OLEDUserInterface::printAtPosition(const String &str, int x, int y) {
  if (x == CENTER) {
    x = 64 - (str.length() / 2) * 6;
  }

  m_display.setCursor(x, y);
  m_display.print(str);
}

void OLEDUserInterface::displayMessage(const __FlashStringHelper *message) {
  m_display.println(message);
  m_display.display();
  waitForUserInput();
  m_display.clearDisplay();
  m_display.setCursor(0, 0);
}

bool OLEDUserInterface::supportsLargeMessages() {
  return false;
}

MenuWindow OLEDUserInterface::getMenuWindow(uint8_t currentChoice, uint8_t numAnswers) {
  // Determine the window of answers to display.
  // First extend the window towards the beginning an average amount
  // Then extend the window towards the end an average amount
  // Then make another pass and use any unused window space
  uint8_t currentAnswerWindowMin = currentChoice;
  uint8_t currentAnswerWindowMax = currentChoice;

  if (currentAnswerWindowMin >= ANSWERS_BEFORE_CURRENT_WHEN_IN_MIDDLE) {
    currentAnswerWindowMin = currentAnswerWindowMin - ANSWERS_BEFORE_CURRENT_WHEN_IN_MIDDLE;
  }
  else {
    currentAnswerWindowMin = 0;
  }

  if (currentAnswerWindowMax + ANSWERS_AFTER_CURRENT_WHEN_IN_MIDDLE <= numAnswers - 1) {
    currentAnswerWindowMax = currentAnswerWindowMax + ANSWERS_AFTER_CURRENT_WHEN_IN_MIDDLE;
  }
  else {
    currentAnswerWindowMax = numAnswers - 1;
  }

  uint8_t currentWindowSize = currentAnswerWindowMax - currentAnswerWindowMin + 1;
  if (currentWindowSize < MAX_ANSWERS_DISPLAYABLE) {
    uint8_t numSlotsAvailable = MAX_ANSWERS_DISPLAYABLE - currentWindowSize;
    if (currentAnswerWindowMin >= numSlotsAvailable) {
      currentAnswerWindowMin = currentAnswerWindowMin - numSlotsAvailable;
    }
    else {
      currentAnswerWindowMin = 0;
    }
  }

  currentWindowSize = currentAnswerWindowMax - currentAnswerWindowMin + 1;
  if (currentWindowSize < MAX_ANSWERS_DISPLAYABLE) {
    uint8_t numSlotsAvailable = MAX_ANSWERS_DISPLAYABLE - currentWindowSize;
    if (currentAnswerWindowMax + numSlotsAvailable <= numAnswers - 1) {
      currentAnswerWindowMax = currentAnswerWindowMax + numSlotsAvailable;
    }
    else {
      currentAnswerWindowMax = numAnswers - 1;
    }
  }

  MenuWindow window;
  window.Start = currentAnswerWindowMin;
  window.End = currentAnswerWindowMax;
  return window;
}

uint8_t OLEDUserInterface::askMultipleChoiceQuestion(const String &question, const String *answers, uint8_t numAnswers, uint8_t defaultChoice, bool wrapSelectionMovement) {
  //clear the screen
  m_display.clearDisplay();
  m_display.display();
  m_display.setCursor(0, 0);

  // Turn off text wrapping. This function assumes 1 line per choice.
  m_display.setTextWrap(false);

  // change the rgb led to the start menu color
  rgbLed(defaultChoice);

  uint8_t currentChoice = defaultChoice;

  MenuWindow window = getMenuWindow(currentChoice, numAnswers);
  uint8_t offsetInWindow = currentChoice - window.Start;
  bool drawMoreAboveIndicator = (window.Start > 0 || !wrapSelectionMovement) && offsetInWindow > 0;
  bool drawMoreBelowIndicator = (window.End + 1 < numAnswers || !wrapSelectionMovement) && offsetInWindow < window.End - window.Start;

  // print menu
  m_display.println(question);
  for (uint8_t i = window.Start; i <= window.End; i++) {
    // Add space for the selection dot
    m_display.print(" ");
    // Print menu item
    m_display.println(answers[i]);
  }

  drawOrClearMoreIndicators(drawMoreAboveIndicator, drawMoreBelowIndicator);

  // draw selection box
  m_display.drawPixel(0, 8 * offsetInWindow + 12, WHITE);

  m_display.display();

  unsigned long idleTime = millis();
  byte currentColor = 0;

  // wait until user makes his choice
  while (1) {
    // Attract Mode
    if (millis() - idleTime > 300000) {
      if ((millis() - idleTime) % 4000 == 0) {
        if (currentColor < 7) {
          currentColor++;
          if (currentColor == 1) {
            currentColor = 2; // skip red as that signifies an error to the user
          }
        }
        else {
          currentColor = 0;
        }
      }
      rgbLed(currentColor);
    }

    ButtonEvent b = waitForButtonEvent();

    if (b == ButtonEvent::DoubleClick) {
      idleTime = millis();

      if (currentChoice > 0) {
        currentChoice--;
      }
      else {
        if (wrapSelectionMovement) {
          currentChoice = numAnswers - 1;
        }
        else {
          currentChoice = ANSWER_GO_BACK;
          break;
        }
      }
    }
    if (b == ButtonEvent::Click) {
      idleTime = millis();

      if (currentChoice < numAnswers - 1) {
        currentChoice++;
      }
      else {
        if (wrapSelectionMovement) {
          currentChoice = 0;
        }
        else {
          currentChoice = ANSWER_NEXT_PAGE;
          break;
        }
      }
    }

    // If selection changed, redraw
    if (b == ButtonEvent::Click || b == ButtonEvent::DoubleClick) {
      MenuWindow previousWindow = window;
      uint8_t previousOffsetInWindow = offsetInWindow;

      window = getMenuWindow(currentChoice, numAnswers);
      offsetInWindow = currentChoice - window.Start;
      drawMoreAboveIndicator = (window.Start > 0 || !wrapSelectionMovement) && offsetInWindow > 0;
      drawMoreBelowIndicator = (window.End + 1 < numAnswers || !wrapSelectionMovement) && offsetInWindow < window.End - window.Start;

      // if window didn't change, just change the selection pixel and up/down arrows instead of doing a full redraw
      if (previousWindow.Start == window.Start && previousWindow.End == window.End) {
        m_display.drawPixel(0, 8 * previousOffsetInWindow + 12, BLACK);

        drawOrClearMoreIndicators(drawMoreAboveIndicator, drawMoreBelowIndicator);

        m_display.drawPixel(0, 8 * offsetInWindow + 12, WHITE);
        m_display.display();
      }
      else {
        m_display.clearDisplay();
        m_display.display();
        m_display.setCursor(0, 0);
        m_display.println(question);
        for (uint8_t i = window.Start; i <= window.End; i++) {
          // Add space for the selection dot
          m_display.print(" ");
          // Print menu item
          m_display.println(answers[i]);
        }

        drawOrClearMoreIndicators(drawMoreAboveIndicator, drawMoreBelowIndicator);

        // draw selection box
        m_display.drawPixel(0, 8 * offsetInWindow + 12, WHITE);
        m_display.display();
      }

      // change RGB led to the color of the current menu option
      rgbLed(currentChoice);
    }

    // if the Cart Dumpers button is hold continiously leave the menu
    // so the currently highlighted action can be executed

    if (b == ButtonEvent::Hold) {
      idleTime = millis();
      break;
    }
  }

  rgb.setColor(0, 0, 0);
  m_display.setTextWrap(true);
  m_display.clearDisplay();
  m_display.display();
  return currentChoice;
}

void OLEDUserInterface::drawOrClearMoreIndicators(bool drawMoreAboveIndicator, bool drawMoreBelowIndicator) {
  uint16_t drawMoreAboveIndicatorColor = drawMoreAboveIndicator ? WHITE : BLACK;
  m_display.drawPixel(0, 8 * 0 + 12, drawMoreAboveIndicatorColor);
  m_display.drawPixel(1, 8 * 0 + 12, drawMoreAboveIndicatorColor);
  m_display.drawPixel(2, 8 * 0 + 12, drawMoreAboveIndicatorColor);
  m_display.drawPixel(1, 8 * 0 + 12 - 1, drawMoreAboveIndicatorColor);

  uint16_t drawMoreBelowIndicatorColor = drawMoreBelowIndicator ? WHITE : BLACK;
  m_display.drawPixel(0, 8 * (MAX_ANSWERS_DISPLAYABLE - 1) + 12, drawMoreBelowIndicatorColor);
  m_display.drawPixel(1, 8 * (MAX_ANSWERS_DISPLAYABLE - 1) + 12, drawMoreBelowIndicatorColor);
  m_display.drawPixel(2, 8 * (MAX_ANSWERS_DISPLAYABLE - 1) + 12, drawMoreBelowIndicatorColor);
  m_display.drawPixel(1, 8 * (MAX_ANSWERS_DISPLAYABLE - 1) + 12 + 1, drawMoreBelowIndicatorColor);
}

// Display a question box with selectable answers. Make sure default choice is in [0, num_answers)
uint8_t OLEDUserInterface::askMultipleChoiceQuestion(const String &question, const String *answers, uint8_t numAnswers, uint8_t defaultChoice) {
  return askMultipleChoiceQuestion(question, answers, numAnswers, defaultChoice, true);
}

String OLEDUserInterface::askQuestionWithPagedAnswers(const String &question, PagedAnswerSource &answerSource) {
  while (true) {
    // get up to MAX_LINES - 1 answers
    uint8_t pageSize = MAX_LINES - 1;
    String answers[pageSize];
    uint8_t answerIndex = 0;
    for (answerIndex = 0; answerIndex < pageSize; answerIndex++) {
      String answer = answerSource.GetNextAnswer();
      if (answer.length() == 0) {
        break;
      }
    }

    uint8_t numAnswers = answerIndex;

    uint8_t answer;
    if (numAnswers == 0) {
      answer = ANSWER_GO_BACK;
    }
    else {
      answer = askMultipleChoiceQuestion(question, answers, numAnswers, 0, false);
    }

    if (answer == ANSWER_GO_BACK) {
      return String(F(""));
    }
    else if (answer == ANSWER_NEXT_PAGE) {
      continue;
    }
    else {
      return answers[answer];
    }
  }
}

// puts digits of number in digits array, starting with the most significant digit
void getDigits(uint32_t number, uint8_t numDigits, uint8_t *digits) {
  for (uint8_t place = numDigits; place > 0; place--) {
    digits[place - 1] = number % 10;
    number = number / 10;
  }
}

uint32_t getNumber(uint8_t *digits, uint8_t numDigits) {
  uint32_t number = 0;
  for (uint8_t place = 0; place < numDigits; place++) {
    number = number * 10;
    number = number + digits[place];
  }
  return number;
}

uint32_t OLEDUserInterface::readNumber(uint8_t numDigits, uint32_t defaultValue, uint32_t maxValue, const String &prompt, const __FlashStringHelper *outOfRangeMessage) {
  // No more than 3 digits currently supported
  if (numDigits == 0 || numDigits > 3) {
    return 0;
  }

  if (defaultValue > maxValue) {
    defaultValue = maxValue;
  }

  // Split into digits
  uint8_t defaultDigits[numDigits];
  getDigits(defaultValue, numDigits, defaultDigits);

  uint8_t maxValueDigits[numDigits];
  getDigits(maxValue, numDigits, maxValueDigits);

  uint8_t digits[numDigits];
  for (uint8_t place = 0; place < numDigits; place++) {
    digits[place] = defaultDigits[place];
  }

  // Cycle through all digits
  for (uint8_t placeCurrentlyReading = 0; placeCurrentlyReading < numDigits; placeCurrentlyReading++) {
    while (true) {
      uint8_t digitsWithUnreadAsZeros[numDigits];
      for (uint8_t place = 0; place < numDigits; place++) {
        if (place <= placeCurrentlyReading) {
          digitsWithUnreadAsZeros[place] = digits[place];
        }
        else {
          digitsWithUnreadAsZeros[place] = 0;
        }
      }

      // if greater than max value, roll back to digit of max value
      uint32_t numberWithUnreadAsZeros = getNumber(digitsWithUnreadAsZeros, numDigits);
      if (numberWithUnreadAsZeros > maxValue) {
        digits[placeCurrentlyReading] = maxValueDigits[placeCurrentlyReading];
      }

      m_display.clearDisplay();
      m_display.setCursor(0, 0);
      m_display.println(prompt);

      for (uint8_t placeCurrentlyDrawing = 0; placeCurrentlyDrawing < numDigits; placeCurrentlyDrawing++) {
        m_display.setCursor(23 + 20 * placeCurrentlyDrawing, 20);
        m_display.println(digits[placeCurrentlyDrawing]);
      }

      m_display.println("");
      m_display.println(F("Press to Change"));
      m_display.println(F("Hold to Select"));

      for (uint8_t placeCurrentlyDrawing = 0; placeCurrentlyDrawing < numDigits; placeCurrentlyDrawing++) {
        uint16_t color;
        if (placeCurrentlyDrawing == placeCurrentlyReading) {
          color = WHITE;
        }
        else {
          color = BLACK;
        }

        m_display.drawLine(20 + 20 * placeCurrentlyDrawing, 30, 30 + 20 * placeCurrentlyDrawing, 30, color);
      }

      m_display.display();
      ButtonEvent b = waitForButtonEvent();

      if (b == ButtonEvent::Click) {
        // click - bump up current place
        if (digits[placeCurrentlyReading] == 9) {
          digits[placeCurrentlyReading] = 0;
        }
        else {
          digits[placeCurrentlyReading] = digits[placeCurrentlyReading] + 1;
        }

        uint8_t digitsWithUnreadAsZeros[numDigits];
        for (uint8_t place = 0; place < numDigits; place++) {
          if (place <= placeCurrentlyReading) {
            digitsWithUnreadAsZeros[place] = digits[place];
          }
          else {
            digitsWithUnreadAsZeros[place] = 0;
          }
        }

        // if greater than max value, roll back to 0
        uint32_t numberWithUnreadAsZeros = getNumber(digitsWithUnreadAsZeros, numDigits);
        if (numberWithUnreadAsZeros > maxValue) {
          digits[placeCurrentlyReading] = 0;
        }
      }
      else if (b == ButtonEvent::DoubleClick) {
        // double click - decrement current place
        if (digits[placeCurrentlyReading] == 0) {
          digits[placeCurrentlyReading] = 9;
        }
        else {
          digits[placeCurrentlyReading] = digits[placeCurrentlyReading] - 1;
        }

        uint8_t digitsWithUnreadAsZeros[numDigits];
        for (uint8_t place = 0; place < numDigits; place++) {
          if (place <= placeCurrentlyReading) {
            digitsWithUnreadAsZeros[place] = digits[place];
          }
          else {
            digitsWithUnreadAsZeros[place] = 0;
          }
        }

        // if greater than max value, roll back to digit of max value
        uint32_t numberWithUnreadAsZeros = getNumber(digitsWithUnreadAsZeros, numDigits);
        if (numberWithUnreadAsZeros > maxValue) {
          digits[placeCurrentlyReading] = maxValueDigits[placeCurrentlyReading];
        }
      }
      else if (b == ButtonEvent::Hold) {
        break;
      }
    }
  }
  m_display.clearDisplay();
  m_display.setCursor(0, 0);

  return getNumber(digits, numDigits);
}

void OLEDUserInterface::clearOutput() {
  m_display.clearDisplay();
  m_display.setCursor(0, 0);
}

void OLEDUserInterface::flushOutput() {
  m_display.display();
}

void OLEDUserInterface::printMsg(const __FlashStringHelper *message) {
  m_display.print(message);
}

void OLEDUserInterface::printMsg(const char *message) {
  m_display.print(message);
}

void OLEDUserInterface::printMsg(const String &message) {
  m_display.print(message);
}

void OLEDUserInterface::printMsg(long unsigned int message) {
  m_display.print(message);
}

void OLEDUserInterface::printMsg(byte message, int outputFormat) {
  m_display.print(message, outputFormat);
}

void OLEDUserInterface::printlnMsg(const __FlashStringHelper *message) {
  m_display.println(message);
}

void OLEDUserInterface::printlnMsg(const char *message) {
  m_display.println(message);
}

void OLEDUserInterface::printlnMsg(const String &message) {
  m_display.println(message);
}

void OLEDUserInterface::printlnMsg(long unsigned int message) {
  m_display.println(message);
}

void OLEDUserInterface::printlnMsg(byte message, int outputFormat) {
  m_display.println(message, outputFormat);
}

void OLEDUserInterface::forceReset() {
  printlnMsg(F(""));
  printlnMsg(F("Press Button..."));
  flushOutput();
  waitForUserInput();
  resetArduino();
}

#endif // #if defined(enable_OLED) && !defined(UNIT_TEST)
