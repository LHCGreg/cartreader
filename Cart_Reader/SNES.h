#ifndef cartreader_SNES_h
#define cartreader_SNES_h

#include <Arduino.h>

void snesMenu();

String getNextSnesSRAMOutputFilePathAndPrintMessage(const String &romName);
void readSRAM(const String &outputFilePath);

// Write file to SRAM
void writeSRAM(const String &inputFilePath);

unsigned long verifySRAM(const String &filePath);
boolean compare_checksum(const String &filePath);
void checkAltConf();

// Switch control pins to write
void controlOut_SNES();

// Switch control pins to read
void controlIn_SNES();

#endif
