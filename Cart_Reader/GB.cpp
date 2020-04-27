//******************************************
// GAME BOY MODULE
//******************************************

#include <Arduino.h>
#include "GB.h"
#include "filebrowser.h"
#include "GBA.h"
#include "NP.h"
#include "GBSmart.h"
#include "ui.h"
#include "globals.h"
#include "utils.h"
#include "SD.h"

/******************************************
   Variables
 *****************************************/
// Game Boy
int sramBanks;
unsigned int romBanks;
uint16_t sramEndAddress = 0;

/******************************************
   Function prototypes
 *****************************************/
String getNextGBSaveOutputPath(const String &romName);
void setup_GB();
void writeFlash_GB(byte MBC);

// Start menu for both GB and GBA
void gbxMenu() {
  while (true) {
    const __FlashStringHelper *item_GB = F("Game Boy (Color)");
    const __FlashStringHelper *item_GBA = F("Game Boy Advance");
    const __FlashStringHelper *item_NPower = F("NPower GB Memory");
    const __FlashStringHelper *item_GBSmart = F("GB Smart");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_GB,
      item_GBA,
      item_NPower,
      item_GBSmart,
      item_Back,
    };

    const __FlashStringHelper *answer = ui->askMultipleChoiceQuestion(
      F("Select Game Boy"), menu, ARRAY_LENGTH(menu), item_GB);

    if (answer == item_GB) {
      ui->clearOutput();
      ui->flushOutput();
      setup_GB();
      mode = CartReaderMode::GB;
      gbMenu();
    }
    else if (answer == item_GBA) {
      ui->clearOutput();
      ui->flushOutput();
      setup_GBA();
      mode = CartReaderMode::GBA;
      gbaMenu();
    }
    else if (answer == item_NPower) {
      ui->clearOutput();
      ui->flushOutput();
      setup_GBM();
      mode = CartReaderMode::GBM;
      gbmMenu();
    }
    else if (answer == item_GBSmart) {
      ui->clearOutput();
      ui->flushOutput();
      setup_GBSmart();
      mode = CartReaderMode::GBSmart;
      gbSmartMenu();
    }
    else if (answer == item_Back) {
      break;
    }
  }
}

void gbMenu() {
  while (true) {
    const __FlashStringHelper *item_ReadROM = F("Read Rom");
    const __FlashStringHelper *item_ReadSave = F("Read Save");
    const __FlashStringHelper *item_WriteSave = F("Write Save");
    const __FlashStringHelper *item_FlashMBC3 = F("Flash MBC3 cart");
    const __FlashStringHelper *item_FlashMBC5 = F("Flash MBC5 cart");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_ReadROM,
      item_ReadSave,
      item_WriteSave,
      item_FlashMBC3,
      item_FlashMBC5,
      item_Back,
    };

    const __FlashStringHelper *answer = ui->askMultipleChoiceQuestion(
      F("GB Cart Reader"), menu, ARRAY_LENGTH(menu), item_ReadROM);

    if (answer == item_ReadROM) {
      ui->clearOutput();
      String outputFilePath = getNextGBRomOutputPathAndPrintMessage(romName);
      readROM_GB(outputFilePath);
      compare_checksum_GB(outputFilePath);
    }
    else if (answer == item_ReadSave) {
      ui->clearOutput();
      // Does cartridge have SRAM
      if (sramEndAddress > 0) {
        // Change working dir to root
        readSRAM_GB();
      }
      else {
        ui->printError(F("Cart has no Sram"));
      }
    }
    else if (answer == item_WriteSave) {
      ui->clearOutput();
      // Does cartridge have SRAM
      if (sramEndAddress > 0) {
        String inputFilePath = fileBrowser(F("Select sav file"));
        writeSRAM_GB(inputFilePath);
        uint32_t writeErrors = verifySRAM_GB(inputFilePath);
        if (writeErrors == 0) {
          ui->printlnMsg(F("Verified OK"));
          ui->flushOutput();
        }
        else {
          ui->printMsg(F("Error: "));
          ui->printMsg(writeErrors);
          ui->printlnMsg(F(" bytes "));
          ui->printError(F("did not verify."));
        }
      }
      else {
        ui->printError(F("Cart has no Sram"));
      }
    }
    else if (answer == item_FlashMBC3) {
      //MBC3
      writeFlash_GB(3);
      // Reset
      ui->waitForUserInput();
      resetArduino();
    }
    else if (answer == item_FlashMBC5) {
      //MBC5
      writeFlash_GB(5);
      // Reset
      ui->waitForUserInput();
      resetArduino();
    }
    else if (answer == item_Back) {
      break;
    }

    ui->printlnMsg(F(""));
    ui->printlnMsg(F("Press Button..."));
    ui->flushOutput();
    ui->waitForUserInput();
  }
}

