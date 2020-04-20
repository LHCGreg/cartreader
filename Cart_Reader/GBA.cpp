//******************************************
// GAME BOY ADVANCE MODULE
//******************************************

#include <Arduino.h>
#include "GBA.h"
#include "filebrowser.h"
#include "ui.h"
#include "globals.h"
#include "utils.h"
#include "SD.h"

/******************************************
   Variables
 *****************************************/
char calcChecksumStr[5];
boolean readType;

/******************************************
   Function prototypes
 *****************************************/
uint8_t saveTypeMenu();
String getNextGBARomOutputPathAndPrintMessage(const String &gameName);
String getNextGBAEepromOutputPathAndPrintMessage(const String &gameName);
String getNextGBASRAMOutputPathAndPrintMessage(const String &gameName);
String getNextGBAFlashSaveOutputPathAndPrintMessage(const String &gameName);
void setROM_GBA();
void getCartInfo_GBA();
void readROM_GBA(const String &outputFilePath);
boolean compare_checksum_GBA(const String &filePath);
void readSRAM_GBA(unsigned long sramSize, uint32_t pos);
void writeSRAM_GBA(const String &inputFilePath, unsigned long sramSize, uint32_t pos);
unsigned long verifySRAM_GBA(const String &filePath, unsigned long sramSize, uint32_t pos);
void idFlash_GBA();
void resetFLASH_GBA();
byte readByteFlash_GBA(unsigned long myAddress);
void writeByteFlash_GBA(unsigned long myAddress, byte myData);
void eraseFLASH_GBA();
boolean blankcheckFLASH_GBA(unsigned long flashSize);
void switchBank_GBA(byte bankNum);
void readFLASH_GBA(const String &outputFilePath, unsigned long flashSize, uint32_t pos);
void writeFLASH_GBA(const String &inputFilePath, unsigned long flashSize, uint32_t pos);
void verifyFLASH_GBA(const String &filePath, unsigned long flashSize, uint32_t pos);
void writeEeprom_GBA(const String &inputFilePath, word eepSize);
void readEeprom_GBA(word eepSize);
void writeBlock_EEP(word startAddr, word eepSize);
void readBlock_EEP(word startAddress, word eepSize);
unsigned long verifyEEP_GBA(const String &filePath, word eepSize);
void flashRepro_GBA();

void gbaMenu() {
  while (true) {
    const __FlashStringHelper *item_ReadROM = F("Read Rom");
    const __FlashStringHelper *item_ReadSave = F("Read Save");
    const __FlashStringHelper *item_WriteSave = F("Write Save");
    const __FlashStringHelper *item_ForceSaveType = F("Force Savetype");
    const __FlashStringHelper *item_FlashRepro = F("Flash Repro");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_ReadROM,
      item_ReadSave,
      item_WriteSave,
      item_ForceSaveType,
      item_FlashRepro,
      item_Back,
    };

    const __FlashStringHelper *answer = ui->askMultipleChoiceQuestion(
      F("GBA Cart Reader"), menu, ARRAY_LENGTH(menu), item_ReadROM);

    if (answer == item_ReadROM) {
      // Read rom
      switch (cartSize) {
        case 0: {
          const __FlashStringHelper *romItem_1MB = F("1MB");
          const __FlashStringHelper *romItem_2MB = F("2MB");
          const __FlashStringHelper *romItem_4MB = F("4MB");
          const __FlashStringHelper *romItem_8MB = F("8MB");
          const __FlashStringHelper *romItem_16MB = F("16MB");
          const __FlashStringHelper *romItem_32MB = F("32MB");
          const __FlashStringHelper *romMenu[] = {
            romItem_1MB,
            romItem_2MB,
            romItem_4MB,
            romItem_8MB,
            romItem_16MB,
            romItem_32MB,
          };

          const __FlashStringHelper *romAnswer = ui->askMultipleChoiceQuestion(
            F("Select ROM size"), romMenu, ARRAY_LENGTH(romMenu), romItem_1MB);

          if (romAnswer == romItem_1MB) {
            cartSize = 0x100000;
          }
          else if (romAnswer == romItem_2MB) {
            cartSize = 0x200000;
          }
          else if (romAnswer == romItem_4MB) {
            cartSize = 0x400000;
          }
          else if (romAnswer == romItem_8MB) {
            cartSize = 0x800000;
          }
          else if (romAnswer == romItem_16MB) {
            cartSize = 0x1000000;
          }
          else if (romAnswer == romItem_32MB) {
            cartSize = 0x2000000;
          }
          else {
            ui->printErrorAndAbort(F("Unknown size"), true);
          }

          break;
        }
        case 1: {
          // 1MB
          cartSize = 0x100000;
          break;
        }
        case 4: {
          // 4MB
          cartSize = 0x400000;
          break;
        }
        case 8: {
          // 8MB
          cartSize = 0x800000;
          break;
        }
        case 16: {
          // 16MB
          cartSize = 0x1000000;
          break;
        }
        case 32: {
          // 32MB
          cartSize = 0x2000000;
          break;
        }
      }
      ui->clearOutput();

      String outputFilePath = getNextGBARomOutputPathAndPrintMessage(romName);
      readROM_GBA(outputFilePath);
      compare_checksum_GBA(outputFilePath);
      ui->printlnMsg(F(""));
      ui->printlnMsg(F("Press Button..."));
      ui->flushOutput();
      ui->waitForUserInput();
    }
    else if (answer == item_ReadSave) {
      if (saveType == 0) {
        saveType = saveTypeMenu();
      }
      switch (saveType) {
        case 1: {
          ui->clearOutput();
          // 4K EEPROM
          readEeprom_GBA(4);
          setROM_GBA();
          break;
        }

        case 2: {
          ui->clearOutput();
          // 64K EEPROM
          readEeprom_GBA(64);
          setROM_GBA();
          break;
        }

        case 3: {
          ui->clearOutput();
          // 256K SRAM/FRAM
          readSRAM_GBA(32768, 0);
          setROM_GBA();
          break;
        }

        case 4: {
          ui->clearOutput();
          String outputFilePath = getNextGBAFlashSaveOutputPathAndPrintMessage(romName);
          // 512K FLASH
          readFLASH_GBA(outputFilePath, 65536, 0);
          setROM_GBA();
          break;
        }

        case 5: {
          ui->clearOutput();
          String outputFilePath = getNextGBAFlashSaveOutputPathAndPrintMessage(romName);
          // 1024K FLASH (divided into two banks)
          switchBank_GBA(0x0);
          setROM_GBA();
          readFLASH_GBA(outputFilePath, 65536, 0);
          switchBank_GBA(0x1);
          setROM_GBA();
          readFLASH_GBA(outputFilePath, 65536, 65536);
          setROM_GBA();
          break;
        }

        case 6: {
          ui->clearOutput();
          // 512K SRAM/FRAM
          readSRAM_GBA(65536, 0);
          setROM_GBA();
          break;
        }
      }
      ui->printlnMsg(F(""));
      ui->printlnMsg(F("Press Button..."));
      ui->flushOutput();
      ui->waitForUserInput();
    }
    else if (answer == item_WriteSave) {
      if (saveType == 0) {
        saveType = saveTypeMenu();
      }

      switch (saveType) {
        case 1: {
          ui->clearOutput();
          // 4K EEPROM
          String inputFilePath = fileBrowser(F("Select eep file"));
          writeEeprom_GBA(inputFilePath, 4);
          uint32_t writeErrors = verifyEEP_GBA(inputFilePath, 4);
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
          setROM_GBA();
          break;
        }

        case 2: {
          ui->clearOutput();
          // 64K EEPROM
          String inputFilePath = fileBrowser(F("Select eep file"));
          writeEeprom_GBA(inputFilePath, 64);
          uint32_t writeErrors = verifyEEP_GBA(inputFilePath, 64);
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
          setROM_GBA();
          break;
        }

        case 3: {
          ui->clearOutput();
          // 256K SRAM/FRAM
          String inputFilePath = fileBrowser(F("Select srm file"));
          writeSRAM_GBA(inputFilePath, 32768, 0);
          uint32_t writeErrors = verifySRAM_GBA(inputFilePath, 32768, 0);
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
          setROM_GBA();
          break;
        }

        case 4: {
          ui->clearOutput();
          // 512K FLASH
          idFlash_GBA();
          resetFLASH_GBA();
          if (strcmp(flashid, "BFD4") != 0) {
            String errorMessage = F("Flashrom Type not supported\nID: ");
            errorMessage.concat(flashid);
            ui->printErrorAndAbort(errorMessage, true);
          }
          eraseFLASH_GBA();
          if (blankcheckFLASH_GBA(65536)) {
            String inputFilePath = fileBrowser(F("Select fla file"));
            writeFLASH_GBA(inputFilePath, 65536, 0);
            verifyFLASH_GBA(inputFilePath, 65536, 0);
          }
          else {
            ui->printError(F("Erase failed"));
          }
          setROM_GBA();
          break;
        }

        case 5: {
          ui->clearOutput();
          // 1M FLASH
          idFlash_GBA();
          resetFLASH_GBA();
          if (strcmp(flashid, "C209") != 0) {
            String errorMessage = F("Flashrom Type not supported\nID: ");
            errorMessage.concat(flashid);
            ui->printErrorAndAbort(errorMessage, true);
          }
          eraseFLASH_GBA();
          // 131072 bytes are divided into two 65536 byte banks
          switchBank_GBA(0x0);
          setROM_GBA();
          String inputFilePath;
          if (blankcheckFLASH_GBA(65536)) {
            inputFilePath = fileBrowser(F("Select fla file"));
            writeFLASH_GBA(inputFilePath, 65536, 0);
            verifyFLASH_GBA(inputFilePath, 65536, 0);
          }
          else {
            ui->printError(F("Erase failed"));
          }
          switchBank_GBA(0x1);
          setROM_GBA();
          if (blankcheckFLASH_GBA(65536)) {
            writeFLASH_GBA(inputFilePath, 65536, 65536);
            verifyFLASH_GBA(inputFilePath, 65536, 65536);
          }
          else {
            ui->printError(F("Erase failed"));
          }
          setROM_GBA();
          break;
        }

        case 6: {
          ui->clearOutput();
          // 512K SRAM/FRAM
          String inputFilePath = fileBrowser(F("Select srm file"));
          writeSRAM_GBA(inputFilePath, 65536, 0);
          uint32_t writeErrors = verifySRAM_GBA(inputFilePath, 65536, 0);
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
          setROM_GBA();
          break;
        }
      }
      ui->printlnMsg(F(""));
      ui->printlnMsg(F("Press Button..."));
      ui->flushOutput();
      ui->waitForUserInput();
    }
    else if (answer == item_ForceSaveType) {
      saveType = saveTypeMenu();
      ui->flushOutput();
    }
    else if (answer == item_FlashRepro) {
      ui->clearOutput();
      flashRepro_GBA();
      ui->printlnMsg(F(""));
      ui->printlnMsg(F("Press Button..."));
      ui->flushOutput();
      ui->waitForUserInput();
      resetArduino();
    }
    else if (answer == item_Back) {
      break;
    }
  }
}

