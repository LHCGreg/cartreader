#ifndef cartreader_GB_h
#define cartreader_GB_h

#include <Arduino.h>

void gbxMenu();
void getCartInfo_GB();
void showCartInfo_GB();
byte readByte_GB(word myAddress);
void writeByte_GB(int myAddress, uint8_t myData);
String getNextGBRomOutputPathAndPrintMessage(const String &gameName);
void readROM_GB(const String &outputFilePath);
boolean compare_checksum_GB(const String &filePath);
void readSRAM_GB();
void writeSRAM_GB(const String &inputFilePath);
unsigned long verifySRAM_GB(const String &filePath);

// Switch data pins to read
void dataIn_GB();

#endif
