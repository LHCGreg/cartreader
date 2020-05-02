//******************************************
// NINTENDO POWER  MODULE
//******************************************
//  (GB Memory starts at around line 1740)

#include <Arduino.h>
#include "NP.h"
#include "SNES.h"
#include "FLASH.h"
#include "filebrowser.h"
#include "RGB_LED.h"
#include "ui.h"
#include "globals.h"
#include "utils.h"
#include "SD.h"

/******************************************
   SF Memory Cassette
******************************************/

/******************************************
   SF Memory Clock Source
******************************************/
// The clock signal for the SF Memory cassette
// is generated with the Adafruit Clock Generator
// or a similar external clock source

/******************************************
   Variables
 *****************************************/
// SF Memory status
byte sfmReady = 0;

// SF Memory Menu
boolean hasMenu = true;
byte numGames = 0;

// Arrays that hold game info
int gameSize[8];
int saveSize[8];
byte gameAddress[8];
byte gameVersion[8];
char gameCode[8][10];
boolean hirom[8];

/******************************************
   Function prototypes
 *****************************************/
void sfmGameMenu();
String getNextSFMFlashOutputPathAndPrintMessage();
String getNextSFMRomOutputPathAndPrintMessage(const String &romName);
String getNextNPMappingOutputPath();
String getNextGBMRomOutputPath();
String getNextGBMMappingOutputPath();
void getGames();
void controlIn_SFM();
byte readBank_SFM(byte myBank, word myAddress);
void getCartInfo_SFM();
boolean checkcart_SFM();
void readROM_SFM(const String &outputFilePath);
void resetFlash_SFM(int startBank);
void idFlash_SFM(int startBank);
void busyCheck_SFM(byte startBank);
void readFlash_SFM();
void printMapping();
void readMapping();
void eraseMapping(byte startBank);
byte blankcheckMapping_SFM();
void writeMapping_SFM(const String &inputFilePath, byte startBank, uint32_t pos);
boolean unlockHirom();
byte send_SFM(byte command);
void write_SFM(const String &inputFilePath, int startBank, uint32_t pos);
byte readByte_GBM(word myAddress);
void readROM_GBM(word numBanks);
void send_GBM(byte myCommand);
boolean readFlashID_GBM();
void eraseFlash_GBM();
boolean blankcheckFlash_GBM();
void writeFlash_GBM(const String &inputFilePath);
void readMapping_GBM();
void eraseMapping_GBM();
boolean blankcheckMapping_GBM();
void writeMapping_GBM(const String &inputFilePath);

/******************************************
  Menu
*****************************************/
void sfmMenu() {
  while (true) {
    const __FlashStringHelper *item_GameMenu = F("Game Menu");
    const __FlashStringHelper *item_FlashMenu = F("Flash Menu");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_GameMenu,
      item_FlashMenu,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("SF Memory"), menu, ARRAY_LENGTH(menu), item_GameMenu);

    if (answer == item_GameMenu) {
      sfmGameMenu();
    }
    else if (answer == item_FlashMenu) {
      mode = CartReaderMode::SFMFlash;
      sfmFlashMenu();
    }
    else if (answer == item_Back) {
      break;
    }
  }
}

void sfmGameMenu() {
  // Switch to hirom all
  if (send_SFM(0x04) == 0x2A) {
    delay(300);

    // Fill arrays with data
    getGames();

    if (hasMenu) {
      // Create submenu options
      String menuOptionsSFMGames[8];
      char gameCode10Bytes[11];
      gameCode10Bytes[10] = '\0';
      for (int i = 0; i < numGames; i++) {
        strncpy(gameCode10Bytes, gameCode[i], 10);
        menuOptionsSFMGames[i] = gameCode10Bytes;
      }

      uint8_t gameSubMenu = ui.askMultipleChoiceQuestion(F("Select Game"), menuOptionsSFMGames, numGames, 0);

      // Switch to game
      send_SFM(gameSubMenu + 0x80);
      delay(200);
      // Check for successfull switch
      byte timeout = 0;
      while (readBank_SFM(0, 0x2400) != 0x7D) {
        delay(200);
        // Try again
        send_SFM(gameSubMenu + 0x80);
        delay(200);
        timeout++;
        // Abort, something is wrong
        if (timeout == 5) {
          ui.clearOutput();
          ui.printMsg(F("Game "));
          ui.printMsg(gameSubMenu + 0x80, HEX);
          ui.printlnMsg(F(" Timeout"));
          ui.printlnMsg(readBank_SFM(0, 0x2400), HEX);
          ui.printlnMsg(F(""));
          ui.printErrorAndAbort(F("Powercycle SFM cart"), false);
        }
      }
      // Copy gameCode to romName in case of japanese chars in romName
      strcpy(romName, gameCode[gameSubMenu + 1]);

      // Print info
      getCartInfo_SFM();
      mode = CartReaderMode::SFMGame;
      sfmGameOptions();
    }
    else {
      // No menu so switch to only game
      // Switch to game
      send_SFM(0x80);
      delay(200);

      // Copy gameCode to romName in case of japanese chars in romName
      strcpy(romName, gameCode[0]);

      // Print info
      getCartInfo_SFM();
      mode = CartReaderMode::SFMGame;
    }
  }
  else {
    ui.printError(F("Switch to HiRom failed"));
  }
}

void sfmGameOptions() {
  while (true) {
    const __FlashStringHelper *item_ReadSRAM = F("Read Sram");
    const __FlashStringHelper *item_ReadGame = F("Read Game");
    const __FlashStringHelper *item_WriteSRAM = F("Write Sram");
    const __FlashStringHelper *item_SwitchGame = F("Switch Game");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_ReadSRAM,
      item_ReadGame,
      item_WriteSRAM,
      item_SwitchGame,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("SFM Game Menu"), menu, ARRAY_LENGTH(menu), item_ReadSRAM);

    if (answer == item_ReadSRAM) {
      ui.clearOutput();
      String outputFilePath = getNextSnesSRAMOutputFilePathAndPrintMessage(romName);
      readSRAM(outputFilePath);
    }
    else if (answer == item_ReadGame) {
      ui.clearOutput();
      String outputFilePath = getNextSFMRomOutputPathAndPrintMessage(romName);
      readROM_SFM(outputFilePath);
      compare_checksum(outputFilePath);
    }
    else if (answer == item_WriteSRAM) {
      ui.clearOutput();
      String inputFilePath = fileBrowser(F("Select srm file"));
      writeSRAM(inputFilePath);
      uint32_t writeErrors = verifySRAM(inputFilePath);
      if (writeErrors == 0) {
        ui.printlnMsg(F("Verified OK"));
        ui.flushOutput();
      }
      else {
        ui.printMsg(F("Error: "));
        ui.printMsg(writeErrors);
        ui.printlnMsg(F(" bytes "));
        ui.printError(F("did not verify."));
      }
    }
    else if (answer == item_SwitchGame) {
      sfmGameMenu();
    }
    else if (answer == item_Back) {
      break;
    }

    if (answer != item_SwitchGame) {
      ui.printlnMsg(F(""));
      ui.printlnMsg(F("Press Button..."));
      ui.flushOutput();
      ui.waitForUserInput();
    }
  }
}

