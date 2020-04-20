#ifndef cartreader_globals_h
#define cartreader_globals_h

#include <si5351.h>

extern char ver[5];

extern bool errorLvl;

// Adafruit Clock Generator
extern Si5351 clockgen;

// Array that holds the data
extern byte sdBuffer[512];

enum class CartReaderMode : uint8_t {
  N64Cart = 0,
  N64Controller = 1,
  SNES = 2,
  SFM = 3,
  SFMFlash = 4,
  SFMGame = 5,
  GB = 6,
  FLASH8 = 7,
  FLASH16 = 8,
  GBA = 9,
  GBM = 10,
  MDCart = 11,
  EPROM = 12,
  PCE = 13,
  SV = 14,
  NES = 15,
  SMS = 16,
  SegaCD = 17,
  GBSmart = 18,
  GBSmartFlash = 19,
  GBSmartGame = 20,
  WS = 21,
};

extern CartReaderMode mode;

// Global so that reading user input can send some clock pulses to the N64 Eeprom in case it locked up
extern byte saveType;

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

#endif