uint8_t saveTypeMenu() {
  const __FlashStringHelper *item_4KEEPROM = F("4K EEPROM");
  const __FlashStringHelper *item_64KEEPROM = F("64K EEPROM");
  const __FlashStringHelper *item_256KRAM = F("256K SRAM/FRAM");
  const __FlashStringHelper *item_512KRAM = F("512K SRAM/FRAM");
  const __FlashStringHelper *item_512KFlashROM = F("512K FLASHROM");
  const __FlashStringHelper *item_1MFlashROM = F("1M FLASHROM");
  const __FlashStringHelper *menu[] = {
    item_4KEEPROM,
    item_64KEEPROM,
    item_256KRAM,
    item_512KRAM,
    item_512KFlashROM,
    item_1MFlashROM,
  };

  const __FlashStringHelper *answer = ui->askMultipleChoiceQuestion(
    F("Select save type"), menu, ARRAY_LENGTH(menu), item_4KEEPROM);

  if (answer == item_4KEEPROM) {
    return 1;
  }
  else if (answer == item_64KEEPROM) {
    return 2;
  }
  else if (answer == item_256KRAM) {
    return 3;
  }
  else if (answer == item_512KRAM) {
    return 6;
  }
  else if (answer == item_512KFlashROM) {
    return 4;
  }
  else if (answer == item_1MFlashROM) {
    return 5;
  }
  else {
    ui->printErrorAndAbort(F("Unknown type"), true);
  }
}

String getNextGBAOutputPathAndPrintMessage(const String &fileType, const String &gameName, const String &extension) {
  return getNextOutputPathAndPrintMessage(F("GBA"), fileType, gameName, extension);
}

String getNextGBARomOutputPathAndPrintMessage(const String &gameName) {
  return getNextOutputPathAndPrintMessage(F("GBA"), F("ROM"), gameName, F(".gba"));
}

String getNextGBAEepromOutputPathAndPrintMessage(const String &gameName) {
  return getNextOutputPathAndPrintMessage(F("GBA"), F("SAVE"), gameName, F(".eep"));
}

String getNextGBASRAMOutputPathAndPrintMessage(const String &gameName) {
  return getNextOutputPathAndPrintMessage(F("GBA"), F("SAVE"), gameName, F(".srm"));
}

String getNextGBAFlashSaveOutputPathAndPrintMessage(const String &gameName) {
  return getNextOutputPathAndPrintMessage(F("GBA"), F("SAVE"), gameName, F(".fla"));
}

/******************************************
   Setup
 *****************************************/
void setup_GBA() {
  setROM_GBA();

  // Print start page
  getCartInfo_GBA();
  ui->clearOutput();

  ui->printMsg(F("Name: "));
  ui->printlnMsg(romName);
  ui->printMsg(F("Cart ID: "));
  ui->printlnMsg(cartID);
  ui->printMsg(F("Rom Size: "));
  if (cartSize == 0)
    ui->printlnMsg(F("Unknown"));
  else {
    ui->printMsg(cartSize);
    ui->printlnMsg(F("MB"));
  }
  ui->printMsg(F("Save: "));
  switch (saveType)
  {
    case 0:
      ui->printlnMsg(F("Unknown"));
      break;

    case 1:
      ui->printlnMsg(F("4K Eeprom"));
      break;

    case 2:
      ui->printlnMsg(F("64K Eeprom"));
      break;

    case 3:
      ui->printlnMsg(F("256K Sram"));
      break;

    case 4:
      ui->printlnMsg(F("512K Flash"));
      break;

    case 5:
      ui->printlnMsg(F("1024K Flash"));
      break;
  }

  ui->printMsg(F("Checksum: "));
  ui->printlnMsg(checksumStr);
  ui->printMsg(F("Version: 1."));
  ui->printlnMsg(romVersion);

  // Wait for user input
  ui->printlnMsg(F("Press Button..."));
  ui->flushOutput();
  ui->waitForUserInput();
}

/******************************************
   Low level functions
*****************************************/
void setROM_GBA() {
  // CS_SRAM(PH0)
  DDRH |= (1 << 0); PORTH |= (1 << 0);
  // CS_ROM(PH3)
  DDRH |= (1 << 3); PORTH |= (1 << 3);
  // WR(PH5)
  DDRH |= (1 << 5); PORTH |= (1 << 5);
  // RD(PH6)
  DDRH |= (1 << 6); PORTH |= (1 << 6);
  // AD0-AD7
  DDRF = 0xFF;
  // AD8-AD15
  DDRK = 0xFF;
  // AD16-AD23
  DDRC = 0xFF;
  // Wait
  delay(500);
}

word readWord_GBA(unsigned long myAddress) {
  // Set address/data ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  DDRC = 0xFF;

  // Divide address by two to get word addressing
  myAddress = myAddress >> 1;

  // Output address to address pins,
  PORTF = myAddress;
  PORTK = myAddress >> 8;
  PORTC = myAddress >> 16;

  // Pull CS(PH3) to LOW
  PORTH &= ~ (1 << 3);

  // Set address/data ports to input
  PORTF = 0x0;
  PORTK = 0x0;
  DDRF = 0x0;
  DDRK = 0x0;

  // Pull RD(PH6) to LOW
  PORTH &= ~ (1 << 6);

  // Delay here or read error with repro
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  word myWord = (PINK << 8) | PINF;

  // Switch RD(PH6) to HIGH
  PORTH |= (1 << 6);

  // Switch CS_ROM(PH3) to HIGH
  PORTH |= (1 << 3);

  return myWord;
}

void writeWord_GBA(unsigned long myAddress, word myWord) {
  // Set address/data ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  DDRC = 0xFF;

  // Divide address by two to get word addressing
  myAddress = myAddress >> 1;

  // Output address to address pins,
  PORTF = myAddress;
  PORTK = myAddress >> 8;
  PORTC = myAddress >> 16;

  // Pull CS(PH3) to LOW
  PORTH &= ~ (1 << 3);

  __asm__("nop\n\t""nop\n\t");

  // Output data
  PORTF = myWord & 0xFF;
  PORTK = myWord >> 8;

  // Pull WR(PH5) to LOW
  PORTH &= ~ (1 << 5);

  __asm__("nop\n\t""nop\n\t");

  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);

  // Switch CS_ROM(PH3) to HIGH
  PORTH |= (1 << 3);
}