String getNextGBRomOutputPathAndPrintMessage(const String &romName) {
  return getNextOutputPathWithNumberedFolderAndPrintMessage(F("GB"), F("ROM"), romName, F(".GB"));
}

String getNextGBSaveOutputPath(const String &romName) {
  return getNextOutputPathWithNumberedFolder(F("GB"), F("SAVE"), romName, F(".sav"));
}

/******************************************
   Setup
 *****************************************/
void setup_GB() {
  // Set RST(PH0) to Input
  DDRH &= ~(1 << 0);
  // Activate Internal Pullup Resistors
  PORTH |= (1 << 0);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;

  // Set Control Pins to Output CS(PH3) WR(PH5) RD(PH6)
  DDRH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Output a high signal on all pins, pins are active low therefore everything is disabled now
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Disable Internal Pullups
  //PORTC = 0x00;

  delay(400);

  // Print start page
  getCartInfo_GB();
  showCartInfo_GB();
}

void showCartInfo_GB() {
  ui->clearOutput();
  if (strcmp(checksumStr, "00") != 0) {
    ui->printlnMsg(F("GB Cart Info"));
    ui->printMsg(F("Name: "));
    ui->printlnMsg(romName);
    ui->printMsg(F("Rom Type: "));
    switch (romType) {
      case 0: ui->printMsg(F("ROM ONLY")); break;
      case 1: ui->printMsg(F("MBC1")); break;
      case 2: ui->printMsg(F("MBC1+RAM")); break;
      case 3: ui->printMsg(F("MBC1+RAM")); break;
      case 5: ui->printMsg(F("MBC2")); break;
      case 6: ui->printMsg(F("MBC2")); break;
      case 8: ui->printMsg(F("ROM+RAM")); break;
      case 9: ui->printMsg(F("ROM ONLY")); break;
      case 11: ui->printMsg(F("MMM01")); break;
      case 12: ui->printMsg(F("MMM01+RAM")); break;
      case 13: ui->printMsg(F("MMM01+RAM")); break;
      case 15: ui->printMsg(F("MBC3+TIMER")); break;
      case 16: ui->printMsg(F("MBC3+TIMER+RAM")); break;
      case 17: ui->printMsg(F("MBC3")); break;
      case 18: ui->printMsg(F("MBC3+RAM")); break;
      case 19: ui->printMsg(F("MBC3+RAM")); break;
      case 21: ui->printMsg(F("MBC4")); break;
      case 22: ui->printMsg(F("MBC4+RAM")); break;
      case 23: ui->printMsg(F("MBC4+RAM")); break;
      case 25: ui->printMsg(F("MBC5")); break;
      case 26: ui->printMsg(F("MBC5+RAM")); break;
      case 27: ui->printMsg(F("MBC5+RAM")); break;
      case 28: ui->printMsg(F("MBC5+RUMBLE")); break;
      case 29: ui->printMsg(F("MBC5+RUMBLE+RAM")); break;
      case 30: ui->printMsg(F("MBC5+RUMBLE+RAM")); break;
      case 252: ui->printMsg(F("Gameboy Camera")); break;
      default: ui->printMsg(F("Not found"));
    }
    ui->printlnMsg(F(" "));
    ui->printMsg(F("Rom Size: "));
    switch (romSize) {
      case 0: ui->printMsg(F("32KB")); break;
      case 1: ui->printMsg(F("64KB")); break;
      case 2: ui->printMsg(F("128KB")); break;
      case 3: ui->printMsg(F("256KB")); break;
      case 4: ui->printMsg(F("512KB")); break;
      case 5: ui->printMsg(F("1MB")); break;
      case 6: ui->printMsg(F("2MB")); break;
      case 7: ui->printMsg(F("4MB")); break;
      case 82: ui->printMsg(F("1.1MB")); break;
      case 83: ui->printMsg(F("1.2MB")); break;
      case 84: ui->printMsg(F("1.5MB)")); break;
      default: ui->printMsg(F("Not found"));
    }
    ui->printlnMsg(F(""));
    ui->printMsg(F("Banks: "));
    ui->printlnMsg(romBanks);

    ui->printMsg(F("Sram Size: "));
    switch (sramSize) {
      case 0:
        if (romType == 6) {
          ui->printMsg(F("512B"));
        }
        else {
          ui->printMsg(F("None"));
        }
        break;
      case 1: ui->printMsg(F("2KB")); break;
      case 2: ui->printMsg(F("8KB")); break;
      case 3: ui->printMsg(F("32KB")); break;
      case 4: ui->printMsg(F("128KB")); break;
      default: ui->printMsg(F("Not found"));
    }
    ui->printlnMsg(F(""));
    ui->printMsg(F("Checksum: "));
    ui->printlnMsg(checksumStr);
    ui->flushOutput();

    // Wait for user input
    ui->printlnMsg(F("Press Button..."));
    ui->flushOutput();
    ui->waitForUserInput();
  }
  else {
    ui->printErrorAndAbort(F("GAMEPAK ERROR"), true);
  }
}