void sfmFlashMenu() {
  while (true) {
    const __FlashStringHelper *item_ReadFlash = F("Read Flash");
    const __FlashStringHelper *item_WriteFlash = F("Write Flash");
    const __FlashStringHelper *item_PrintMapping = F("Print Mapping");
    const __FlashStringHelper *item_ReadMapping = F("Read Mapping");
    const __FlashStringHelper *item_WriteMapping = F("Write Mapping");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_ReadFlash,
      item_WriteFlash,
      item_PrintMapping,
      item_ReadMapping,
      item_WriteMapping,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("SFM Flash Menu"), menu, ARRAY_LENGTH(menu), item_ReadFlash);
    
    if (answer == item_ReadFlash) {
      // Clear screen
      ui.clearOutput();

      // Reset to HIROM ALL
      romType = 1;
      ui.printMsg(F("Switch to HiRom..."));
      ui.flushOutput();
      if (send_SFM(0x04) == 0x2A) {
        ui.printlnMsg(F("OK"));
        ui.flushOutput();

        // Reset flash
        resetFlash_SFM(0xC0);
        resetFlash_SFM(0xE0);

        flashSize = 4194304;
        numBanks = 64;

        // Read flash
        readFlash_SFM();
      }
      else {
        ui.printError(F("Switch to HiRom failed"));
      }
    }
    else if (answer == item_WriteFlash) {
      // Clear screen
      ui.clearOutput();

      // Print warning
      ui.printlnMsg(F("Attention"));
      ui.printlnMsg(F("This will erase your"));
      ui.printlnMsg(F("NP Cartridge."));
      ui.printlnMsg("");
      ui.printlnMsg(F("Press Button"));
      ui.printlnMsg(F("to continue"));
      ui.flushOutput();
      ui.waitForUserInput();

      // Clear screen
      ui.clearOutput();

      // Launch file browser
      String inputFilePath = fileBrowser(F("Select 4MB file"));
      ui.clearOutput();
      flashSize = 2097152;
      numBanks = 32;
      ui.printlnMsg(F("Writing 1st rom"));
      ui.flushOutput();
      // Program 1st flashrom
      write_SFM(inputFilePath, 0xC0, 0);
      ui.clearOutput();
      ui.printlnMsg(F("Writing 2nd rom"));
      ui.flushOutput();
      // Program 2nd flashrom
      write_SFM(inputFilePath, 0xE0, 2097152);
    }
    else if (answer == item_PrintMapping) {
      // Clear screen
      ui.clearOutput();

      // Reset to HIROM ALL
      romType = 1;
      ui.printMsg(F("Switch to HiRom..."));
      ui.flushOutput();
      if (send_SFM(0x04) == 0x2A) {
        ui.printlnMsg(F("OK"));
        ui.flushOutput();
        idFlash_SFM(0xC0);
        if (strcmp(flashid, "c2f3") == 0) {
          idFlash_SFM(0xE0);
          if (strcmp(flashid, "c2f3") == 0) {
            // Reset flash
            resetFlash_SFM(0xC0);
            resetFlash_SFM(0xE0);
            delay(100);
            // Clear screen
            ui.clearOutput();
            printMapping();
            resetFlash_SFM(0xC0);
            resetFlash_SFM(0xE0);
          }
          else {
            ui.printErrorAndAbort(F("Error: Wrong Flash ID"), false);
          }
        }
        else {
          ui.printErrorAndAbort(F("Error: Wrong Flash ID"), false);
        }
      }
      else {
        ui.printError(F("failed"));
      }
    }
    else if (answer == item_ReadMapping) {
      // Clear screen
      ui.clearOutput();

      // Reset to HIROM ALL
      romType = 1;
      ui.printMsg(F("Switch to HiRom..."));
      ui.flushOutput();
      if (send_SFM(0x04) == 0x2A) {
        ui.printlnMsg(F("OK"));
        ui.flushOutput();
        idFlash_SFM(0xC0);
        if (strcmp(flashid, "c2f3") == 0) {
          idFlash_SFM(0xE0);
          if (strcmp(flashid, "c2f3") == 0) {
            // Reset flash
            resetFlash_SFM(0xC0);
            resetFlash_SFM(0xE0);
            delay(100);
            readMapping();
            resetFlash_SFM(0xC0);
            resetFlash_SFM(0xE0);
          }
          else {
            ui.printErrorAndAbort(F("Error: Wrong Flash ID"), false);
          }
        }
        else {
          ui.printErrorAndAbort(F("Error: Wrong Flash ID"), false);
        }
      }
      else {
        ui.printError(F("failed"));
      }
    }
    else if (answer == item_WriteMapping) {
      ui.clearOutput();

      // Print warning
      ui.printlnMsg(F("Attention"));
      ui.printlnMsg(F("This will erase your"));
      ui.printlnMsg(F("NP Cartridge."));
      ui.printlnMsg("");
      ui.printlnMsg(F("Press Button"));
      ui.printlnMsg(F("to continue"));
      ui.flushOutput();
      ui.waitForUserInput();

      // Clear screen
      ui.clearOutput();

      // Erase mapping
      eraseMapping(0xD0);
      eraseMapping(0xE0);
      ui.printMsg(F("Blankcheck..."));
      ui.flushOutput();
      if (blankcheckMapping_SFM()) {
        ui.printlnMsg(F("OK"));
        ui.flushOutput();

        // Clear screen
        ui.clearOutput();

        // Launch file browser
        String inputFilePath = fileBrowser(F("Select MAP file"));
        ui.clearOutput();
        ui.flushOutput();

        // Write mapping
        writeMapping_SFM(inputFilePath, 0xD0, 0);
        writeMapping_SFM(inputFilePath, 0xE0, 256);
      }
      else {
        ui.printlnMsg(F("Nope"));
      }
    }
    else if (answer == item_Back) {
      mode = CartReaderMode::SFM;
      break;
    }

    ui.printlnMsg(F(""));
    ui.printlnMsg(F("Press Button..."));
    ui.flushOutput();
    ui.waitForUserInput();
  }
}

String getNextSFMFlashOutputPathAndPrintMessage() {
  return getNextOutputPathWithNumberedFilenameAndPrintMessage(F("NP"), F("SFM"), F(".bin"));
}

String getNextSFMRomOutputPathAndPrintMessage(const String &romName) {
  return getNextOutputPathWithNumberedFolderAndPrintMessage(F("NP"), F(""), romName, F(".sfc"));
}

String getNextNPMappingOutputPath() {
  return getNextOutputPathWithNumberedFilename(F("NP"), F("NP"), F(".MAP"));
}

String getNextGBMRomOutputPath() {
  return getNextOutputPathWithNumberedFilename(F("NP"), F(".GBM"), F(".bin"));
}

String getNextGBMMappingOutputPath() {
  return getNextOutputPathWithNumberedFilename(F("NP"), F(".GBM"), F(".map"));
}

