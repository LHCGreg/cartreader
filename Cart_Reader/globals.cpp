#include "globals.h"
#include <Arduino.h>
#include <si5351.h>

char ver[5] = "4.6a";

bool errorLvl = false;

// Adafruit Clock Generator
Si5351 clockgen;

// Array that holds the data
byte sdBuffer[512];

// Operation mode
CartReaderMode mode;

byte saveType;

char flashid[5];
byte numBanks = 128;
unsigned long sramSize = 0;
char romName[17];
int romType = 0;
word romSize = 0;
char checksumStr[5];
byte romVersion = 0;
char cartID[5];
unsigned long cartSize;
char vendorID[5];
unsigned long fileSize;
unsigned long sramBase;
byte eepbit[8];
byte eeptemp;

// Variable to count errors
unsigned long writeErrors;
