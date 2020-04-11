#ifndef cartreader_menu_h
#define cartreader_menu_h

#include <Arduino.h>
#include <stdint.h>
#include "globals.h"
#include "options.h"
#include "RGB_LED.h"
#include "utils.h"

// Temporary array that holds the menu option read out of progmem
extern char menuOptions[7][20];

void convertPgm(const char *const pgmOptions[], byte numArrays);

void aboutScreen();
void draw_progressbar(uint32_t processed, uint32_t total);
void wait();
void print_Msg(const __FlashStringHelper *string);
void print_Msg(const char string[]);
void print_Msg(long unsigned int message);
void print_Msg(byte message, int outputFormat);
void print_Msg(String string);
void println_Msg(String string);
void println_Msg(byte message, int outputFormat);
void println_Msg(const char message[]);
void println_Msg(const __FlashStringHelper *string);
void println_Msg(long unsigned int message);
void display_Update();
void display_Clear();
unsigned char question_box(const __FlashStringHelper *question, char answers[7][20], int num_answers, int default_choice);

template <class printable>
[[noreturn]] void print_Error(const printable &errorMessage) {
  errorLvl = 1;
  rgb.setColor(255, 0, 0);
  println_Msg(errorMessage);
  display_Update();

#ifdef enable_OLED
  println_Msg(F(""));
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
  resetArduino();
#else
  println_Msg(F("Fatal Error, please reset"));
  while (1)
    ;
#endif
}

template <class printable>
void print_Warning(const printable &errorMessage) {
  errorLvl = 1;
  rgb.setColor(255, 0, 0);
  println_Msg(errorMessage);
  display_Update();
}

#endif