// This function swaps bit at positions p1 and p2 in an integer n
word swapBits(word n, word p1, word p2)
{
  // Move p1'th to rightmost side
  word bit1 =  (n >> p1) & 1;

  // Move p2'th to rightmost side
  word bit2 =  (n >> p2) & 1;

  // XOR the two bits */
  word x = (bit1 ^ bit2);

  // Put the xor bit back to their original positions
  x = (x << p1) | (x << p2);

  // XOR 'x' with the original number so that the two sets are swapped
  word result = n ^ x;

  return result;
}

// Some repros have D0 and D1 switched
word readWord_GAB(unsigned long myAddress) {
  word tempWord = swapBits(readWord_GBA(myAddress), 0, 1);
  return tempWord;
}

void writeWord_GAB(unsigned long myAddress, word myWord) {
  writeWord_GBA(myAddress, swapBits(myWord, 0, 1));
}

byte readByte_GBA(unsigned long myAddress) {
  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data port to input
  DDRC = 0x0;

  // Output address to address pins,
  PORTF = myAddress;
  PORTK = myAddress >> 8;

  // Pull OE_SRAM(PH6) to LOW
  PORTH &= ~(1 << 6);
  // Pull CE_SRAM(PH0) to LOW
  PORTH &= ~(1 << 0);

  // Hold address for at least 25ns and wait 150ns before access
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read byte
  byte tempByte = PINC;

  // Pull CE_SRAM(PH0) HIGH
  PORTH |= (1 << 0);
  // Pull OE_SRAM(PH6) HIGH
  PORTH |= (1 << 6);

  return tempByte;
}

