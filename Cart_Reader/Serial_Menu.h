#ifndef cartreader_Serial_menu_h
#define cartreader_Serial_menu_h

#include "options.h"

#ifndef enable_OLED

void wait_serial();
byte questionBox_Serial(const __FlashStringHelper *question, char answers[7][20], int num_answers, int default_choice);

#endif

#endif
