#ifndef cartreader_GB_h
#define cartreader_GB_h

#include <Arduino.h>

void gbxMenu();
void getCartInfo_GB();
void showCartInfo_GB();
byte readByte_GB(word myAddress);
void writeByte_GB(int myAddress, uint8_t myData);
void readROM_GB();
boolean compare_checksum_GB();
void readSRAM_GB();
void writeSRAM_GB();
uint32_t verifySRAM_GB();

// Switch data pins to read
void dataIn_GB();

#endif