void writeByte_GBA(unsigned long myAddress, byte myData) {
  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data port to output
  DDRC = 0xFF;

  // Output address to address pins
  PORTF = myAddress;
  PORTK = myAddress >> 8;
  // Output data to data pins
  PORTC = myData;

  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull WE_SRAM(PH5) to LOW
  PORTH &= ~(1 << 5);
  // Pull CE_SRAM(PH0) to LOW
  PORTH &= ~(1 << 0);

  // Leave WR low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull CE_SRAM(PH0) HIGH
  PORTH |= (1 << 0);
  // Pull WE_SRAM(PH5) HIGH
  PORTH |= (1 << 5);

  // Leave WR high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}

/******************************************
  GBA ROM Functions
*****************************************/
// Read info out of rom header
void getCartInfo_GBA() {
  // Read Header into array
  for (int currWord = 0; currWord < 192; currWord += 2) {
    word tempWord = readWord_GBA(currWord);

    sdBuffer[currWord] = tempWord & 0xFF;
    sdBuffer[currWord + 1] = (tempWord >> 8) & 0xFF;
  }

  // Compare Nintendo logo against known checksum, 156 bytes starting at 0x04
  word logoChecksum = 0;
  for (int currByte = 0x4; currByte < 0xA0; currByte++) {
    logoChecksum += sdBuffer[currByte];
  }

  if (logoChecksum != 0x4B1B) {
    ui->printError(F("CARTRIDGE ERROR"));
    strcpy(romName, "ERROR");
    ui->printlnMsg(F(""));
    ui->printlnMsg(F(""));
    ui->printlnMsg(F(""));
    ui->printlnMsg(F("Press Button to"));
    ui->printlnMsg(F("ignore or powercycle"));
    ui->printlnMsg(F("to try again"));
    ui->flushOutput();
    ui->waitForUserInput();
  }
  else {
    char tempStr2[2];
    char tempStr[5];

    // cart not in list
    cartSize = 0;
    saveType = 0;

    // Get cart ID
    cartID[0] = char(sdBuffer[0xAC]);
    cartID[1] = char(sdBuffer[0xAD]);
    cartID[2] = char(sdBuffer[0xAE]);
    cartID[3] = char(sdBuffer[0xAF]);

    SafeSDFile gbaFile = SafeSDFile::openForReading(F("/gba.txt"));
    // Loop through file
    while (gbaFile.bytesAvailable() > 0) {
      // Read 4 bytes into String, do it one at a time so byte order doesn't get mixed up
      sprintf(tempStr, "%c", gbaFile.readByteOrDie());
      for (byte i = 0; i < 3; i++) {
        sprintf(tempStr2, "%c", gbaFile.readByteOrDie());
        strcat(tempStr, tempStr2);
      }

      // Check if string is a match
      if (strcmp(tempStr, cartID) == 0) {
        // Skip the , in the file
        gbaFile.seekCur(1);

        // Read the next ascii character and subtract 48 to convert to decimal
        cartSize = gbaFile.readByteOrDie() - 48;
        // Remove leading 0 for single digit cart sizes
        if (cartSize != 0) {
          cartSize = cartSize * 10 +  gbaFile.readByteOrDie() - 48;
        }
        else {
          cartSize = gbaFile.readByteOrDie() - 48;
        }

        // Skip the , in the file
        gbaFile.seekCur(1);

        // Read the next ascii character and subtract 48 to convert to decimal
        saveType = gbaFile.readByteOrDie() - 48;
      }
      // If no match, empty string, advance by 7 and try again
      else {
        gbaFile.seekCur(7);
      }
    }
    // Close the file:
    gbaFile.close();

    // Get name
    byte myByte = 0;
    byte myLength = 0;
    for (int addr = 0xA0; addr <= 0xAB; addr++) {
      myByte = sdBuffer[addr];
      if (((char(myByte) >= 48 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 15) {
        romName[myLength] = char(myByte);
        myLength++;
      }
    }

    // Get ROM version
    romVersion = sdBuffer[0xBC];

    // Get Checksum as string
    sprintf(checksumStr, "%02X", sdBuffer[0xBD]);

    // Calculate Checksum
    int calcChecksum = 0x00;
    for (int n = 0xA0; n < 0xBD; n++) {
      calcChecksum -= sdBuffer[n];
    }
    calcChecksum = (calcChecksum - 0x19) & 0xFF;
    // Turn into string
    sprintf(calcChecksumStr, "%02X", calcChecksum);

    // Compare checksum
    if (strcmp(calcChecksumStr, checksumStr) != 0) {
      ui->printMsg(F("Result: "));
      ui->printlnMsg(calcChecksumStr);
      ui->printError(F("Checksum Error"));
      ui->printlnMsg(F(""));
      ui->printlnMsg(F("Press Button..."));
      ui->flushOutput();
      ui->waitForUserInput();
    }
  }
}

// Dump ROM
void readROM_GBA(const String &outputFilePath) {
  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  // Read rom
  for (unsigned int myAddress = 0; myAddress < cartSize; myAddress += 512) {
    // Blink led
    if (myAddress % 16384 == 0)
      PORTB ^= (1 << 4);

    for (int currWord = 0; currWord < 512; currWord += 2) {
      word tempWord = readWord_GBA(myAddress + currWord);
      sdBuffer[currWord] = tempWord & 0xFF;
      sdBuffer[currWord + 1] = (tempWord >> 8) & 0xFF;
    }

    // Write to SD
    outputFile.write(sdBuffer, 512);
  }

  // Close the file:
  outputFile.close();
}

// Calculate the checksum of the dumped rom
boolean compare_checksum_GBA(const String &filePath) {
  ui->printlnMsg(F("Calculating Checksum"));
  ui->flushOutput();

  SafeSDFile romFile = SafeSDFile::openForReading(filePath);
  // Read rom header
  romFile.readOrDie(sdBuffer, 512);
  romFile.close();

  // Calculate Checksum
  int calcChecksum = 0x00;
  for (int n = 0xA0; n < 0xBD; n++) {
    calcChecksum -= sdBuffer[n];
  }
  calcChecksum = (calcChecksum - 0x19) & 0xFF;

  // Turn into string
  sprintf(calcChecksumStr, "%02X", calcChecksum);

  if (strcmp(calcChecksumStr, checksumStr) == 0) {
    ui->printlnMsg(F("Checksum matches"));
    ui->flushOutput();
    return 1;
  }
  else {
    ui->printMsg(F("Result: "));
    ui->printlnMsg(calcChecksumStr);
    ui->printError(F("Checksum Error"));
    return 0;
  }
}


/******************************************
  GBA SRAM SAVE Functions
*****************************************/
void readSRAM_GBA(unsigned long sramSize, uint32_t pos) {
  String outputFilePath = getNextGBASRAMOutputPathAndPrintMessage(romName);

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  // Seek to a new position in the file
  if (pos != 0)
    outputFile.seekCur(pos);

  for (unsigned long currAddress = 0; currAddress < sramSize; currAddress += 512) {
    for (int c = 0; c < 512; c++) {
      // Read byte
      sdBuffer[c] = readByte_GBA(currAddress + c);
    }

    // Write sdBuffer to file
    outputFile.write(sdBuffer, 512);
  }
  // Close the file:
  outputFile.close();

  // Signal end of process
  ui->printlnMsg(F("Done"));
  ui->flushOutput();
}

void writeSRAM_GBA(const String &inputFilePath, unsigned long sramSize, uint32_t pos) {
  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

  // Seek to a new position in the file
  if (pos != 0)
    inputFile.seekCur(pos);

  for (unsigned long currAddress = 0; currAddress < sramSize; currAddress += 512) {
    //fill sdBuffer
    inputFile.read(sdBuffer, 512);

    for (int c = 0; c < 512; c++) {
      // Write byte
      writeByte_GBA(currAddress + c, sdBuffer[c]);
    }
  }
  // Close the file:
  inputFile.close();
  ui->printlnMsg(F("SRAM writing finished"));
  ui->flushOutput();
}

unsigned long verifySRAM_GBA(const String &filePath, unsigned long sramSize, uint32_t pos) {
  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(filePath);
  // Variable for errors
  writeErrors = 0;

  // Seek to a new position in the file
  if (pos != 0)
    inputFile.seekCur(pos);

  for (unsigned long currAddress = 0; currAddress < sramSize; currAddress += 512) {
    //fill sdBuffer
    inputFile.read(sdBuffer, 512);

    for (int c = 0; c < 512; c++) {
      // Read byte
      if (readByte_GBA(currAddress + c) != sdBuffer[c]) {
        writeErrors++;
      }
    }
  }
  // Close the file:
  inputFile.close();
  return writeErrors;
}

/******************************************
  GBA FRAM SAVE Functions
*****************************************/
// MB85R256 FRAM (Ferroelectric Random Access Memory) 32,768 words x 8 bits
void readFRAM_GBA (unsigned long framSize) {
  // Output a HIGH signal on CS_ROM(PH3) WE_SRAM(PH5)
  PORTH |= (1 << 3) | (1 << 5);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;

  // Set data pins to input
  DDRC = 0x00;

  // Output a LOW signal on  CE_SRAM(PH0) and OE_SRAM(PH6)
  PORTH &= ~((1 << 0) | (1 << 6));

  String outputFilePath = getNextGBASRAMOutputPathAndPrintMessage(romName);

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);
  for (unsigned long currAddress = 0; currAddress < framSize; currAddress += 512) {
    for (int c = 0; c < 512; c++) {
      // Pull OE_SRAM(PH6) HIGH
      PORTH |= (1 << 6);

      // Set address
      PORTF = (currAddress + c) & 0xFF;
      PORTK = ((currAddress + c) >> 8) & 0xFF;

      // Arduino running at 16Mhz -> one nop = 62.5ns
      // Leave CS_SRAM HIGH for at least 85ns
      __asm__("nop\n\t""nop\n\t");

      // Pull OE_SRAM(PH6) LOW
      PORTH &= ~ (1 << 6);

      // Hold address for at least 25ns and wait 150ns before access
      __asm__("nop\n\t""nop\n\t""nop\n\t");

      // Read byte
      sdBuffer[c] = PINC;
    }
    // Write sdBuffer to file
    outputFile.write(sdBuffer, 512);
  }
  // Close the file:
  outputFile.close();

  // Signal end of process
  ui->printlnMsg(F("Done"));
  ui->flushOutput();
}

// Write file to SRAM
void writeFRAM_GBA (const String &inputFilePath, unsigned long framSize) {
  // Output a HIGH signal on CS_ROM(PH3) and OE_SRAM(PH6)
  PORTH |= (1 << 3) | (1 << 6);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;

  // Set data port to output
  DDRC = 0xFF;

  // Output a LOW signal on CE_SRAM(PH0) and WE_SRAM(PH5)
  PORTH &= ~((1 << 0) | (1 << 5));

  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  for (unsigned long currAddress = 0; currAddress < framSize; currAddress += 512) {
    //fill sdBuffer
    inputFile.read(sdBuffer, 512);

    for (int c = 0; c < 512; c++) {
      // Output Data on PORTC
      PORTC = sdBuffer[c];

      // Arduino running at 16Mhz -> one nop = 62.5ns
      // Data setup time 50ns
      __asm__("nop\n\t");

      // Pull WE_SRAM (PH5) HIGH
      PORTH |= (1 << 5);

      // Set address
      PORTF = (currAddress + c) & 0xFF;
      PORTK = ((currAddress + c) >> 8) & 0xFF;

      // Leave WE_SRAM (PH5) HIGH for at least 85ns
      __asm__("nop\n\t""nop\n\t");

      // Pull WE_SRAM (PH5) LOW
      PORTH &= ~ (1 << 5);

      // Hold address for at least 25ns and wait 150ns before next write
      __asm__("nop\n\t""nop\n\t""nop\n\t");
    }
  }
  // Close the file:
  inputFile.close();
  ui->printlnMsg(F("SRAM writing finished"));
  ui->flushOutput();
}

// Check if the SRAM was written without any error
unsigned long verifyFRAM_GBA(const String &filePath, unsigned long framSize) {
  // Output a HIGH signal on CS_ROM(PH3) WE_SRAM(PH5)
  PORTH |= (1 << 3) | (1 << 5);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;

  // Set data pins to input
  DDRC = 0x00;

  // Output a LOW signal on  CE_SRAM(PH0) and OE_SRAM(PH6)
  PORTH &= ~((1 << 0) | (1 << 6));

  //open file on sd card
  SafeSDFile romFile = SafeSDFile::openForReading(filePath);

  // Variable for errors
  writeErrors = 0;

  for (unsigned long currAddress = 0; currAddress < framSize; currAddress += 512) {
    //fill sdBuffer
    romFile.read(sdBuffer, 512);

    for (int c = 0; c < 512; c++) {
      // Pull OE_SRAM(PH6) HIGH
      PORTH |= (1 << 6);

      // Set address
      PORTF = (currAddress + c) & 0xFF;
      PORTK = ((currAddress + c) >> 8) & 0xFF;

      // Arduino running at 16Mhz -> one nop = 62.5ns
      // Leave CS_SRAM HIGH for at least 85ns
      __asm__("nop\n\t""nop\n\t");

      // Pull OE_SRAM(PH6) LOW
      PORTH &= ~ (1 << 6);

      // Hold address for at least 25ns and wait 150ns before access
      __asm__("nop\n\t""nop\n\t""nop\n\t");

      // Read byte
      if (PINC != sdBuffer[c]) {
        writeErrors++;
      }
    }
  }

  // Close the file:
  romFile.close();
  return writeErrors;
}

/******************************************
  GBA FLASH SAVE Functions
*****************************************/
// SST 39VF512 Flashrom
void idFlash_GBA() {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data pins to output
  DDRC = 0xFF;

  // Output a LOW signal on CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  // ID command sequence
  writeByteFlash_GBA(0x5555, 0xaa);
  writeByteFlash_GBA(0x2aaa, 0x55);
  writeByteFlash_GBA(0x5555, 0x90);

  // Set data pins to input
  DDRC = 0x00;

  // Output a LOW signal on OE_FLASH(PH6)
  PORTH &= ~(1 << 6);

  // Wait 150ns before reading ID
  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t""nop\n\t""nop\n\t");

  // Read the two id bytes into a string
  sprintf(flashid, "%02X%02X", readByteFlash_GBA(0), readByteFlash_GBA(1));

  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);
}

// Reset FLASH
void resetFLASH_GBA() {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data pins to output
  DDRC = 0xFF;

  // Output a LOW signal on CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  // Reset command sequence
  writeByteFlash_GBA(0x5555, 0xAA);
  writeByteFlash_GBA(0x2AAA, 0x55);
  writeByteFlash_GBA(0x5555, 0xf0);
  writeByteFlash_GBA(0x5555, 0xf0);

  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);

  // Wait
  delay(100);
}

byte readByteFlash_GBA(unsigned long myAddress) {
  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  // Wait until byte is ready to read
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read byte
  byte tempByte = PINC;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempByte;
}

void writeByteFlash_GBA(unsigned long myAddress, byte myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch WE_FLASH(PH5) to LOW
  PORTH &= ~(1 << 5);

  // Leave WE low for at least 40ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch WE_FLASH(PH5) to HIGH
  PORTH |= (1 << 5);

  // Leave WE high for a bit
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}