// Read the games from the menu area
void getGames() {
  // Set data pins to input
  dataIn();
  // Set control pins to input
  controlIn_SFM();

  // Check if menu is present
  byte menuString[] = {0x4D, 0x45, 0x4E, 0x55, 0x20, 0x50, 0x52, 0x4F, 0x47, 0x52, 0x41, 0x4D};
  for (int i = 0; i < 12; i++) {
    if (menuString[i] != readBank_SFM(0xC0, 0x7FC0 + i)) {
      hasMenu = false;
    }
  }

  if (hasMenu) {
    // Count number of games
    for (word i = 0x0000; i < 0xE000; i += 0x2000) {
      if (readBank_SFM(0xC6, i) == numGames )
        numGames++;
    }

    // Get game info
    for (int i = 0; i < numGames; i++) {
      // Read starting address and size
      gameAddress[i] = 0xC0 + readBank_SFM(0xC6, i * 0x2000 + 0x01) * 0x8;
      gameSize[i] = readBank_SFM(0xC6, i * 0x2000 + 0x03) * 128;
      saveSize[i] = readBank_SFM(0xC6, i * 0x2000 + 0x05) / 8;

      //check if hirom
      if (readBank_SFM(gameAddress[i], 0xFFD5) == 0x31) {
        hirom[i] = true;
      }
      else if (readBank_SFM(gameAddress[i], 0xFFD5) == 0x21) {
        hirom[i] = true;
      }
      else {
        hirom[i] = false;
      }

      if (hirom[i]) {
        gameVersion[i] = readBank_SFM(gameAddress[i], 0xFFDB);
      } else {
        gameVersion[i] = readBank_SFM(gameAddress[i], 0x7FDB);
      }

      // Read game code
      byte myByte = 0;
      byte myLength = 0;
      for (int j = 0; j < 9; j++) {
        myByte = readBank_SFM(0xC6, i * 0x2000 + 0x07 + j);
        // Remove funny characters
        if (((char(myByte) >= 44 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 9) {
          gameCode[i][myLength] = char(myByte);
          myLength++;
        }
      }
      // End char array in case game code is less than 9 chars
      gameCode[i][myLength] = '\0';
    }
  }
  else {
    //check if hirom
    if (readBank_SFM(0xC0, 0xFFD5) == 0x31) {
      hirom[0] = true;
    }
    else {
      hirom[0] = false;
    }

    if (hirom[0]) {
      gameVersion[0] = readBank_SFM(0xC0, 0xFFDB);
      gameCode[0][0] = 'G';
      gameCode[0][1] = 'A';
      gameCode[0][2] = 'M';
      gameCode[0][3] = 'E';
      gameCode[0][4] = '-';
      gameCode[0][5] = char(readBank_SFM(0xC0, 0xFFB2));
      gameCode[0][6] = char(readBank_SFM(0xC0, 0xFFB3));
      gameCode[0][7] = char(readBank_SFM(0xC0, 0xFFB4));
      gameCode[0][8] = char(readBank_SFM(0xC0, 0xFFB5));
      gameCode[0][9] = '\0';

      byte romSizeExp = readBank_SFM(0xC0, 0xFFD7) - 7;
      gameSize[0] = 1;
      while (romSizeExp--)
        gameSize[0] *= 2;
    }
    else {
      gameVersion[0] = readBank_SFM(0xC0, 0x7FDB);
      gameCode[0][0] = 'G';
      gameCode[0][1] = 'A';
      gameCode[0][2] = 'M';
      gameCode[0][3] = 'E';
      gameCode[0][4] = '-';
      gameCode[0][5] = char(readBank_SFM(0xC0, 0x7FB2));
      gameCode[0][6] = char(readBank_SFM(0xC0, 0x7FB3));
      gameCode[0][7] = char(readBank_SFM(0xC0, 0x7FB4));
      gameCode[0][8] = char(readBank_SFM(0xC0, 0x7FB5));
      gameCode[0][9] = '\0';

      byte romSizeExp = readBank_SFM(0xC0, 0x7FD7) - 7;
      gameSize[0] = 1;
      while (romSizeExp--)
        gameSize[0] *= 2;
    }
  }
}

/******************************************
   Setup
 *****************************************/
void setup_SFM() {
  // Set cicrstPin(PG1) to Output
  DDRG |= (1 << 1);
  // Output a high signal to disable snesCIC
  PORTG |= (1 << 1);
  // Set cichstPin(PG0) to Input
  DDRG &= ~(1 << 0);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //BA0-BA7
  DDRL = 0xFF;

  // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) RD(PH6)
  DDRH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);
  // Switch RST(PH0) and WR(PH5) to HIGH
  PORTH |= (1 << 0) | (1 << 5);
  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));

  // Set IRQ(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  //PORTH |= (1 << 4);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Enable Internal Pullups
  //PORTC = 0xFF;

  // Unused pins
  // Set CPU Clock(PH1) to Output
  DDRH |= (1 << 1);
  //PORTH &= ~(1 << 1);

  // Adafruit Clock Generator
  clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
  clockgen.set_freq(2147727200ULL, SI5351_CLK0);

  // start outputting master clock
  clockgen.output_enable(SI5351_CLK1, 0);
  clockgen.output_enable(SI5351_CLK2, 0);
  clockgen.output_enable(SI5351_CLK0, 1);

  // Wait until all is stable
  delay(500);

  // Switch to HiRom All
  byte timeout = 0;
  send_SFM(0x04);
  delay(200);
  while (readBank_SFM(0, 0x2400) != 0x2A) {
    delay(100);
    // Try again
    send_SFM(0x04);
    delay(100);
    timeout++;
    // Abort, something is wrong
    if (timeout == 5) {
      ui.printlnMsg(F("Hirom All Timeout"));
      ui.printlnMsg(F(""));
      ui.printlnMsg(F(""));
      ui.printErrorAndAbort(F("Powercycle SFM cart"), false);
    }
  }
}

/******************************************
   I/O Functions
 *****************************************/
// Switch control pins to write
void controlOut_SFM() {
  // Switch RD(PH6) and WR(PH5) to HIGH
  PORTH |= (1 << 6) | (1 << 5);
  // Switch CS(PH3) to LOW
  PORTH &= ~(1 << 3);
}

// Switch control pins to read
void controlIn_SFM() {
  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);
  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));
}

/******************************************
   Low level functions
 *****************************************/
// Write one byte of data to a location specified by bank and address, 00:0000
void writeBank_SFM(byte myBank, word myAddress, byte myData) {
  PORTL = myBank;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch WR(PH5) to LOW
  PORTH &= ~(1 << 5);

  // Leave WR low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);

  // Leave WR high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}

// Read one byte of data from a location specified by bank and address, 00:0000
byte readBank_SFM(byte myBank, word myAddress) {
  PORTL = myBank;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns -> 1000ns total
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  byte tempByte = PINC;
  return tempByte;
}

/******************************************
  SNES ROM Functions
******************************************/
void getCartInfo_SFM() {
  // Print start page
  if (checkcart_SFM() == 0) {
    // Checksum either corrupt or 0000
    errorLvl = 1;
    rgb.setColor(255, 0, 0);
    ui.clearOutput();
    ui.printlnMsg(F("ERROR"));
    ui.printlnMsg(F("Rom header corrupt"));
    ui.printlnMsg(F("or missing"));
    ui.flushOutput();
    ui.waitForUserInput();
    // ui.waitForUserInput() clears errors but in this case we still have an error
    errorLvl = 1;
  }

  ui.clearOutput();
  ui.printMsg(F("Name: "));
  ui.printlnMsg(romName);
  ui.printlnMsg(F(" "));

  ui.printMsg(F("Version: 1."));
  ui.printlnMsg(romVersion);

  ui.printMsg(F("Checksum: "));
  ui.printlnMsg(checksumStr);

  ui.printMsg(F("Size: "));
  ui.printMsg(romSize);
  ui.printlnMsg(F("Mbit "));

  ui.printMsg(F("Type: "));
  if (romType == 1)
    ui.printlnMsg(F("HiROM"));
  else if (romType == 0)
    ui.printlnMsg(F("LoROM"));
  else
    ui.printlnMsg(romType);

  ui.printMsg(F("Banks: "));
  ui.printlnMsg(numBanks);

  ui.printMsg(F("Sram: "));
  ui.printMsg(sramSize);
  ui.printlnMsg(F("Kbit"));
  ui.printlnMsg(F("Press Button"));
  ui.flushOutput();
  // Wait for user input
  ui.waitForUserInput();
}

// Read header information
boolean checkcart_SFM() {
  // set control to read
  dataIn();

  // Get Checksum as string
  sprintf(checksumStr, "%02X%02X", readBank_SFM(0, 65503), readBank_SFM(0, 65502));

  romType = readBank_SFM(0, 0xFFD5);
  if ((romType >> 5) != 1) {  // Detect invalid romType byte due to too long ROM name (22 chars)
    romType = 0; // LoROM   // Krusty's Super Fun House (U) 1.0 & Contra 3 (U)
  }
  else {
    romType &= 1; // Must be LoROM or HiROM
  }

  // Check RomSize
  byte romSizeExp = readBank_SFM(0, 65495) - 7;
  romSize = 1;
  while (romSizeExp--)
    romSize *= 2;

  numBanks = (long(romSize) * 1024 * 1024 / 8) / (32768 + (long(romType) * 32768));

  //Check SD card for alt config
  checkAltConf();

  // Get name
  byte myByte = 0;
  byte myLength = 0;
  for (unsigned int i = 65472; i < 65492; i++) {
    myByte = readBank_SFM(0, i);
    if (((char(myByte) >= 48 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 15) {
      romName[myLength] = char(myByte);
      myLength++;
    }
  }
  // If name consists out of all japanese characters use game code
  if (myLength == 0) {
    // Get rom code
    romName[0] = 'S';
    romName[1] = 'H';
    romName[2] = 'V';
    romName[3] = 'C';
    romName[4] = '-';
    for (unsigned int i = 0; i < 4; i++) {
      myByte = readBank_SFM(0, 0xFFB2 + i);
      if (((char(myByte) >= 48 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 4) {
        romName[myLength + 5] = char(myByte);
        myLength++;
      }
    }
    if (myLength == 0) {
      // Rom code unknown
      romName[0] = 'U';
      romName[1] = 'N';
      romName[2] = 'K';
      romName[3] = 'N';
      romName[4] = 'O';
      romName[5] = 'W';
      romName[6] = 'N';
    }
  }

  // Read sramSizeExp
  byte sramSizeExp = readBank_SFM(0, 0xFFD8);

  // Calculate sramSize
  if (sramSizeExp != 0) {
    sramSizeExp = sramSizeExp + 3;
    sramSize = 1;
    while (sramSizeExp--)
      sramSize *= 2;
  }
  else {
    sramSize = 0;
  }

  // ROM Version
  romVersion = readBank_SFM(0, 65499);

  // Test if checksum is equal to reverse checksum
  if (((word(readBank_SFM(0, 65500)) + (word(readBank_SFM(0, 65501)) * 256)) + (word(readBank_SFM(0, 65502)) + (word(readBank_SFM(0, 65503)) * 256))) == 65535 ) {
    if (strcmp("0000", checksumStr) == 0) {
      return 0;
    }
    else {
      return 1;
    }
  }
  // Either rom checksum is wrong or no cart is inserted
  else {
    return 0;
  }
}

// Read rom to SD card
void readROM_SFM(const String &outputFilePath) {
  // Set control
  dataIn();
  controlIn_SFM();

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  // Check if LoROM or HiROM...
  if (romType == 0) {
    ui.printlnMsg(F("Dumping LoRom..."));
    ui.flushOutput();

    // Read up to 96 banks starting at bank 0×00.
    for (int currBank = 0; currBank < numBanks; currBank++) {
      // Dump the bytes to SD 512B at a time
      for (long currByte = 32768; currByte < 65536; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SFM(currBank, currByte + c);
        }
        outputFile.write(sdBuffer, 512);
      }
    }
  }
  // Dump High-type ROM
  else {
    ui.printlnMsg(F("Dumping HiRom..."));
    ui.flushOutput();

    for (int currBank = 192; currBank < (numBanks + 192); currBank++) {
      for (long currByte = 0; currByte < 65536; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SFM(currBank, currByte + c);
        }
        outputFile.write(sdBuffer, 512);
      }
    }
  }
  // Close the file:
  outputFile.close();

  // Signal end of process
  ui.printMsg(F("Saved as "));
  ui.printlnMsg(outputFilePath);
}

/******************************************
   29F1601 flashrom functions (NP)
 *****************************************/
// Reset the MX29F1601 flashrom, startbank is 0xC0 for first and 0xE0 for second flashrom
void resetFlash_SFM(int startBank) {
  // Configure control pins
  controlOut_SFM();
  // Set data pins to output
  dataOut();

  // Reset command sequence
  if (romType) {
    writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
    writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
    writeBank_SFM(startBank, 0x5555L * 2, 0xf0);
  }
  else {
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xaa);
    writeBank_SFM(0, 0x8000 + 0x2AAAL * 2, 0x55);
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xf0);
  }
}

// Print flashrom manufacturer and device ID
void idFlash_SFM(int startBank) {
  // Configure control pins
  controlOut_SFM();
  // Set data pins to output
  dataOut();

  if (romType) {
    // ID command sequence
    writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
    writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
    writeBank_SFM(startBank, 0x5555L * 2, 0x90);

    // Set data pins to input again
    dataIn();
    // Set control pins to input
    controlIn_SFM();

    // Read the two id bytes into a string
    sprintf(flashid, "%x%x", readBank_SFM(startBank, 0x00), readBank_SFM(startBank, 0x02));
  }
  else {
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xaa);
    writeBank_SFM(0, 0x8000 + 0x2AAAL * 2, 0x55);
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0x90);

    // Set data pins to input again
    dataIn();
    // Set control pins to input
    controlIn_SFM();

    // Read the two id bytes into a string
    sprintf(flashid, "%x%x", readBank_SFM(0, 0x8000), readBank_SFM(0, 0x8000 + 0x02));
  }
}

