#ifndef cartreader_Cart_Reader_h
#define cartreader_Cart_Reader_h

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <SdFat.h>
#include <si5351.h>
#include <EEPROM.h>
#include "options.h"

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
void print_Error(const __FlashStringHelper *errorMessage, boolean forceReset);
void display_Clear();
void display_Update();
void wait();

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
extern Adafruit_SSD1306 display;

// Adafruit Clock Generator
extern Si5351 clockgen;

extern void(*resetArduino) (void);

// Converts a progmem array into a ram array
void convertPgm(const char* const pgmOptions[], byte numArrays);

unsigned char question_box(const __FlashStringHelper* question, char answers[7][20], int num_answers, int default_choice);

void pulseClock_N64(unsigned int times);

// Progressbar
void draw_progressbar(uint32_t processedsize, uint32_t totalsize);

// Common I/O Functions

// Switch data pins to write
void dataOut();

// Switch data pins to read
void dataIn();

extern byte mode;
extern bool errorLvl;

// Common
extern char romName[17];
extern unsigned long sramSize;
extern int romType;
extern byte saveType;
extern word romSize;
extern byte numBanks;
extern char checksumStr[5];
extern bool errorLvl;
extern byte romVersion;
extern char cartID[5];
extern unsigned long cartSize;
extern char flashid[5];
extern char vendorID[5];
extern unsigned long fileSize;
extern unsigned long sramBase;
extern byte eepbit[8];
extern byte eeptemp;

// Variable to count errors
extern unsigned long writeErrors;

// Temporary array that holds the menu option read out of progmem
extern char menuOptions[7][20];

// CRC32 lookup table // 256 entries
extern const uint32_t crc_32_tab[] PROGMEM; /* CRC polynomial 0xedb88320 */

// Array that holds the data
extern byte sdBuffer[512];

// File browser
#define FILENAME_LENGTH 32
#define FILEPATH_LENGTH 64
#define FILEOPTS_LENGTH 20

//remember folder number to create a new folder for every save
extern int foldern;
extern char folder[36];

extern char fileName[FILENAME_LENGTH];
extern char filePath[FILEPATH_LENGTH];

// SD Card
#define sdSpeed SPI_FULL_SPEED
// SD Card (Pin 50 = MISO, Pin 51 = MOSI, Pin 52 = SCK, Pin 53 = SS)
#define chipSelectPin 53
extern SdFat sd;
extern SdFile myFile;

// Mode menu
#define mode_N64_Cart 0
#define mode_N64_Controller 1
#define mode_SNES 2
#define mode_SFM 3
#define mode_SFM_Flash 4
#define mode_SFM_Game 5
#define mode_GB 6
#define mode_FLASH8 7
#define mode_FLASH16 8
#define mode_GBA 9
#define mode_GBM 10
#define mode_MD_Cart 11
#define mode_EPROM 12
#define mode_PCE 13
#define mode_SV 14
#define mode_NES 15
#define mode_SMS 16
#define mode_SEGA_CD 17
#define mode_GB_GBSmart 18
#define mode_GB_GBSmart_Flash 19
#define mode_GB_GBSmart_Game 20
#define mode_WS 21

// optimization-safe nop delay
#define NOP __asm__ __volatile__ ("nop\n\t")

// AVR Eeprom

template <class T> int EEPROM_writeAnything(int ee, const T& value) {
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value) {
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

#endif