// Erase FLASH
void eraseFLASH_GBA() {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data pins to output
  DDRC = 0xFF;

  // Output a LOW signal on CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  // Erase command sequence
  writeByteFlash_GBA(0x5555, 0xaa);
  writeByteFlash_GBA(0x2aaa, 0x55);
  writeByteFlash_GBA(0x5555, 0x80);
  writeByteFlash_GBA(0x5555, 0xaa);
  writeByteFlash_GBA(0x2aaa, 0x55);
  writeByteFlash_GBA(0x5555, 0x10);

  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);

  // Wait until all is erased
  delay(500);
}

boolean blankcheckFLASH_GBA (unsigned long flashSize) {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5)
  PORTH |= (1 << 3) | (1 << 5);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set address to 0
  PORTF = 0x00;
  PORTK = 0x00;

  // Set data pins to input
  DDRC = 0x00;
  // Disable Pullups
  //PORTC = 0x00;

  boolean blank = 1;

  // Output a LOW signal on  CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  // Output a LOW signal on OE_FLASH(PH6)
  PORTH &= ~(1 << 6);

  for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 512) {
    // Fill buffer
    for (int c = 0; c < 512; c++) {
      // Read byte
      sdBuffer[c] = readByteFlash_GBA(currAddress + c);
    }
    // Check buffer
    for (unsigned long currByte = 0; currByte < 512; currByte++) {
      if (sdBuffer[currByte] != 0xFF) {
        currByte = 512;
        currAddress = flashSize;
        blank = 0;
      }
    }
  }
  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);

  return blank;
}

// The MX29L010 is 131072 bytes in size and has 16 sectors per bank
// each sector is 4096 bytes, there are 32 sectors total
// therefore the bank size is 65536 bytes, so we have two banks in total
void switchBank_GBA(byte bankNum) {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data pins to output
  DDRC = 0xFF;

  // Output a LOW signal on CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  // Switch bank command sequence
  writeByte_GBA(0x5555, 0xAA);
  writeByte_GBA(0x2AAA, 0x55);
  writeByte_GBA(0x5555, 0xB0);
  writeByte_GBA(0x0000, bankNum);

  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);
}

void readFLASH_GBA(const String &outputFilePath, unsigned long flashSize, uint32_t pos) {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5)
  PORTH |= (1 << 3) | (1 << 5);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set address to 0
  PORTF = 0x00;
  PORTK = 0x00;

  // Set data pins to input
  DDRC = 0x00;

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  // Seek to a new position in the file
  if (pos != 0)
    outputFile.seekCur(pos);

  // Output a LOW signal on CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  // Output a LOW signal on OE_FLASH(PH6)
  PORTH &= ~(1 << 6);

  for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 512) {
    for (int c = 0; c < 512; c++) {
      // Read byte
      sdBuffer[c] = readByteFlash_GBA(currAddress + c);
    }
    // Write sdBuffer to file
    outputFile.write(sdBuffer, 512);
  }
  outputFile.close();

  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);

  // Signal end of process
  ui->printlnMsg(F("Done"));
  ui->flushOutput();
}

void busyCheck_GBA(int currByte) {
  // Set data pins to input
  DDRC = 0x00;
  // Output a LOW signal on OE_FLASH(PH6)
  PORTH &= ~(1 << 6);
  // Read PINC
  while (PINC != sdBuffer[currByte]) {}
  // Output a HIGH signal on OE_FLASH(PH6)
  PORTH |= (1 << 6);
  // Set data pins to output
  DDRC = 0xFF;
}

void writeFLASH_GBA(const String &inputFilePath, unsigned long flashSize, uint32_t pos) {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data port to output
  DDRC = 0xFF;

  ui->printMsg(F("Writing flash..."));
  ui->flushOutput();

  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

  // Seek to a new position in the file
  if (pos != 0)
    inputFile.seekCur(pos);

  // Output a LOW signal on CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 512) {
    //fill sdBuffer
    inputFile.read(sdBuffer, 512);

    for (int c = 0; c < 512; c++) {
      // Write command sequence
      writeByteFlash_GBA(0x5555, 0xaa);
      writeByteFlash_GBA(0x2aaa, 0x55);
      writeByteFlash_GBA(0x5555, 0xa0);
      // Write current byte
      writeByteFlash_GBA(currAddress + c, sdBuffer[c]);

      // Wait
      busyCheck_GBA(c);
    }
  }
  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);

  // Close the file:
  inputFile.close();
  ui->printlnMsg(F("done"));
  ui->flushOutput();
}

// Check if the Flashrom was written without any error
void verifyFLASH_GBA(const String &filePath, unsigned long flashSize, uint32_t pos) {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5)
  PORTH |= (1 << 3) | (1 << 5);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;

  // Set data pins to input
  DDRC = 0x00;

  // Output a LOW signal on CE_FLASH(PH0) and  OE_FLASH(PH6)
  PORTH &= ~((1 << 0) | (1 << 6));

  // Signal beginning of process
  ui->printMsg(F("Verify..."));
  ui->flushOutput();

  unsigned long wrError = 0;

  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(filePath);

  // Seek to a new position in the file
  if (pos != 0)
    inputFile.seekCur(pos);

  for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 512) {
    inputFile.read(sdBuffer, 512);

    for (int c = 0; c < 512; c++) {
      // Read byte
      if (sdBuffer[c] != readByteFlash_GBA(currAddress + c)) {
        wrError++;
      }
    }
  }
  inputFile.close();

  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);

  if (wrError == 0) {
    ui->printlnMsg(F("OK"));
  }
  else {
    ui->printMsg(wrError);
    ui->printError(F(" Errors"));
  }
}

/******************************************
  GBA Eeprom SAVE Functions
*****************************************/
// Write eeprom from file
void writeEeprom_GBA(const String &inputFilePath, word eepSize) {
  ui->clearOutput();
  ui->printMsg(F("Writing eeprom..."));
  ui->flushOutput();

  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  for (word i = 0; i < eepSize * 16; i += 64) {
    // Fill romBuffer
    inputFile.read(sdBuffer, 512);
    // Disable interrupts for more uniform clock pulses
    noInterrupts();
    // Write 512 bytes
    writeBlock_EEP(i, eepSize);
    interrupts();

    // Wait
    delayMicroseconds(200);
  }

  // Close the file:
  inputFile.close();
  ui->printlnMsg(F("done"));
  ui->flushOutput();
}

// Read eeprom to file
void readEeprom_GBA(word eepSize) {
  String outputFilePath = getNextGBAEepromOutputPathAndPrintMessage(romName);

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  // Each block contains 8 Bytes, so for a 8KB eeprom 1024 blocks need to be read
  for (word currAddress = 0; currAddress < eepSize * 16; currAddress += 64) {
    // Disable interrupts for more uniform clock pulses
    noInterrupts();
    // Fill sd Buffer
    readBlock_EEP(currAddress, eepSize);
    interrupts();

    // Write sdBuffer to file
    outputFile.write(sdBuffer, 512);

    // Wait
    delayMicroseconds(200);
  }
  outputFile.close();
}

// Send address as bits to eeprom
void send_GBA(word currAddr, word numBits) {
  for (word addrBit = numBits; addrBit > 0; addrBit--) {
    // If you want the k-th bit of n, then do
    // (n & ( 1 << k )) >> k
    if (((currAddr & ( 1 << (addrBit - 1))) >> (addrBit - 1))) {
      // Set A0(PF0) to High
      PORTF |= (1 << 0);
      // Set WR(PH5) to LOW
      PORTH &= ~ (1 << 5);
      // Set WR(PH5) to High
      PORTH |= (1 << 5);
    }
    else {
      // Set A0(PF0) to Low
      PORTF &= ~ (1 << 0);
      // Set WR(PH5) to LOW
      PORTH &= ~ (1 << 5);
      // Set WR(PH5) to High
      PORTH |= (1 << 5);
    }
  }
}

