#include "menu.h"
#include "globals.h"
#include "options.h"
#include "OLED_menu.h"
#include "utils.h"
#include "RGB_LED.h"

// Temporary array that holds the menu option read out of progmem
char menuOptions[7][20];

// Converts a progmem array into a ram array
void convertPgm(const char *const pgmOptions[], byte numArrays) {
  for (int i = 0; i < numArrays; i++) {
    strlcpy_P(menuOptions[i], (char *) pgm_read_word(&(pgmOptions[i])), 20);
  }
}

void aboutScreen() {
  display_Clear();
  // Draw the Logo
  display.drawBitmap(0, 0, sig, 128, 64, 1);
  println_Msg(F("Cartridge Reader"));
  println_Msg(F("github.com/sanni"));
  print_Msg(F("2019 Version "));
  println_Msg(ver);
  println_Msg(F(""));
  println_Msg(F(""));
  println_Msg(F(""));
  println_Msg(F(""));
  println_Msg(F("Press Button"));
  display_Update();

  while (1) {
#ifdef enable_OLED
    // get input button
    int b = checkButton();

    // if the cart readers input button is pressed shortly
    if (b == 1) {
      resetArduino();
    }

    // if the cart readers input button is pressed long
    if (b == 3) {
      resetArduino();
    }

    // if the button is pressed super long
    if (b == 4) {
      display_Clear();
      println_Msg(F("Resetting folder..."));
      display_Update();
      delay(2000);
      foldern = 0;
      EEPROM_writeAnything(0, foldern);
      resetArduino();
    }
#else
    wait_serial();
    resetArduino();
#endif
    rgb.setColor(random(0, 255), random(0, 255), random(0, 255));
    delay(random(50, 100));
  }
}

void draw_progressbar(uint32_t processed, uint32_t total) {
  uint8_t current, i;
  static uint8_t previous;
  uint8_t steps = 20;

  //Find progressbar length and draw if processed size is not 0
  if (processed == 0) {
    previous = 0;
    print_Msg(F("["));
    display_Update();
    return;
  }

  // Progress bar
  current = (processed >= total) ? steps : processed / (total / steps);

  //Draw "*" if needed
  if (current > previous) {
    for (i = previous; i < current; i++) {
      // steps are 20, so 20 - 1 = 19.
      if (i == (19)) {
        //If end of progress bar, finish progress bar by drawing "]"
        print_Msg(F("]"));
      }
      else {
        print_Msg(F("*"));
      }
    }
    //update previous "*" status
    previous = current;
    //Update display
    display_Update();
  }
}

void print_Error(const __FlashStringHelper *errorMessage, boolean forceReset) {
  errorLvl = 1;
  rgb.setColor(255, 0, 0);
  println_Msg(errorMessage);
  display_Update();

  if (forceReset) {
#ifdef enable_OLED
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
    if (ignoreError == 0) {
      resetArduino();
    }
    else {
      ignoreError = 0;
      display_Clear();
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F("  Error Overwrite"));
      display_Update();
      delay(2000);
    }
#else
    println_Msg(F("Fatal Error, please reset"));
    while (1)
      ;
#endif
  }
}

void wait() {
#ifdef enable_OLED
  wait_btn();
#else
  wait_serial();
#endif
}

void print_Msg(const __FlashStringHelper *string) {
#ifdef enable_OLED
  display.print(string);
#else
  Serial.print(string);
#endif
}

void print_Msg(const char string[]) {
#ifdef enable_OLED
  display.print(string);
#else
  Serial.print(string);
#endif
}

void print_Msg(long unsigned int message) {
#ifdef enable_OLED
  display.print(message);
#else
  Serial.print(message);
#endif
}

void print_Msg(byte message, int outputFormat) {
#ifdef enable_OLED
  display.print(message, outputFormat);
#else
  Serial.print(message, outputFormat);
#endif
}

void print_Msg(String string) {
#ifdef enable_OLED
  display.print(string);
#else
  Serial.print(string);
#endif
}

void println_Msg(String string) {
#ifdef enable_OLED
  display.println(string);
#else
  Serial.println(string);
#endif
}

void println_Msg(byte message, int outputFormat) {
#ifdef enable_OLED
  display.println(message, outputFormat);
#else
  Serial.println(message, outputFormat);
#endif
}

void println_Msg(const char message[]) {
#ifdef enable_OLED
  display.println(message);
#else
  Serial.println(message);
#endif
}

void println_Msg(const __FlashStringHelper *string) {
#ifdef enable_OLED
  display.println(string);
#else
  Serial.println(string);
#endif
}

void println_Msg(long unsigned int message) {
#ifdef enable_OLED
  display.println(message);
#else
  Serial.println(message);
#endif
}

void display_Update() {
#ifdef enable_OLED
  display.display();
#else
  delay(100);
#endif
}

void display_Clear() {
#ifdef enable_OLED
  display.clearDisplay();
  display.setCursor(0, 0);
#endif
}

unsigned char question_box(const __FlashStringHelper *question, char answers[7][20], int num_answers, int default_choice) {
#ifdef enable_OLED
  return questionBox_OLED(question, answers, num_answers, default_choice);
#else
  return questionBox_Serial(question, answers, num_answers, default_choice);
#endif
}