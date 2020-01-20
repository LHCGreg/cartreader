#ifndef cartreader_SNES_h
#define cartreader_SNES_h

#include <Arduino.h>

void snesMenu();

void readSRAM();

// Write file to SRAM
void writeSRAM (boolean browseFile);

unsigned long verifySRAM();
boolean compare_checksum();
void checkAltConf();

// Switch control pins to write
void controlOut_SNES();

// Switch control pins to read
void controlIn_SNES();

#endif