// Write 512K eeprom block
void writeBlock_EEP(word startAddr, word eepSize) {
  // Setup
  // Set CS_ROM(PH3) WR(PH5) RD(PH6) to Output
  DDRH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Set A0(PF0) to Output
  DDRF |= (1 << 0);
  // Set A23/D7(PC7) to Output
  DDRC |= (1 << 7);

  // Set CS_ROM(PH3) WR(PH5) RD(PH6) to High
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Set A0(PF0) to High
  PORTF |= (1 << 0);
  // Set A23/D7(PC7) to High
  PORTC |= (1 << 7);

  __asm__("nop\n\t""nop\n\t");

  // Write 64*8=512 bytes
  for (word currAddr = startAddr; currAddr < startAddr + 64; currAddr++) {
    // Set CS_ROM(PH3) to LOW
    PORTH &= ~ (1 << 3);

    // Send write request "10"
    // Set A0(PF0) to High
    PORTF |= (1 << 0);
    // Set WR(PH5) to LOW
    PORTH &= ~ (1 << 5);
    // Set WR(PH5) to High
    PORTH |= (1 << 5);
    // Set A0(PF0) to LOW
    PORTF &= ~ (1 << 0);
    // Set WR(PH5) to LOW
    PORTH &= ~ (1 << 5);
    // Set WR(PH5) to High
    PORTH |= (1 << 5);

    // Send either 6 or 14 bit address
    if (eepSize == 4) {
      send_GBA(currAddr, 6);
    }
    else {
      send_GBA(currAddr, 14);
    }

    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Send data
    for (byte currByte = 0; currByte < 8; currByte++) {
      send_GBA(sdBuffer[(currAddr - startAddr) * 8 + currByte], 8);
    }

    // Send stop bit
    // Set A0(PF0) to LOW
    PORTF &= ~ (1 << 0);
    // Set WR(PH5) to LOW
    PORTH &= ~ (1 << 5);
    // WR(PH5) to High
    PORTH |=  (1 << 5);

    // Set CS_ROM(PH3) to High
    PORTH |= (1 << 3);

    // Wait until done
    // Set A0(PF0) to Input
    DDRF &= ~ (1 << 0);

    do {
      // Set  CS_ROM(PH3) RD(PH6) to LOW
      PORTH &= ~((1 << 3) | (1 << 6));
      // Set  CS_ROM(PH3) RD(PH6) to High
      PORTH  |= (1 << 3) | (1 << 6);
    }
    while ((PINF & 0x1) == 0);

    // Set A0(PF0) to Output
    DDRF |= (1 << 0);
  }
}

// Reads 512 bytes from eeprom
void readBlock_EEP(word startAddress, word eepSize) {
  // Setup
  // Set CS_ROM(PH3) WR(PH5) RD(PH6) to Output
  DDRH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Set A0(PF0) to Output
  DDRF |= (1 << 0);
  // Set A23/D7(PC7) to Output
  DDRC |= (1 << 7);

  // Set CS_ROM(PH3) WR(PH5) RD(PH6) to High
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Set A0(PF0) to High
  PORTF |= (1 << 0);
  // Set A23/D7(PC7) to High
  PORTC |= (1 << 7);

  __asm__("nop\n\t""nop\n\t");

  // Read 64*8=512 bytes
  for (word currAddr = startAddress; currAddr < startAddress + 64; currAddr++) {
    // Set CS_ROM(PH3) to LOW
    PORTH &= ~ (1 << 3);

    // Send read request "11"
    // Set A0(PF0) to High
    PORTF |= (1 << 0);
    // Set WR(PH5) to LOW
    PORTH &= ~ (1 << 5);
    // Set WR(PH5) to High
    PORTH |= (1 << 5);
    // Set WR(PH5) to LOW
    PORTH &= ~ (1 << 5);
    // Set WR(PH5) to High
    PORTH |= (1 << 5);

    // Send either 6 or 14 bit address
    if (eepSize == 4) {
      send_GBA(currAddr, 6);
    }
    else {
      send_GBA(currAddr, 14);
    }

    // Send stop bit
    // Set A0(PF0) to LOW
    PORTF &= ~ (1 << 0);
    // Set WR(PH5) to LOW
    PORTH &= ~ (1 << 5);
    // WR(PH5) to High
    PORTH |=  (1 << 5);

    // Set CS_ROM(PH3) to High
    PORTH |= (1 << 3);

    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Read data
    // Set A0(PF0) to Input
    DDRF &= ~ (1 << 0);
    // Set CS_ROM(PH3) to low
    PORTH &= ~(1 << 3);

    // Array that holds the bits
    bool tempBits[65];

    // Ignore the first 4 bits
    for (byte i = 0; i < 4; i++) {
      // Set RD(PH6) to LOW
      PORTH &= ~ (1 << 6);
      // Set RD(PH6) to High
      PORTH  |= (1 << 6);
    }

    // Read the remaining 64bits into array
    for (byte currBit = 0; currBit < 64; currBit++) {
      // Set RD(PH6) to LOW
      PORTH &= ~ (1 << 6);
      // Set RD(PH6) to High
      PORTH  |= (1 << 6);

      // Read bit from A0(PF0)
      tempBits[currBit] = (PINF & 0x1);
    }

    // Set CS_ROM(PH3) to High
    PORTH |= (1 << 3);
    // Set A0(PF0) to High
    PORTF |= (1 << 0);
    // Set A0(PF0) to Output
    DDRF |= (1 << 0);

    // OR 8 bits into one byte for a total of 8 bytes
    for (byte j = 0; j < 64; j += 8) {
      sdBuffer[((currAddr - startAddress) * 8) + (j / 8)] = tempBits[0 + j] << 7 | tempBits[1 + j] << 6 | tempBits[2 + j] << 5 | tempBits[3 + j] << 4 | tempBits[4 + j] << 3 | tempBits[5 + j] << 2 | tempBits[6 + j] << 1 | tempBits[7 + j];
    }
  }
}

// Check if the SRAM was written without any error
unsigned long verifyEEP_GBA(const String &filePath, word eepSize) {
  unsigned long wrError = 0;

  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(filePath);

  // Fill sd Buffer
  for (word currAddress = 0; currAddress < eepSize * 16; currAddress += 64) {
    // Disable interrupts for more uniform clock pulses
    noInterrupts();
    readBlock_EEP(currAddress, eepSize);
    interrupts();

    // Compare
    for (int currByte = 0; currByte < 512; currByte++) {
      if (sdBuffer[currByte] != inputFile.readByteOrDie()) {
        wrError++;
      }
    }
  }
  inputFile.close();
  return wrError;
}

/******************************************
  GBA REPRO Functions (32MB Intel 4000L0YBQ0 and 16MB MX29GL128E)
*****************************************/
// Reset to read mode
void resetIntel_GBA(unsigned long partitionSize) {
  for (unsigned long currPartition = 0; currPartition < cartSize; currPartition += partitionSize) {
    writeWord_GBA(currPartition, 0xFFFF);
  }
}

void resetMX29GL128E_GBA() {
  writeWord_GAB(0, 0xF0);
}

boolean sectorCheckMX29GL128E_GBA() {
  boolean sectorProtect = 0;
  writeWord_GAB(0xAAA, 0xAA);
  writeWord_GAB(0x555, 0x55);
  writeWord_GAB(0xAAA, 0x90);
  for (unsigned long currSector = 0x0; currSector < 0xFFFFFF; currSector += 0x20000) {
    if (readWord_GAB(currSector + 0x04) != 0x0)
      sectorProtect = 1;
  }
  resetMX29GL128E_GBA();
  return sectorProtect;
}

void idFlashrom_GBA() {
  // Send Intel ID command to flashrom
  writeWord_GBA(0, 0x90);
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read flashrom ID
  sprintf(flashid, "%02X%02X", ((readWord_GBA(0x2) >> 8) & 0xFF), (readWord_GBA(0x4) & 0xFF));

  // Intel Strataflash
  if (strcmp(flashid, "8802") == 0 || (strcmp(flashid, "8816") == 0)) {
    cartSize = 0x2000000;
  }
  else {
    // Send swapped MX29GL128E/MSP55LV128 ID command to flashrom
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(0xAAA, 0x90);
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Read flashrom ID
    sprintf(flashid, "%02X%02X", ((readWord_GAB(0x2) >> 8) & 0xFF), (readWord_GAB(0x2) & 0xFF));

    // MX29GL128E or MSP55LV128
    if (strcmp(flashid, "227E") == 0) {
      // MX is 0xC2 and MSP is 0x4 or 0x1
      romType = (readWord_GAB(0x0) & 0xFF);
      cartSize = 0x1000000;
      resetMX29GL128E_GBA();
    }
    else {
      ui->printlnMsg(flashid);
      ui->printErrorAndAbort(F("Unknown Flashid"), false);
    }
  }
}

boolean blankcheckFlashrom_GBA() {
  for (unsigned long currSector = 0; currSector < fileSize; currSector += 0x20000) {
    // Blink led
    PORTB ^= (1 << 4);

    for (unsigned long currByte = 0; currByte < 0x20000; currByte += 2) {
      if (readWord_GBA(currSector + currByte) != 0xFFFF) {
        return 0;
      }
    }
  }
  return 1;
}