// Write the flashroms by reading a file from the SD card, pos defines where in the file the reading/writing should start
void writeFlash_SFM(const String &inputFilePath, int startBank, uint32_t pos) {
  ui.clearOutput();
  ui.printMsg(F("Writing Bank 0x"));
  ui.printMsg(startBank, HEX);
  ui.printMsg(F("..."));
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

  // Seek to a new position in the file
  if (pos != 0)
    inputFile.seekCur(pos);

  // Configure control pins
  controlOut_SFM();
  // Set data pins to output
  dataOut();

  if (romType) {
    // Write hirom
    for (int currBank = startBank; currBank < startBank + numBanks; currBank++) {
      // Fill SDBuffer with 1 page at a time then write it repeat until all bytes are written
      for (unsigned long currByte = 0; currByte < 0x10000; currByte += 128) {
        inputFile.read(sdBuffer, 128);
        // Write command sequence
        writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
        writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
        writeBank_SFM(startBank, 0x5555L * 2, 0xa0);

        for (byte c = 0; c < 128; c++) {

          // Write one byte of data
          writeBank_SFM(currBank, currByte + c, sdBuffer[c]);

          if (c == 127) {
            // Write the last byte twice or else it won't write at all
            writeBank_SFM(currBank, currByte + c, sdBuffer[c]);
          }
        }
        // Wait until write is finished
        busyCheck_SFM(startBank);
      }
    }
  }
  else {
    // Write lorom
    for (int currBank = 0; currBank < numBanks; currBank++) {
      for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte += 128) {
        inputFile.read(sdBuffer, 128);
        // Write command sequence
        writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xaa);
        writeBank_SFM(0, 0x8000 + 0x2AAAL * 2, 0x55);
        writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xa0);

        for (byte c = 0; c < 128; c++) {
          // Write one byte of data
          writeBank_SFM(currBank, currByte + c, sdBuffer[c]);

          if (c == 127) {
            // Write the last byte twice or else it won't write at all
            writeBank_SFM(currBank, currByte + c, sdBuffer[c]);
          }
        }
        // Wait until write is finished
        busyCheck_SFM(startBank);
      }
    }
  }
  // Close the file:
  inputFile.close();
  ui.printlnMsg("");
}

// Delay between write operations based on status register
void busyCheck_SFM(byte startBank) {
  // Set data pins to input
  dataIn();
  // Set control pins to input and therefore pull CE low and latch status register content
  controlIn_SFM();

  // Read register
  readBank_SFM(startBank, 0x0000);

  // Read D7 while D7 = 0
  //1 = B00000001, 1 << 7 = B10000000, PINC = B1XXXXXXX (X = don't care), & = bitwise and
  while (!(PINC & (1 << 7))) {
    // CE or OE must be toggled with each subsequent status read or the
    // completion of a program or erase operation will not be evident.
    // Switch RD(PH6) to HIGH
    PORTH |= (1 << 6);

    // one nop ~62.5ns
    __asm__("nop\n\t");

    // Switch RD(PH6) to LOW
    PORTH &= ~(1 << 6);

    // one nop ~62.5ns
    __asm__("nop\n\t");
  }

  // Configure control pins
  controlOut_SFM();
  // Set data pins to output
  dataOut();
}

// Erase the flashrom to 0xFF
void eraseFlash_SFM(int startBank) {

  // Configure control pins
  controlOut_SFM();
  // Set data pins to output
  dataOut();

  if (romType) {
    // Erase command sequence
    writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
    writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
    writeBank_SFM(startBank, 0x5555L * 2, 0x80);
    writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
    writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
    writeBank_SFM(startBank, 0x5555L * 2, 0x10);
  }
  else {
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xaa);
    writeBank_SFM(0, 0x8000 + 0x2AAAL * 2, 0x55);
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0x80);
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xaa);
    writeBank_SFM(0, 0x8000 + 0x2AAAL * 2, 0x55);
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0x10);
  }

  // Wait for erase to complete
  busyCheck_SFM(startBank);
}

// Check if an erase succeeded, return 1 if blank and 0 if not
byte blankcheck_SFM(int startBank) {

  // Set data pins to input again
  dataIn();
  // Set control pins to input
  controlIn_SFM();

  byte blank = 1;
  if (romType) {
    for (int currBank = startBank; currBank < startBank + numBanks; currBank++) {
      for (unsigned long currByte = 0; currByte < 0x10000; currByte++) {
        if (readBank_SFM(currBank, currByte) != 0xFF) {
          currBank =  startBank + numBanks;
          blank = 0;
        }
      }
    }
  }
  else {
    for (int currBank = 0; currBank < numBanks; currBank++) {
      for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte++) {
        if (readBank_SFM(currBank, currByte) != 0xFF) {
          currBank = numBanks;
          blank = 0;
        }
      }
    }
  }
  return blank;
}

