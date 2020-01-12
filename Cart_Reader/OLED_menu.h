#ifndef cartreader_OLED_menu_h
#define cartreader_OLED_menu_h

#include "options.h"

#ifdef enable_OLED

// Read button state
int checkButton();

// Read button 1
int checkButton1();

// Read button 2
int checkButton2();

// Wait for user to push button
void wait_btn();

// Display a question box with selectable answers. Make sure default choice is in (0, num_answers]
unsigned char questionBox_OLED(const __FlashStringHelper* question, char answers[7][20], int num_answers, int default_choice);

extern const uint8_t PROGMEM icon[];
extern const uint8_t PROGMEM sig[];
#endif

#endif