/******************************************
   I/O Functions
 *****************************************/

/******************************************
  Low level functions
*****************************************/
// Switch data pins to read
void dataIn_GB() {
  // Set to Input
  DDRC = 0x00;
}

byte readByte_GB(word myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  byte tempByte = PINC;

  // Switch CS(PH3) and RD(PH6) to HIGH
  PORTH |= (1 << 3) | (1 << 6);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempByte;
}

void writeByte_GB(int myAddress, uint8_t myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull WR(PH5) low
  PORTH &= ~(1 << 5);

  // Leave WE low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull WR(PH5) HIGH
  PORTH |= (1 << 5);

  // Leave WE high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}

/******************************************
  Game Boy functions
*****************************************/
// Read Cartridge Header
void getCartInfo_GB() {
  romType = readByte_GB(0x0147);
  romSize = readByte_GB(0x0148);
  sramSize = readByte_GB(0x0149);

  // ROM banks
  romBanks = 2; // Default 32K
  if (romSize >= 1) { // Calculate rom size
    romBanks = 2 << romSize;
  }

  // RAM banks
  sramBanks = 0; // Default 0K RAM
  if (romType == 6) {
    sramBanks = 1;
  }
  switch (sramSize) {
    case 2:
      sramBanks = 1;
      break;
    case 3:
      sramBanks = 4;
      break;
    case 4:
      sramBanks = 16;
      break;
    case 5:
      sramBanks = 8;
      break;
  }

  // RAM end address
  if (romType == 6) {
    sramEndAddress = 0xA1FF;  // MBC2 512bytes (nibbles)
  }
  if (sramSize == 1) {
    sramEndAddress = 0xA7FF;  // 2K RAM
  }
  if (sramSize > 1) {
    sramEndAddress = 0xBFFF;  // 8K RAM
  }

  // Get Checksum as string
  sprintf(checksumStr, "%02X%02X", readByte_GB(0x014E), readByte_GB(0x014F));

  // Get name
  byte myByte = 0;
  byte myLength = 0;

  for (int addr = 0x0134; addr <= 0x13C; addr++) {
    myByte = readByte_GB(addr);
    if (((char(myByte) >= 48 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 15) {
      romName[myLength] = char(myByte);
      myLength++;
    }
  }
}

// Dump ROM
void readROM_GB(const String &outputFilePath) {
  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  uint16_t romAddress = 0;

  // Read number of banks and switch banks
  for (uint16_t bank = 1; bank < romBanks; bank++) {
    // Switch data pins to output
    dataOut();

    if (romType >= 5) { // MBC2 and above
      writeByte_GB(0x2100, bank); // Set ROM bank
    }
    else { // MBC1
      writeByte_GB(0x6000, 0); // Set ROM Mode
      writeByte_GB(0x4000, bank >> 5); // Set bits 5 & 6 (01100000) of ROM bank
      writeByte_GB(0x2000, bank & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
    }

    // Switch data pins to intput
    dataIn_GB();

    if (bank > 1) {
      romAddress = 0x4000;
    }

    // Read up to 7FFF per bank
    while (romAddress <= 0x7FFF) {
      uint8_t readData[512];
      for (int i = 0; i < 512; i++) {
        readData[i] = readByte_GB(romAddress + i);
      }
      outputFile.write(readData, 512);
      romAddress += 512;
    }
  }

  // Close the file:
  outputFile.close();
}

unsigned int calc_checksum_GB(const String &filePath) {
  unsigned int calcChecksum = 0;
  //  int calcFilesize = 0; // unused
  unsigned long i = 0;
  int c = 0;

  SafeSDFile inputFile = SafeSDFile::openForReading(filePath);
  //calcFilesize = myFile.fileSize() * 8 / 1024 / 1024; // unused
  for (i = 0; i < (inputFile.fileSize() / 512); i++) {
    inputFile.read(sdBuffer, 512);
    for (c = 0; c < 512; c++) {
      calcChecksum += sdBuffer[c];
    }
  }
  inputFile.close();
  // Subtract checksum bytes
  calcChecksum -= readByte_GB(0x014E);
  calcChecksum -= readByte_GB(0x014F);

  // Return result
  return (calcChecksum);
}

boolean compare_checksum_GB(const String &filePath) {
  ui->printlnMsg(F("Calculating Checksum"));
  ui->flushOutput();

  char calcsumStr[5];
  sprintf(calcsumStr, "%04X", calc_checksum_GB(filePath));

  if (strcmp(calcsumStr, checksumStr) == 0) {
    ui->printMsg(F("Result: "));
    ui->printlnMsg(calcsumStr);
    ui->printlnMsg(F("Checksum matches"));
    ui->flushOutput();
    return 1;
  }
  else {
    ui->printMsg(F("Result: "));
    ui->printlnMsg(calcsumStr);
    ui->printError(F("Checksum Error"));
    return 0;
  }
}

// Read RAM
void readSRAM_GB() {
  // Does cartridge have RAM
  if (sramEndAddress > 0) {
    String outputFilePath = getNextGBSaveOutputPath(romName);

    //open file on sd card
    SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

    dataIn_GB();

    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    readByte_GB(0x0134);

    dataOut();
    if (romType <= 4) { // MBC1
      writeByte_GB(0x6000, 1); // Set RAM Mode
    }

    // Initialise MBC
    writeByte_GB(0x0000, 0x0A);

    // Switch SRAM banks
    for (uint8_t bank = 0; bank < sramBanks; bank++) {
      dataOut();
      writeByte_GB(0x4000, bank);

      // Read SRAM
      dataIn_GB();
      for (uint16_t sramAddress = 0xA000; sramAddress <= sramEndAddress; sramAddress += 64) {
        uint8_t readData[64];
        for (uint8_t i = 0; i < 64; i++) {
          readData[i] = readByte_GB(sramAddress + i);
        }
        outputFile.write(readData, 64);
      }
    }

    // Disable SRAM
    dataOut();
    writeByte_GB(0x0000, 0x00);
    dataIn_GB();

    // Close the file:
    outputFile.close();

    // Signal end of process
    ui->printMsg(F("Saved to "));
    ui->printlnMsg(outputFilePath);
    ui->flushOutput();
  }
  else {
    ui->printError(F("Cart has no SRAM"));
  }
}

// Write RAM
void writeSRAM_GB(const String& inputFilePath) {
  // Does cartridge have SRAM
  if (sramEndAddress > 0) {
    //open file on sd card
    SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

    // Set pins to input
    dataIn_GB();

    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    readByte_GB(0x0134);

    dataOut();

    if (romType <= 4) { // MBC1
      writeByte_GB(0x6000, 1); // Set RAM Mode
    }

    // Initialise MBC
    writeByte_GB(0x0000, 0x0A);

    // Switch RAM banks
    for (uint8_t bank = 0; bank < sramBanks; bank++) {
      writeByte_GB(0x4000, bank);

      // Write RAM
      for (uint16_t sramAddress = 0xA000; sramAddress <= sramEndAddress; sramAddress++) {
        // Pull CS(PH3) LOW
        PORTH &= ~(1 << 3);
        // Write to RAM
        uint8_t byteToWriteToSram = inputFile.readByteOrDie(F("File not large enough.\nSRAM partially written!"));
        writeByte_GB(sramAddress, byteToWriteToSram);
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        // Pull CS(PH3) HIGH
        PORTH |= (1 << 3) ;
      }
    }
    // Disable RAM
    writeByte_GB(0x0000, 0x00);

    // Set pins to input
    dataIn_GB();

    // Close the file:
    inputFile.close();
    ui->clearOutput();
    ui->printlnMsg(F("SRAM writing finished"));
    ui->flushOutput();
  }
  else {
    ui->printError(F("Cart has no SRAM"));
  }
}

// Check if the SRAM was written without any error
unsigned long verifySRAM_GB(const String &filePath) {
  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(filePath);

  // Variable for errors
  writeErrors = 0;

  dataIn_GB();

  // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
  readByte_GB(0x0134);

  // Does cartridge have RAM
  if (sramEndAddress > 0) {
    dataOut();
    if (romType <= 4) { // MBC1
      writeByte_GB(0x6000, 1); // Set RAM Mode
    }

    // Initialise MBC
    writeByte_GB(0x0000, 0x0A);

    // Switch SRAM banks
    for (uint8_t bank = 0; bank < sramBanks; bank++) {
      dataOut();
      writeByte_GB(0x4000, bank);

      // Read SRAM
      dataIn_GB();
      for (uint16_t sramAddress = 0xA000; sramAddress <= sramEndAddress; sramAddress += 64) {
        //fill sdBuffer
        inputFile.read(sdBuffer, 64);
        for (int c = 0; c < 64; c++) {
          if (readByte_GB(sramAddress + c) != sdBuffer[c]) {
            writeErrors++;
          }
        }
      }
    }
    dataOut();
    // Disable RAM
    writeByte_GB(0x0000, 0x00);
    dataIn_GB();
  }
  // Close the file:
  inputFile.close();
  return writeErrors;
}

// Write 29F032 flashrom
// A0-A13 directly connected to cart edge -> 16384(0x0-0x3FFF) bytes per bank -> 256(0x0-0xFF) banks
// A14-A21 connected to MBC5
void writeFlash_GB(byte MBC) {
  // Launch filebrowser
  String inputFilePath = fileBrowser(F("Select file"));
  ui->clearOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  inputFile.seekCur(0x147);
  romType = inputFile.readByteOrDie(F("Failed reading ROM type from file!"));
  romSize = inputFile.readByteOrDie(F("Failed reading ROM size from file!"));
  // Go back to file beginning
  inputFile.seekSet(0);

  // ROM banks
  romBanks = 2; // Default 32K
  if (romSize >= 1) { // Calculate rom size
    romBanks = 2 << romSize;
  }

  // Set data pins to output
  dataOut();

  // Set ROM bank hi 0
  writeByte_GB(0x3000, 0);
  // Set ROM bank low 0
  writeByte_GB(0x2000, 0);
  delay(100);

  // Reset flash
  writeByte_GB(0x555, 0xf0);
  delay(100);

  // ID command sequence
  writeByte_GB(0x555, 0xaa);
  writeByte_GB(0x2aa, 0x55);
  writeByte_GB(0x555, 0x90);

  dataIn_GB();

  // Read the two id bytes into a string
  sprintf(flashid, "%02X%02X", readByte_GB(0), readByte_GB(1));

  if (strcmp(flashid, "04D4") == 0) {
    ui->printlnMsg(F("MBM29F033C"));
    ui->printMsg(F("Banks: "));
    ui->printMsg(romBanks);
    ui->printlnMsg(F("/256"));
    ui->flushOutput();
  }
  else if (strcmp(flashid, "0141") == 0) {
    ui->printlnMsg(F("AM29F032B"));
    ui->printMsg(F("Banks: "));
    ui->printMsg(romBanks);
    ui->printlnMsg(F("/256"));
    ui->flushOutput();
  }
  else if (strcmp(flashid, "01AD") == 0) {
    ui->printlnMsg(F("AM29F016B"));
    ui->printMsg(F("Banks: "));
    ui->printMsg(romBanks);
    ui->printlnMsg(F("/128"));
    ui->flushOutput();
  }
  else if (strcmp(flashid, "04AD") == 0) {
    ui->printlnMsg(F("AM29F016D"));
    ui->printMsg(F("Banks: "));
    ui->printMsg(romBanks);
    ui->printlnMsg(F("/128"));
    ui->flushOutput();
  }
  else if (strcmp(flashid, "01D5") == 0) {
    ui->printlnMsg(F("AM29F080B"));
    ui->printMsg(F("Banks: "));
    ui->printMsg(romBanks);
    ui->printlnMsg(F("/64"));
    ui->flushOutput();
  }
  else {
    ui->printMsg(F("Flash ID: "));
    ui->printlnMsg(flashid);
    ui->flushOutput();
    ui->printErrorAndAbort(F("Unknown flashrom"), false);
  }
  dataOut();

  // Reset flash
  writeByte_GB(0x555, 0xf0);

  delay(100);
  ui->printlnMsg(F("Erasing flash"));
  ui->flushOutput();

  // Erase flash
  writeByte_GB(0x555, 0xaa);
  writeByte_GB(0x2aa, 0x55);
  writeByte_GB(0x555, 0x80);
  writeByte_GB(0x555, 0xaa);
  writeByte_GB(0x2aa, 0x55);
  writeByte_GB(0x555, 0x10);

  dataIn_GB();

  // Read the status register
  byte statusReg = readByte_GB(0);

  // After a completed erase D7 will output 1
  while ((statusReg & 0x80) != 0x80) {
    // Blink led
    PORTB ^= (1 << 4);
    delay(100);
    // Update Status
    statusReg = readByte_GB(0);
  }

  // Blankcheck
  ui->printlnMsg(F("Blankcheck"));
  ui->flushOutput();

  // Read x number of banks
  for (unsigned int currBank = 0; currBank < romBanks; currBank++) {
    // Blink led
    PORTB ^= (1 << 4);

    dataOut();

    // Set ROM bank
    writeByte_GB(0x2000, currBank);
    dataIn();

    for (unsigned int currAddr = 0x4000; currAddr < 0x7FFF; currAddr += 512) {
      uint8_t readData[512];
      for (int currByte = 0; currByte < 512; currByte++) {
        readData[currByte] = readByte_GB(currAddr + currByte);
      }
      for (int j = 0; j < 512; j++) {
        if (readData[j] != 0xFF) {
          ui->printlnMsg(F("Not empty"));
          ui->printErrorAndAbort(F("Erase failed"), false);
        }
      }
    }
  }

  if (MBC == 3) {
    ui->printlnMsg(F("Writing flash MBC3"));
    ui->flushOutput();

    // Write flash
    dataOut();

    uint16_t currAddr = 0;
    uint16_t endAddr = 0x3FFF;

    for (unsigned int currBank = 0; currBank < romBanks; currBank++) {
      // Blink led
      PORTB ^= (1 << 4);

      // Set ROM bank
      writeByte_GB(0x2100, currBank);

      if (currBank > 0) {
        currAddr = 0x4000;
        endAddr = 0x7FFF;
      }

      while (currAddr <= endAddr) {
        inputFile.read(sdBuffer, 512);

        for (int currByte = 0; currByte < 512; currByte++) {
          // Write command sequence
          writeByte_GB(0x555, 0xaa);
          writeByte_GB(0x2aa, 0x55);
          writeByte_GB(0x555, 0xa0);
          // Write current byte
          writeByte_GB(currAddr + currByte, sdBuffer[currByte]);

          // Set data pins to input
          dataIn();

          // Setting CS(PH3) and OE/RD(PH6) LOW
          PORTH &= ~((1 << 3) | (1 << 6));

          // Busy check
          while ((PINC & 0x80) != (sdBuffer[currByte] & 0x80)) {
          }

          // Switch CS(PH3) and OE/RD(PH6) to HIGH
          PORTH |= (1 << 3) | (1 << 6);

          // Set data pins to output
          dataOut();
        }
        currAddr += 512;
      }
    }
  }

  else if (MBC == 5) {
    ui->printlnMsg(F("Writing flash MBC5"));
    ui->flushOutput();

    // Write flash
    dataOut();

    for (unsigned int currBank = 0; currBank < romBanks; currBank++) {
      // Blink led
      PORTB ^= (1 << 4);

      // Set ROM bank
      writeByte_GB(0x2000, currBank);
      // 0x2A8000 fix
      writeByte_GB(0x4000, 0x0);

      for (unsigned int currAddr = 0x4000; currAddr < 0x7FFF; currAddr += 512) {
        inputFile.read(sdBuffer, 512);

        for (int currByte = 0; currByte < 512; currByte++) {
          // Write command sequence
          writeByte_GB(0x555, 0xaa);
          writeByte_GB(0x2aa, 0x55);
          writeByte_GB(0x555, 0xa0);
          // Write current byte
          writeByte_GB(currAddr + currByte, sdBuffer[currByte]);

          // Set data pins to input
          dataIn();

          // Setting CS(PH3) and OE/RD(PH6) LOW
          PORTH &= ~((1 << 3) | (1 << 6));

          // Busy check
          while ((PINC & 0x80) != (sdBuffer[currByte] & 0x80)) {
          }

          // Switch CS(PH3) and OE/RD(PH6) to HIGH
          PORTH |= (1 << 3) | (1 << 6);

          // Set data pins to output
          dataOut();
        }
      }
    }
  }

  // Set data pins to input again
  dataIn_GB();

  ui->printlnMsg(F("Verifying"));
  ui->flushOutput();

  // Go back to file beginning
  inputFile.seekSet(0);
  //unsigned int addr = 0;  // unused
  writeErrors = 0;

  // Verify flashrom
  uint16_t romAddress = 0;

  // Read number of banks and switch banks
  for (uint16_t bank = 1; bank < romBanks; bank++) {
    // Switch data pins to output
    dataOut();

    if (romType >= 5) { // MBC2 and above
      writeByte_GB(0x2100, bank); // Set ROM bank
    }
    else { // MBC1
      writeByte_GB(0x6000, 0); // Set ROM Mode
      writeByte_GB(0x4000, bank >> 5); // Set bits 5 & 6 (01100000) of ROM bank
      writeByte_GB(0x2000, bank & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
    }

    // Switch data pins to intput
    dataIn_GB();

    if (bank > 1) {
      romAddress = 0x4000;
    }
    // Blink led
    PORTB ^= (1 << 4);

    // Read up to 7FFF per bank
    while (romAddress <= 0x7FFF) {
      // Fill sdBuffer
      inputFile.read(sdBuffer, 512);
      // Compare
      for (int i = 0; i < 512; i++) {
        if (readByte_GB(romAddress + i) != sdBuffer[i]) {
          writeErrors++;
        }
      }
      romAddress += 512;
    }
  }
  // Close the file:
  inputFile.close();

  if (writeErrors == 0) {
    ui->printlnMsg(F("OK"));
    ui->flushOutput();
  }
  else {
    ui->printMsg(F("Error: "));
    ui->printMsg(writeErrors);
    ui->printlnMsg(F(" bytes "));
    ui->printError(F("did not verify."));
  }
}

//******************************************
// End of File
//******************************************