// Check if a write succeeded, returns 0 if all is ok and number of errors if not
unsigned long verifyFlash_SFM(const String &filePath, int startBank, uint32_t pos) {
  unsigned long  verified = 0;

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(filePath);

  // Set file starting position
  inputFile.seekCur(pos);

  // Set data pins to input
  dataIn();
  // Set control pins to input
  controlIn_SFM();

  if (romType) {
    for (int currBank = startBank; currBank < startBank + numBanks; currBank++) {
      for (unsigned long currByte = 0; currByte < 0x10000; currByte += 512) {
        // Fill SDBuffer
        inputFile.read(sdBuffer, 512);
        for (int c = 0; c < 512; c++) {
          if (readBank_SFM(currBank, currByte + c) != sdBuffer[c]) {
            verified++;
          }
        }
      }
    }
  }
  else {
    for (int currBank = 0; currBank < numBanks; currBank++) {
      for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte += 512) {
        // Fill SDBuffer
        inputFile.read(sdBuffer, 512);
        for (int c = 0; c < 512; c++) {
          if (readBank_SFM(currBank, currByte + c) != sdBuffer[c]) {
            verified++;
          }
        }
      }
    }
  }
  // Close the file:
  inputFile.close();

  // Return 0 if verified ok, or number of errors
  return verified;
}

// Read flashroms and save them to the SD card
void readFlash_SFM() {
  // Set data pins to input
  dataIn();
  // Set control pins to input
  controlIn_SFM();

  String outputFilePath = getNextSFMFlashOutputPathAndPrintMessage();

  // Open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  if (romType) {
    for (int currBank = 0xC0; currBank < 0xC0 + numBanks; currBank++) {
      for (unsigned long currByte = 0; currByte < 0x10000; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SFM(currBank, currByte + c);
        }
        outputFile.write(sdBuffer, 512);
      }
    }
  }
  else {
    for (int currBank = 0; currBank < numBanks; currBank++) {
      for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SFM(currBank, currByte + c);
        }
        outputFile.write(sdBuffer, 512);
      }
    }
  }
  // Close the file:
  outputFile.close();
  ui.printlnMsg("");
  ui.printlnMsg(F("Finished reading"));
  ui.flushOutput();
}

// Display protected sectors/banks as 0xc2 and unprotected as 0x00
void readSectorProtection_SFM(byte startBank) {

  // Configure control pins
  controlOut_SFM();
  // Set data pins to output
  dataOut();

  // Display Sector Protection Status
  writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
  writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
  writeBank_SFM(startBank, 0x5555L * 2, 0x90);

  // Configure control pins
  controlIn_SFM();
  // Set data pins to output
  dataIn();
  ui.clearOutput();
  for (int i = 0; i <= 0x1F; i++) {
    ui.printMsg(F("Sector: 0x"));
    ui.printMsg(startBank + i, HEX);
    ui.printMsg(F(" Sector Protect: 0x"));
    ui.printlnMsg(readBank_SFM(startBank + i, 0x04), HEX);
  }
  ui.flushOutput();
}

