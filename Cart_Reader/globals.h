#ifndef cartreader_globals_h
#define cartreader_globals_h

#include <Adafruit_SSD1306.h>
#include <SdFat.h>
#include <si5351.h>

extern char ver[5];

extern bool errorLvl;

// Adafruit Clock Generator
extern Si5351 clockgen;

// Graphic I2C LCD
extern Adafruit_SSD1306 display;

extern SdFat sd;
extern SdFile myFile;

// Array that holds the data
extern byte sdBuffer[512];

//remember folder number to create a new folder for every save
extern int foldern;
extern char folder[36];

// Global so that reading user input can send some clock pulses to the N64 Eeprom in case it locked up
extern byte saveType;

extern boolean ignoreError;
extern char flashid[5];
extern byte numBanks;
extern unsigned long sramSize;
extern char romName[17];
extern int romType;
extern word romSize;
extern char checksumStr[5];
extern byte romVersion;
extern char cartID[5];
extern unsigned long cartSize;
extern char vendorID[5];
extern unsigned long fileSize;
extern unsigned long sramBase;
extern byte eepbit[8];
extern byte eeptemp;

// Variable to count errors
extern unsigned long writeErrors;

// Operation mode
extern byte mode;

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

#endif
