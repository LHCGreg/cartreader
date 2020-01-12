#ifndef cartreader_menu_h
#define cartreader_menu_h

// Temporary array that holds the menu option read out of progmem
extern char menuOptions[7][20];

void convertPgm(const char *const pgmOptions[], byte numArrays);

void aboutScreen();
void draw_progressbar(uint32_t processed, uint32_t total);
void print_Error(const __FlashStringHelper *errorMessage, boolean forceReset);
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

#endif