// Read the current mapping from the hidden "page buffer" and print it
void printMapping() {
  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset to defaults
  writeBank_SFM(0xC0, 0x0000, 0x38);
  writeBank_SFM(0xC0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SFM(0xC0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SFM(0xC0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SFM(0xC0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Read the mapping out of the first chip
  char buffer[3];

  for (unsigned int currByte = 0xFF00; currByte < 0xFF50; currByte += 10) {
    for (int c = 0; c < 10; c++) {
      itoa (readBank_SFM(0xC0, currByte + c), buffer, 16);
      for (size_t i = 0; i < 2 - strlen(buffer); i++) {
        ui.printMsg("0");
      }
      // Now print the significant bits
      ui.printMsg(buffer);
    }
    ui.printlnMsg("");
  }
  ui.flushOutput();

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset Flash
  writeBank_SFM(0xC0, 0x5555L * 2, 0xaa);
  writeBank_SFM(0xC0, 0x2AAAL * 2, 0x55);
  writeBank_SFM(0xC0, 0x5555L * 2, 0xf0);

  // Reset Flash
  writeBank_SFM(0xE0, 0x5555L * 2, 0xaa);
  writeBank_SFM(0xE0, 0x2AAAL * 2, 0x55);
  writeBank_SFM(0xE0, 0x5555L * 2, 0xf0);

  // Switch to read
  dataIn();
  controlIn_SFM();
}

// Read the current mapping from the hidden "page buffer"
void readMapping() {
  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset to defaults
  writeBank_SFM(0xC0, 0x0000, 0x38);
  writeBank_SFM(0xC0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SFM(0xC0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SFM(0xC0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SFM(0xC0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SFM();

  String outputFilePath = getNextNPMappingOutputPath();

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    outputFile.writeByte(readBank_SFM(0xC0, currByte));
  }

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset to defaults
  writeBank_SFM(0xE0, 0x0000, 0x38);
  writeBank_SFM(0xE0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SFM(0xE0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SFM(0xE0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SFM(0xE0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    outputFile.writeByte(readBank_SFM(0xE0, currByte));
  }

  // Close the file:
  outputFile.close();

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset Flash
  writeBank_SFM(0xC0, 0x5555L * 2, 0xaa);
  writeBank_SFM(0xC0, 0x2AAAL * 2, 0x55);
  writeBank_SFM(0xC0, 0x5555L * 2, 0xf0);

  // Reset Flash
  writeBank_SFM(0xE0, 0x5555L * 2, 0xaa);
  writeBank_SFM(0xE0, 0x2AAAL * 2, 0x55);
  writeBank_SFM(0xE0, 0x5555L * 2, 0xf0);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Signal end of process
  ui.printMsg(F("Saved to "));
  ui.printlnMsg(outputFilePath);
  ui.flushOutput();
}

void eraseMapping(byte startBank) {
  if (unlockHirom()) {
    // Get ID
    idFlash_SFM(startBank);
    if (strcmp(flashid, "c2f3") == 0) {
      resetFlash_SFM(startBank);

      // Switch to write
      dataOut();
      controlOut_SFM();

      // Prepare to erase/write Page Buffer
      writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
      writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
      writeBank_SFM(startBank, 0x5555L * 2, 0x77);
      // Erase Page Buffer
      writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
      writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
      writeBank_SFM(startBank, 0x5555L * 2, 0xe0);

      // Wait until complete
      busyCheck_SFM(startBank);

      // Switch to read
      dataIn();
      controlIn_SFM();
    }
    else {
      ui.printErrorAndAbort(F("Error: Wrong Flash ID"), false);
    }
  }
  else {
    ui.printErrorAndAbort(F("Unlock failed"), false);
  }
}

// Check if the current mapping is all 0xFF
byte blankcheckMapping_SFM() {
  byte blank = 1;

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset to defaults
  writeBank_SFM(0xC0, 0x0000, 0x38);
  writeBank_SFM(0xC0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SFM(0xC0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SFM(0xC0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SFM(0xC0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    if (readBank_SFM(0xC0, currByte) != 0xFF) {
      blank = 0;
    }
  }

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset to defaults
  writeBank_SFM(0xE0, 0x0000, 0x38);
  writeBank_SFM(0xE0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SFM(0xE0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SFM(0xE0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SFM(0xE0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    if (readBank_SFM(0xE0, currByte) != 0xFF) {
      blank = 0;
    }
  }

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset Flash
  writeBank_SFM(0xC0, 0x5555L * 2, 0xaa);
  writeBank_SFM(0xC0, 0x2AAAL * 2, 0x55);
  writeBank_SFM(0xC0, 0x5555L * 2, 0xf0);

  // Reset Flash
  writeBank_SFM(0xE0, 0x5555L * 2, 0xaa);
  writeBank_SFM(0xE0, 0x2AAAL * 2, 0x55);
  writeBank_SFM(0xE0, 0x5555L * 2, 0xf0);

  // Switch to read
  dataIn();
  controlIn_SFM();

  return blank;
}

void writeMapping_SFM(const String &inputFilePath, byte startBank, uint32_t pos) {
  if (unlockHirom()) {
    // Get ID
    idFlash_SFM(startBank);
    if (strcmp(flashid, "c2f3") == 0) {
      resetFlash_SFM(startBank);

      // Switch to write
      dataOut();
      controlOut_SFM();

      // Open file on sd card
      SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

      // Seek to a new position in the file
      if (pos != 0)
        inputFile.seekCur(pos);

      // Write to Page Buffer
      for (unsigned long currByte = 0xFF00; currByte < 0xFFFF; currByte += 128) {
        // Prepare to erase/write Page Buffer
        writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
        writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
        writeBank_SFM(startBank, 0x5555L * 2, 0x77);
        // Write Page Buffer Command
        writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
        writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
        writeBank_SFM(startBank, 0x5555L * 2, 0x99);

        inputFile.read(sdBuffer, 128);

        for (byte c = 0; c < 128; c++) {
          writeBank_SFM(startBank, currByte + c, sdBuffer[c]);
          // Write last byte twice
          if (c == 127) {
            writeBank_SFM(startBank, currByte + c, sdBuffer[c]);
          }
        }
        busyCheck_SFM(startBank);
      }

      // Close the file:
      inputFile.close();
      ui.printlnMsg("");

      // Switch to read
      dataIn();
      controlIn_SFM();
    }
    else {
      ui.printErrorAndAbort(F("Error: Wrong Flash ID"), false);
    }
  }
  else {
    ui.printErrorAndAbort(F("Unlock failed"), false);
  }
}

/******************************************
  SF Memory functions
*****************************************/
// Switch to HiRom All and unlock Write Protection
boolean unlockHirom() {
  romType = 1;
  ui.printMsg(F("Switch to HiRom..."));
  ui.flushOutput();
  if (send_SFM(0x04) == 0x2A) {
    ui.printlnMsg(F("OK"));
    ui.flushOutput();
    // Unlock Write Protection
    ui.printMsg(F("Enable Write..."));
    ui.flushOutput();
    send_SFM(0x02);
    if (readBank_SFM(0, 0x2401) == 0x4) {
      ui.printlnMsg(F("OK"));
      ui.flushOutput();
      return 1;
    }
    else {
      ui.printlnMsg(F("failed"));
      ui.flushOutput();
      return 0;
    }
  }
  else {
    ui.printlnMsg(F("failed"));
    ui.flushOutput();
    return 0;
  }
}

// Send a command to the MX15001 chip
byte send_SFM(byte command) {
  // Switch to write
  dataOut();
  controlOut_SFM();

  // Write command
  writeBank_SFM(0, 0x2400, 0x09);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Read status
  sfmReady = readBank_SFM(0, 0x2400);

  // Switch to write
  dataOut();
  controlOut_SFM();

  writeBank_SFM(0, 0x2401, 0x28);
  writeBank_SFM(0, 0x2401, 0x84);

  // NP_CMD_06h, send this only if above read has returned 7Dh, not if it's already returning 2Ah
  if (sfmReady == 0x7D) {
    writeBank_SFM(0, 0x2400, 0x06);
    writeBank_SFM(0, 0x2400, 0x39);
  }

  // Write the command
  writeBank_SFM(0, 0x2400, command);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Read status
  sfmReady = readBank_SFM(0, 0x2400);
  return sfmReady;
}

// This function will erase and program the NP cart from a 4MB file off the SD card
void write_SFM(const String &inputFilePath, int startBank, uint32_t pos) {
  // Switch NP cart's mapping
  if (unlockHirom()) {
    // Get ID
    idFlash_SFM(startBank);
    if (strcmp(flashid, "c2f3") == 0) {
      ui.printMsg(F("Flash ID: "));
      ui.printlnMsg(flashid);
      ui.flushOutput();
      resetFlash_SFM(startBank);
      delay(1000);
      // Erase flash
      ui.printMsg(F("Blankcheck..."));
      ui.flushOutput();
      if (blankcheck_SFM(startBank)) {
        ui.printlnMsg(F("OK"));
        ui.flushOutput();
      }
      else {
        ui.printlnMsg(F("Nope"));
        ui.clearOutput();
        ui.printMsg(F("Erasing..."));
        ui.flushOutput();
        eraseFlash_SFM(startBank);
        resetFlash_SFM(startBank);
        ui.printlnMsg(F("Done"));
        ui.printMsg(F("Blankcheck..."));
        ui.flushOutput();
        if (blankcheck_SFM(startBank)) {
          ui.printlnMsg(F("OK"));
          ui.flushOutput();
        }
        else {
          ui.printErrorAndAbort(F("Could not erase flash"), false);
        }
      }
      // Write flash
      writeFlash_SFM(inputFilePath, startBank, pos);

      // Reset flash
      resetFlash_SFM(startBank);

      // Checking for errors
      ui.printMsg(F("Verifying..."));
      ui.flushOutput();
      writeErrors = verifyFlash_SFM(inputFilePath, startBank, pos);
      if (writeErrors == 0) {
        ui.printlnMsg(F("OK"));
        ui.flushOutput();
      }
      else {
        ui.printMsg(F("Error: "));
        ui.printMsg(writeErrors);
        ui.printlnMsg(F(" bytes "));
        ui.printErrorAndAbort(F("did not verify."), false);
      }
    }
    else {
      ui.printErrorAndAbort(F("Error: Wrong Flash ID"), false);
    }
  }
  else {
    ui.printErrorAndAbort(F("Unlock failed"), false);
  }
}

/******************************************
  GB Memory Cassette
******************************************/

/******************************************
  Menu
*****************************************/
void gbmMenu() {
  while (true) {
    const __FlashStringHelper *item_ReadID = F("Read ID");
    const __FlashStringHelper *item_ReadFlash = F("Read Flash");
    const __FlashStringHelper *item_EraseFlash = F("Erase Flash");
    const __FlashStringHelper *item_BlankCheck = F("Blankcheck");
    const __FlashStringHelper *item_WriteFlash = F("Write Flash");
    const __FlashStringHelper *item_ReadMapping = F("Read Mapping");
    const __FlashStringHelper *item_WriteMapping = F("Write Mapping");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_ReadID,
      item_ReadFlash,
      item_EraseFlash,
      item_BlankCheck,
      item_WriteFlash,
      item_ReadMapping,
      item_WriteMapping,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("GB Memory Menu"), menu, ARRAY_LENGTH(menu), item_ReadID);

    if (answer == item_ReadID) {
      // Clear screen
      ui.clearOutput();
      readFlashID_GBM();
    }
    else if (answer == item_ReadFlash) {
      // Clear screen
      ui.clearOutput();
      // Print warning
      ui.printlnMsg(F("Attention"));
      ui.printlnMsg(F("Always power cycle"));
      ui.printlnMsg(F("cartreader directly"));
      ui.printlnMsg(F("before reading"));
      ui.printlnMsg("");
      ui.printlnMsg(F("Press Button"));
      ui.printlnMsg(F("to continue"));
      ui.flushOutput();
      ui.waitForUserInput();
      // Clear screen
      ui.clearOutput();

      // Enable access to ports 0120h
      send_GBM(0x09);
      // Map entire flashrom
      send_GBM(0x04);
      // Disable ports 0x0120...
      send_GBM(0x08);
      // Read 1MB rom
      readROM_GBM(64);
    }
    else if (answer == item_EraseFlash) {
      // Clear screen
      ui.clearOutput();
      // Print warning
      ui.printlnMsg(F("Attention"));
      ui.printlnMsg(F("This will erase your"));
      ui.printlnMsg(F("NP Cartridge."));
      ui.printlnMsg("");
      ui.printlnMsg("");
      ui.printlnMsg(F("Press Button"));
      ui.printlnMsg(F("to continue"));
      ui.flushOutput();
      ui.waitForUserInput();
      // Clear screen
      ui.clearOutput();
      eraseFlash_GBM();
    }
    else if (answer == item_BlankCheck) {
      // Clear screen
      ui.clearOutput();
      if (blankcheckFlash_GBM()) {
        ui.printlnMsg(F("OK"));
        ui.flushOutput();
      }
      else {
        ui.printlnMsg(F("ERROR"));
        ui.flushOutput();
      }
    }
    else if (answer == item_WriteFlash) {
      // Clear screen
      ui.clearOutput();

      // Launch file browser
      String inputFilePath = fileBrowser(F("Select 1MB file"));
      ui.clearOutput();

      // Write rom
      writeFlash_GBM(inputFilePath);
    }
    else if (answer == item_ReadMapping) {
      // Clear screen
      ui.clearOutput();

      // Read mapping
      readMapping_GBM();
    }
    else if (answer == item_WriteMapping) {
      // Clear screen
      ui.clearOutput();

      // Print warning
      ui.printlnMsg(F("Attention"));
      ui.printlnMsg(F("This will erase your"));
      ui.printlnMsg(F("NP Cartridge's"));
      ui.printlnMsg(F("mapping data"));
      ui.printlnMsg("");
      ui.printlnMsg(F("Press Button"));
      ui.printlnMsg(F("to continue"));
      ui.flushOutput();
      ui.waitForUserInput();

      // Clear screen
      ui.clearOutput();

      // Launch file browser
      String inputFilePath = fileBrowser(F("Select MAP file"));
      ui.clearOutput();
      ui.flushOutput();

      // Erase mapping
      eraseMapping_GBM();
      if (blankcheckMapping_GBM()) {
        ui.printlnMsg(F("OK"));
        ui.flushOutput();
      }
      else {
        ui.printError(F("Erasing failed"));
        break;
      }

      // Write mapping
      writeMapping_GBM(inputFilePath);
    }
    else if (answer == item_Back) {
      break;
    }

    ui.printlnMsg(F(""));
    ui.printlnMsg(F("Press Button..."));
    ui.flushOutput();
    ui.waitForUserInput();
  }
}

/******************************************
  Setup
*****************************************/
void setup_GBM() {
  // Set RST(PH0) to Input
  DDRH &= ~(1 << 0);
  // Activate Internal Pullup Resistors
  PORTH |= (1 << 0);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;

  // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) RD(PH6)
  DDRH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Output a high signal on all pins, pins are active low therefore everything is disabled now
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;

  delay(400);

  // Check for Nintendo Power GB Memory cart
  byte timeout = 0;

  // First byte of NP register is always 0x21
  while (readByte_GBM(0x120) != 0x21) {
    // Enable ports 0x120h (F2)
    send_GBM(0x09);
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    timeout++;
    if (timeout > 10) {
      ui.printlnMsg(F("Error: Time Out"));
      ui.printErrorAndAbort(F("Please power cycle"), false);
    }
  }
}

/**********************
  LOW LEVEL
**********************/
// Read one word out of the cartridge
byte readByte_GBM(word myAddress) {
  // Set data pins to Input
  DDRC = 0x0;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~(1 << 3);
  PORTH &= ~(1 << 6);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  byte tempByte = PINC;

  // Switch CS(PH3) and RD(PH6) to HIGH
  PORTH |= (1 << 6);
  PORTH |= (1 << 3);

  return tempByte;
}

// Write one word to data pins of the cartridge
void writeByte_GBM(word myAddress, byte myData) {
  // Set data pins to Output
  DDRC = 0xFF;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTC = myData;

  // Pull CS(PH3) and write(PH5) low
  PORTH &= ~(1 << 3);
  PORTH &= ~(1 << 5);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull CS(PH3) and write(PH5) high
  PORTH |= (1 << 5);
  PORTH |= (1 << 3);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Set data pins to Input (or read errors??!)
  DDRC = 0x0;
}

/**********************
  HELPER FUNCTIONS
**********************/
void printSdBuffer(word startByte, word numBytes) {
  for (unsigned int currByte = 0; currByte < numBytes; currByte += 10) {
    for (byte c = 0; c < 10; c++) {
      // Convert to char array so we don't lose leading zeros
      char currByteStr[2];
      sprintf(currByteStr, "%02X", sdBuffer[startByte + currByte + c]);
      ui.printMsg(currByteStr);
    }
    // Add a new line every 10 bytes
    ui.printlnMsg("");
  }
  ui.flushOutput();
}

void readROM_GBM(word numBanks) {
  ui.printlnMsg(F("Reading Rom..."));
  ui.flushOutput();

  String outputFilePath = getNextGBMRomOutputPath();

  // Open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  // Read rom
  word currAddress = 0;

  for (word currBank = 1; currBank < numBanks; currBank++) {
    // Set rom bank
    writeByte_GBM(0x2100, currBank);

    // Switch bank start address
    if (currBank > 1) {
      currAddress = 0x4000;
    }

    for (; currAddress < 0x7FFF; currAddress += 512) {
      for (int currByte = 0; currByte < 512; currByte++) {
        sdBuffer[currByte] = readByte_GBM(currAddress + currByte);
      }
      outputFile.write(sdBuffer, 512);
    }
  }

  // Close the file:
  outputFile.close();

  // Signal end of process
  ui.printMsg(F("Saved to "));
  ui.printlnMsg(outputFilePath);
  ui.flushOutput();
}

/**********************
  GB Memory Functions
**********************/
void send_GBM(byte myCommand) {
  switch (myCommand) {
    case 0x01:
      //CMD_01h -> ???
      writeByte_GBM(0x0120, 0x01);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x02:
      //CMD_02h -> Write enable Step 2
      writeByte_GBM(0x0120, 0x02);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x03:
      //CMD_03h -> Undo write Step 2
      writeByte_GBM(0x0120, 0x03);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x04:
      //CMD_04h -> Map entire flashrom (MBC4 mode)
      writeByte_GBM(0x0120, 0x04);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x05:
      //CMD_05h -> Map menu (MBC5 mode)
      writeByte_GBM(0x0120, 0x05);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x08:
      //CMD_08h -> disable writes/reads to/from special Nintendo Power registers (those at 0120h..013Fh)
      writeByte_GBM(0x0120, 0x08);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x09:
      //CMD_09h Wakeup -> re-enable access to ports 0120h..013Fh
      writeByte_GBM(0x0120, 0x09);
      writeByte_GBM(0x0121, 0xAA);
      writeByte_GBM(0x0122, 0x55);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x0A:
      //CMD_0Ah -> Write enable Step 1
      writeByte_GBM(0x0120, 0x0A);
      writeByte_GBM(0x0125, 0x62);
      writeByte_GBM(0x0126, 0x04);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x10:
      //CMD_10h -> disable writes to normal MBC registers (such like 2100h)
      writeByte_GBM(0x0120, 0x10);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x11:
      //CMD_11h -> re-enable access to MBC registers like 0x2100
      writeByte_GBM(0x0120, 0x11);
      writeByte_GBM(0x013F, 0xA5);
      break;

    default:
      ui.printErrorAndAbort(F("Unknown Command"), false);
      break;
  }
}

void send_GBM(byte myCommand, word myAddress, byte myData) {
  byte myAddrLow = myAddress & 0xFF;
  byte myAddrHigh = (myAddress >> 8) & 0xFF;

  switch (myCommand) {
    case 0x0F:
      // CMD_0Fh -> Write address/byte to flash
      writeByte_GBM(0x0120, 0x0F);
      writeByte_GBM(0x0125, myAddrHigh);
      writeByte_GBM(0x0126, myAddrLow);
      writeByte_GBM(0x0127, myData);
      writeByte_GBM(0x013F, 0xA5);
      break;

    default:
      ui.printErrorAndAbort(F("Unknown Command"), false);
      break;
  }
}

void switchGame_GBM(byte myData) {
  // Enable ports 0x0120 (F2)
  send_GBM(0x09);

  //CMD_C0h -> map selected game without reset
  writeByte_GBM(0x0120, 0xC0 & myData);
  writeByte_GBM(0x013F, 0xA5);
}

void resetFlash_GBM() {
  // Enable ports 0x0120 (F2)
  send_GBM(0x09);

  // Send reset command
  writeByte_GBM(0x2100, 0x01);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0xF0);
  delay(100);
}

boolean readFlashID_GBM() {
  // Enable ports 0x0120 (F2)
  send_GBM(0x09);

  writeByte_GBM(0x2100, 0x01);
  // Read ID command
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x90);

  // Read the two id bytes into a string
  sprintf(flashid, "%02X%02X", readByte_GBM(0), readByte_GBM(1));
  if (strcmp(flashid, "C289") == 0) {
    ui.printMsg(F("Flash ID: "));
    ui.printlnMsg(flashid);
    ui.flushOutput();
    resetFlash_GBM();
    return 1;
  }
  else {
    ui.printMsg(F("Flash ID: "));
    ui.printlnMsg(flashid);
    ui.printErrorAndAbort(F("Unknown Flash ID"), false);
    resetFlash_GBM();
    return 0;
  }
}

void eraseFlash_GBM() {
  ui.printlnMsg(F("Erasing..."));
  ui.flushOutput();

  //enable access to ports 0120h
  send_GBM(0x09);
  // Enable write
  send_GBM(0x0A);
  send_GBM(0x2);

  // Unprotect sector 0
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x60);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x40);

  // Wait for unprotect to complete
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Send erase command
  send_GBM(0x0F, 0x5555, 0xaa);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x80);
  send_GBM(0x0F, 0x5555, 0xaa);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x10);

  // Wait for erase to complete
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Reset flashrom
  resetFlash_GBM();
}

boolean blankcheckFlash_GBM() {
  ui.printMsg(F("Blankcheck..."));
  ui.flushOutput();

  //enable access to ports 0120h (F2)
  send_GBM(0x09);

  // Map entire flashrom
  send_GBM(0x04);
  // Disable ports 0x0120...
  send_GBM(0x08);

  // Read rom
  word currAddress = 0;

  for (byte currBank = 1; currBank < 64; currBank++) {
    // Set rom bank
    writeByte_GBM(0x2100, currBank);

    // Switch bank start address
    if (currBank > 1) {
      currAddress = 0x4000;
    }

    for (; currAddress < 0x7FFF; currAddress++) {
      if (readByte_GBM(currAddress) != 0xFF) {
        return 0;
      }
    }
  }
  return 1;
}

void writeFlash_GBM(const String &inputFilePath) {
  ui.printMsg(F("Writing..."));
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

  // Get rom size from file
  fileSize = inputFile.fileSize();
  if ((fileSize / 0x4000) > 64) {
    ui.printErrorAndAbort(F("File is too big."), false);
  }

  // Enable access to ports 0120h
  send_GBM(0x09);
  // Enable write
  send_GBM(0x0A);
  send_GBM(0x2);

  // Map entire flash rom
  send_GBM(0x4);

  // Set bank for unprotect command, writes to 0x5555 need odd bank number
  writeByte_GBM(0x2100, 0x1);

  // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
  send_GBM(0x10);
  send_GBM(0x08);

  // Unprotect sector 0
  writeByte_GBM(0x5555, 0xAA);
  writeByte_GBM(0x2AAA, 0x55);
  writeByte_GBM(0x5555, 0x60);
  writeByte_GBM(0x5555, 0xAA);
  writeByte_GBM(0x2AAA, 0x55);
  writeByte_GBM(0x5555, 0x40);

  // Check if flashrom is ready for writing or busy
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // first bank: 0x0000-0x7FFF,
  word currAddress = 0x0;

  // Write 63 banks
  for (byte currBank = 0x1; currBank < (fileSize / 0x4000); currBank++) {
    // Blink led
    PORTB ^= (1 << 4);

    // all following banks: 0x4000-0x7FFF
    if (currBank > 1) {
      currAddress = 0x4000;
    }

    // Write single bank in 128 byte steps
    for (; currAddress < 0x7FFF; currAddress += 128) {
      // Fill SD buffer
      inputFile.read(sdBuffer, 128);

      // Enable access to ports 0x120 and 0x2100
      send_GBM(0x09);
      send_GBM(0x11);

      // Set bank
      writeByte_GBM(0x2100, 0x1);

      // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
      send_GBM(0x10);
      send_GBM(0x08);

      // Write flash buffer command
      writeByte_GBM(0x5555, 0xAA);
      writeByte_GBM(0x2AAA, 0x55);
      writeByte_GBM(0x5555, 0xA0);

      // Wait until flashrom is ready again
      while ((readByte_GBM(0) & 0x80) != 0x80) {}

      // Enable access to ports 0x120 and 0x2100
      send_GBM(0x09);
      send_GBM(0x11);

      // Set bank
      writeByte_GBM(0x2100, currBank);

      // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
      send_GBM(0x10);
      send_GBM(0x08);

      // Fill flash buffer
      for (word currByte = 0; currByte < 128; currByte++) {
        writeByte_GBM(currAddress + currByte, sdBuffer[currByte]);
      }
      // Execute write
      writeByte_GBM(currAddress + 127, 0xFF);

      // Wait for write to complete
      while ((readByte_GBM(currAddress) & 0x80) != 0x80) {}
    }
  }
  // Close the file:
  inputFile.close();
  ui.printlnMsg(F("Done"));
}

void readMapping_GBM() {
  // Enable ports 0x0120
  send_GBM(0x09);

  // Set WE and WP
  send_GBM(0x0A);
  send_GBM(0x2);

  // Enable hidden mapping area
  writeByte_GBM(0x2100, 0x01);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x77);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x77);

  // Read mapping
  ui.printlnMsg(F("Reading Mapping..."));
  ui.flushOutput();

  // Get name, add extension and convert to char array for sd lib
  String outputFilePath = getNextGBMMappingOutputPath();

  // Open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);
  for (byte currByte = 0; currByte < 128; currByte++) {
    sdBuffer[currByte] = readByte_GBM(currByte);
  }
  outputFile.write(sdBuffer, 128);

  // Close the file:
  outputFile.close();

  // Signal end of process
  printSdBuffer(0, 20);
  printSdBuffer(102, 20);
  ui.printlnMsg("");
  ui.printMsg(F("Saved to "));
  ui.printlnMsg(outputFilePath);
  ui.flushOutput();

  // Reset flash to leave hidden mapping area
  resetFlash_GBM();
}

void eraseMapping_GBM() {
  ui.printlnMsg(F("Erasing..."));
  ui.flushOutput();

  //enable access to ports 0120h
  send_GBM(0x09);
  // Enable write
  send_GBM(0x0A);
  send_GBM(0x2);

  // Unprotect sector 0
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x60);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x40);

  // Wait for unprotect to complete
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Send erase command
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x60);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x04);

  // Wait for erase to complete
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Reset flashrom
  resetFlash_GBM();
}

boolean blankcheckMapping_GBM() {
  ui.printMsg(F("Blankcheck..."));
  ui.flushOutput();

  // Enable ports 0x0120
  send_GBM(0x09);

  // Set WE and WP
  send_GBM(0x0A);
  send_GBM(0x2);

  // Enable hidden mapping area
  writeByte_GBM(0x2100, 0x01);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x77);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x77);

  // Disable ports 0x0120...
  send_GBM(0x08);

  // Read rom
  for (byte currByte = 0; currByte < 128; currByte++) {
    if (readByte_GBM(currByte) != 0xFF) {
      return 0;
    }
  }
  return 1;
}

void writeMapping_GBM(const String &inputFilePath) {
  ui.printMsg(F("Writing..."));
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

  // Get map file size and check if it exceeds 128KByte
  if (inputFile.fileSize() > 0x80) {
    ui.printErrorAndAbort(F("File is too big."), false);
  }

  // Enable access to ports 0120h
  send_GBM(0x09);

  // Enable write
  send_GBM(0x0A);
  send_GBM(0x2);

  // Map entire flash rom
  send_GBM(0x4);

  // Set bank, writes to 0x5555 need odd bank number
  writeByte_GBM(0x2100, 0x1);

  // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
  send_GBM(0x10);
  send_GBM(0x08);

  // Unlock write to map area
  writeByte_GBM(0x5555, 0xAA);
  writeByte_GBM(0x2AAA, 0x55);
  writeByte_GBM(0x5555, 0x60);
  writeByte_GBM(0x5555, 0xAA);
  writeByte_GBM(0x2AAA, 0x55);
  writeByte_GBM(0x5555, 0xE0);

  // Check if flashrom is ready for writing or busy
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Fill SD buffer
  inputFile.read(sdBuffer, 128);

  // Enable access to ports 0x120 and 0x2100
  send_GBM(0x09);
  send_GBM(0x11);

  // Set bank
  writeByte_GBM(0x2100, 0x1);

  // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
  send_GBM(0x10);
  send_GBM(0x08);

  // Write flash buffer command
  writeByte_GBM(0x5555, 0xAA);
  writeByte_GBM(0x2AAA, 0x55);
  writeByte_GBM(0x5555, 0xA0);

  // Wait until flashrom is ready again
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Enable access to ports 0x120 and 0x2100
  send_GBM(0x09);
  send_GBM(0x11);

  // Set bank
  writeByte_GBM(0x2100, 0);

  // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
  send_GBM(0x10);
  send_GBM(0x08);

  // Fill flash buffer
  for (word currByte = 0; currByte < 128; currByte++) {
    // Blink led
    PORTB ^= (1 << 4);

    writeByte_GBM(currByte, sdBuffer[currByte]);
  }
  // Execute write
  writeByte_GBM(127, 0xFF);

  // Close the file:
  inputFile.close();
  ui.printlnMsg(F("Done"));
}

//******************************************
// End of File
//******************************************