void eraseIntel4000_GBA() {
  // If the game is smaller than 16Mbit only erase the needed blocks
  unsigned long lastBlock = 0xFFFFFF;
  if (fileSize < 0xFFFFFF)
    lastBlock = fileSize;

  // Erase 4 blocks with 16kwords each
  for (unsigned long currBlock = 0x0; currBlock < 0x1FFFF; currBlock += 0x8000) {
    // Unlock Block
    writeWord_GBA(currBlock, 0x60);
    writeWord_GBA(currBlock, 0xD0);

    // Erase Command
    writeWord_GBA(currBlock, 0x20);
    writeWord_GBA(currBlock, 0xD0);

    // Read the status register
    word statusReg = readWord_GBA(currBlock);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GBA(currBlock);
    }
  }

  // Erase 126 blocks with 64kwords each
  for (unsigned long currBlock = 0x20000; currBlock < lastBlock; currBlock += 0x1FFFF) {
    // Unlock Block
    writeWord_GBA(currBlock, 0x60);
    writeWord_GBA(currBlock, 0xD0);

    // Erase Command
    writeWord_GBA(currBlock, 0x20);
    writeWord_GBA(currBlock, 0xD0);

    // Read the status register
    word statusReg = readWord_GBA(currBlock);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GBA(currBlock);
    }
    // Blink led
    PORTB ^= (1 << 4);
  }

  // Erase the second chip
  if (fileSize > 0xFFFFFF) {
    // 126 blocks with 64kwords each
    for (unsigned long currBlock = 0x1000000; currBlock < 0x1FDFFFF; currBlock += 0x1FFFF) {
      // Unlock Block
      writeWord_GBA(currBlock, 0x60);
      writeWord_GBA(currBlock, 0xD0);

      // Erase Command
      writeWord_GBA(currBlock, 0x20);
      writeWord_GBA(currBlock, 0xD0);

      // Read the status register
      word statusReg = readWord_GBA(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        statusReg = readWord_GBA(currBlock);
      }
      // Blink led
      PORTB ^= (1 << 4);
    }

    // 4 blocks with 16kword each
    for (unsigned long currBlock = 0x1FE0000; currBlock < 0x1FFFFFF; currBlock += 0x8000) {
      // Unlock Block
      writeWord_GBA(currBlock, 0x60);
      writeWord_GBA(currBlock, 0xD0);

      // Erase Command
      writeWord_GBA(currBlock, 0x20);
      writeWord_GBA(currBlock, 0xD0);

      // Read the status register
      word statusReg = readWord_GBA(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        statusReg = readWord_GBA(currBlock);
      }
      // Blink led
      PORTB ^= (1 << 4);
    }
  }
}

void eraseIntel4400_GBA() {
  // If the game is smaller than 32Mbit only erase the needed blocks
  unsigned long lastBlock = 0x1FFFFFF;
  if (fileSize < 0x1FFFFFF)
    lastBlock = fileSize;

  // Erase 4 blocks with 16kwords each
  for (unsigned long currBlock = 0x0; currBlock < 0x1FFFF; currBlock += 0x8000) {
    // Unlock Block
    writeWord_GBA(currBlock, 0x60);
    writeWord_GBA(currBlock, 0xD0);

    // Erase Command
    writeWord_GBA(currBlock, 0x20);
    writeWord_GBA(currBlock, 0xD0);

    // Read the status register
    word statusReg = readWord_GBA(currBlock);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GBA(currBlock);
    }
  }

  // Erase 255 blocks with 64kwords each
  for (unsigned long currBlock = 0x20000; currBlock < lastBlock; currBlock += 0x1FFFF) {
    // Unlock Block
    writeWord_GBA(currBlock, 0x60);
    writeWord_GBA(currBlock, 0xD0);

    // Erase Command
    writeWord_GBA(currBlock, 0x20);
    writeWord_GBA(currBlock, 0xD0);

    // Read the status register
    word statusReg = readWord_GBA(currBlock);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GBA(currBlock);
    }
    // Blink led
    PORTB ^= (1 << 4);
  }

  /* No need to erase the second chip as max rom size is 32MB
    if (fileSize > 0x2000000) {
    // 255 blocks with 64kwords each
    for (unsigned long currBlock = 0x2000000; currBlock < 0x3FDFFFF; currBlock += 0x1FFFF) {
      // Unlock Block
      writeWord_GBA(currBlock, 0x60);
      writeWord_GBA(currBlock, 0xD0);

      // Erase Command
      writeWord_GBA(currBlock, 0x20);
      writeWord_GBA(currBlock, 0xD0);

      // Read the status register
      word statusReg = readWord_GBA(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        statusReg = readWord_GBA(currBlock);
      }
      // Blink led
      PORTB ^= (1 << 4);
    }

    // 4 blocks with 16kword each
    for (unsigned long currBlock = 0x3FE0000; currBlock < 0x3FFFFFF; currBlock += 0x8000) {
      // Unlock Block
      writeWord_GBA(currBlock, 0x60);
      writeWord_GBA(currBlock, 0xD0);

      // Erase Command
      writeWord_GBA(currBlock, 0x20);
      writeWord_GBA(currBlock, 0xD0);

      // Read the status register
      word statusReg = readWord_GBA(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        statusReg = readWord_GBA(currBlock);
      }
      // Blink led
      PORTB ^= (1 << 4);
    }
    }*/
}

void sectorEraseMSP55LV128_GBA() {
  unsigned long lastSector = 0xFFFFFF;

  // Erase 256 sectors with 64kbytes each
  unsigned long currSector;
  for (currSector = 0x0; currSector < lastSector; currSector += 0x10000) {
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(0xAAA, 0x80);
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(currSector, 0x30);

    // Read the status register
    word statusReg = readWord_GAB(currSector);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GAB(currSector);
    }
    // Blink LED
    PORTB ^= (1 << 4);
  }
}

void sectorEraseMX29GL128E_GBA() {
  unsigned long lastSector = 0xFFFFFF;

  // Erase 128 sectors with 128kbytes each
  unsigned long currSector;
  for (currSector = 0x0; currSector < lastSector; currSector += 0x20000) {
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(0xAAA, 0x80);
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(currSector, 0x30);

    // Read the status register
    word statusReg = readWord_GAB(currSector);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GAB(currSector);
    }
    // Blink LED
    PORTB ^= (1 << 4);
  }
}

void writeIntel4000_GBA(SafeSDFile &inputFile) {
  for (unsigned long currBlock = 0; currBlock < fileSize; currBlock += 0x20000) {
    // Blink led
    PORTB ^= (1 << 4);

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < 0x20000; currSdBuffer += 512) {
      // Fill SD buffer
      inputFile.read(sdBuffer, 512);

      // Write 32 words at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 64) {
        // Unlock Block
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer, 0x60);
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer, 0xD0);

        // Buffered program command
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer, 0xE8);

        // Check Status register
        word statusReg = readWord_GBA(currBlock + currSdBuffer + currWriteBuffer);
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          statusReg = readWord_GBA(currBlock + currSdBuffer + currWriteBuffer);
        }

        // Write word count (minus 1)
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer, 0x1F);

        // Write buffer
        for (byte currByte = 0; currByte < 64; currByte += 2) {
          // Join two bytes into one word
          word currWord = ( ( sdBuffer[currWriteBuffer + currByte + 1] & 0xFF ) << 8 ) | ( sdBuffer[currWriteBuffer + currByte] & 0xFF );
          writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer + currByte, currWord);
        }

        // Write Buffer to Flash
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer + 62, 0xD0);

        // Read the status register at last written address
        statusReg = readWord_GBA(currBlock + currSdBuffer + currWriteBuffer + 62);
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          statusReg = readWord_GBA(currBlock + currSdBuffer + currWriteBuffer + 62);
        }
      }
    }
  }
}

void writeMSP55LV128_GBA(SafeSDFile &inputFile) {
  for (unsigned long currSector = 0; currSector < fileSize; currSector += 0x10000) {
    // Blink led
    PORTB ^= (1 << 4);

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < 0x10000; currSdBuffer += 512) {
      // Fill SD buffer
      inputFile.read(sdBuffer, 512);

      // Write 16 words at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 32) {
        // Write Buffer command
        writeWord_GAB(0xAAA, 0xAA);
        writeWord_GAB(0x555, 0x55);
        writeWord_GAB(currSector, 0x25);

        // Write word count (minus 1)
        writeWord_GAB(currSector, 0xF);

        // Write buffer
        word currWord;
        for (byte currByte = 0; currByte < 32; currByte += 2) {
          // Join two bytes into one word
          currWord = ( ( sdBuffer[currWriteBuffer + currByte + 1] & 0xFF ) << 8 ) | ( sdBuffer[currWriteBuffer + currByte] & 0xFF );
          writeWord_GBA(currSector + currSdBuffer + currWriteBuffer + currByte, currWord);
        }

        // Confirm write buffer
        writeWord_GAB(currSector, 0x29);

        // Read the status register
        word statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 30);

        while ((statusReg | 0xFF7F) != (currWord | 0xFF7F)) {
          statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 30);
        }
      }
    }
  }
}

void writeMX29GL128E_GBA(SafeSDFile &inputFile) {
  for (unsigned long currSector = 0; currSector < fileSize; currSector += 0x20000) {
    // Blink led
    PORTB ^= (1 << 4);

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < 0x20000; currSdBuffer += 512) {
      // Fill SD buffer
      inputFile.read(sdBuffer, 512);

      // Write 32 words at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 64) {
        // Write Buffer command
        writeWord_GAB(0xAAA, 0xAA);
        writeWord_GAB(0x555, 0x55);
        writeWord_GAB(currSector, 0x25);

        // Write word count (minus 1)
        writeWord_GAB(currSector, 0x1F);

        // Write buffer
        word currWord;
        for (byte currByte = 0; currByte < 64; currByte += 2) {
          // Join two bytes into one word
          currWord = ( ( sdBuffer[currWriteBuffer + currByte + 1] & 0xFF ) << 8 ) | ( sdBuffer[currWriteBuffer + currByte] & 0xFF );
          writeWord_GBA(currSector + currSdBuffer + currWriteBuffer + currByte, currWord);
        }

        // Confirm write buffer
        writeWord_GAB(currSector, 0x29);

        // Read the status register
        word statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 62);

        while ((statusReg | 0xFF7F) != (currWord | 0xFF7F)) {
          statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 62);
        }
      }
    }
  }
}

boolean verifyFlashrom_GBA(const String &filePath) {
  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(filePath);
  writeErrors = 0;

  for (unsigned long currSector = 0; currSector < fileSize; currSector += 131072) {
    // Blink led
    PORTB ^= (1 << 4);
    for (unsigned long currSdBuffer = 0; currSdBuffer < 131072; currSdBuffer += 512) {
      // Fill SD buffer
      inputFile.read(sdBuffer, 512);

      for (int currByte = 0; currByte < 512; currByte += 2) {
        // Join two bytes into one word
        word currWord = ( ( sdBuffer[currByte + 1] & 0xFF ) << 8 ) | ( sdBuffer[currByte] & 0xFF );

        // Compare both
        if (readWord_GBA(currSector + currSdBuffer + currByte) != currWord) {
          writeErrors++;
          inputFile.close();
          return 0;
        }
      }
    }
  }
  // Close the file:
  inputFile.close();
  if (writeErrors == 0) {
    return 1;
  }
  else {
    return 0;
  }
}

void flashRepro_GBA() {
  // Check flashrom ID's
  idFlashrom_GBA();

  if ((strcmp(flashid, "8802") == 0) || (strcmp(flashid, "8816") == 0) || (strcmp(flashid, "227E") == 0)) {
    ui->printMsg(F("ID: "));
    ui->printMsg(flashid);
    ui->printMsg(F(" Size: "));
    ui->printMsg(cartSize / 0x100000);
    ui->printlnMsg(F("MB"));
    // MX29GL128E or MSP55LV128(N)
    if (strcmp(flashid, "227E") == 0) {
      // MX is 0xC2 and MSP55LV128 is 0x4 and MSP55LV128N 0x1
      if (romType == 0xC2) {
        ui->printlnMsg(F("Macronix MX29GL128E"));
      }
      else if ((romType == 0x1) || (romType == 0x4)) {
        ui->printlnMsg(F("Fujitsu MSP55LV128N"));
      }
      else if ((romType == 0x89)) {
        ui->printlnMsg(F("Intel PC28F256M29"));
      }
      else if ((romType == 0x20)) {
        ui->printlnMsg(F("ST M29W128GH"));
      }
      else {
        ui->printMsg(F("romType: 0x"));
        ui->printlnMsg(romType, HEX);
        ui->printErrorAndAbort(F("Unknown manufacturer"), false);
      }
    }
    // Intel 4000L0YBQ0
    else if (strcmp(flashid, "8802") == 0) {
      ui->printlnMsg(F("Intel 4000L0YBQ0"));
    }
    // Intel 4400L0ZDQ0
    else if (strcmp(flashid, "8816") == 0) {
      ui->printlnMsg(F("Intel 4400L0ZDQ0"));
    }
    ui->printlnMsg("");
    ui->printlnMsg(F("This will erase your"));
    ui->printlnMsg(F("Repro Cartridge."));
    ui->printlnMsg(F("Please use 3.3V!"));
    ui->printlnMsg("");
    ui->printlnMsg(F("Press Button"));
    ui->flushOutput();
    ui->waitForUserInput();

    // Launch file browser
    String inputFilePath = fileBrowser(F("Select gba file"));
    ui->clearOutput();
    ui->flushOutput();

    // Open file on sd card
    SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
    // Get rom size from file
    fileSize = inputFile.fileSize();
    ui->printMsg(F("File size: "));
    ui->printMsg(fileSize / 0x100000);
    ui->printlnMsg(F("MB"));
    ui->flushOutput();

    // Erase needed sectors
    if (strcmp(flashid, "8802") == 0) {
      ui->printlnMsg(F("Erasing..."));
      ui->flushOutput();
      eraseIntel4000_GBA();
      resetIntel_GBA(0x200000);
    }
    else if (strcmp(flashid, "8816") == 0) {
      ui->printlnMsg(F("Erasing..."));
      ui->flushOutput();
      eraseIntel4400_GBA();
      resetIntel_GBA(0x200000);
    }
    else if (strcmp(flashid, "227E") == 0) {
      //if (sectorCheckMX29GL128E_GBA()) {
      //print_Error(F("Sector Protected"));
      //}
      //else {
      ui->printlnMsg(F("Erasing..."));
      ui->flushOutput();
      if ((romType == 0xC2) || (romType == 0x89) || (romType == 0x20)) {
        //MX29GL128E
        //PC28F256M29 (0x89)
        sectorEraseMX29GL128E_GBA();
      }
      else if ((romType == 0x1) || (romType == 0x4)) {
        //MSP55LV128(N)
        sectorEraseMSP55LV128_GBA();
      }
      //}
    }
    /* Skip blankcheck to save time
      ui->printMsg(F("Blankcheck..."));
      ui->flushOutput();
      if (blankcheckFlashrom_GBA()) {
      ui->printlnMsg(F("OK"));
    */

    //Write flashrom
    ui->printMsg(F("Writing "));
    ui->printlnMsg(inputFilePath);
    ui->flushOutput();
    if ((strcmp(flashid, "8802") == 0) || (strcmp(flashid, "8816") == 0)) {
      writeIntel4000_GBA(inputFile);
    }
    else if (strcmp(flashid, "227E") == 0) {
      if ((romType == 0xC2) || (romType == 0x89) || (romType == 0x20)) {
        //MX29GL128E (0xC2)
        //PC28F256M29 (0x89)
        writeMX29GL128E_GBA(inputFile);
      }
      else if ((romType == 0x1) || (romType == 0x4)) {
        //MSP55LV128(N)
        writeMSP55LV128_GBA(inputFile);
      }
    }

    // Close the file:
    inputFile.close();

    // Verify
    ui->printMsg(F("Verifying..."));
    ui->flushOutput();
    if (strcmp(flashid, "8802") == 0) {
      // Don't know the correct size so just take some guesses
      resetIntel_GBA(0x8000);
      delay(1000);
      resetIntel_GBA(0x100000);
      delay(1000);
      resetIntel_GBA(0x200000);
      delay(1000);
    }
    else if (strcmp(flashid, "8816") == 0) {
      resetIntel_GBA(0x200000);
      delay(1000);
    }

    else if (strcmp(flashid, "227E") == 0) {
      resetMX29GL128E_GBA();
      delay(1000);
    }
    if (verifyFlashrom_GBA(inputFilePath) == 1) {
      ui->printlnMsg(F("OK"));
      ui->flushOutput();
    }
    else {
      ui->printErrorAndAbort(F("ERROR"), false);
    }
    /* Skipped blankcheck
      }
      else {
      print_Error(F("failed"));
      }
    */
  }
  else {
    ui->printMsg(F("ID: "));
    ui->printlnMsg(flashid);
    ui->printErrorAndAbort(F("Unknown Flash ID"), false);
  }
}

//******************************************
// End of File
//******************************************
