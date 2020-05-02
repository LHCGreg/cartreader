//******************************************
// FLASHROM MODULE
//******************************************

#include <Arduino.h>
#include "FLASH.h"
#include "filebrowser.h"
#include "ui.h"
#include "globals.h"
#include "utils.h"
#include "SD.h"

/******************************************
   Variables
 *****************************************/
// Flashrom
unsigned long flashSize;
byte flashromType;
byte secondID = 1;
unsigned long time;
unsigned long blank;
unsigned long sectorSize;
uint16_t bufferSize;
bool hiROM = 1;

/******************************************
   Function prototypes
 *****************************************/
String getNextFlashOutputPathAndPrintMessage();
void id_Flash16();
void setup_Flash16();
void setup_Eprom();
byte readByte_Flash(unsigned long myAddress);
void idFlash29F032();
void eraseFlash29F032();
void writeFlash29F032(const String &inputFilePath);
void busyCheck29F032(byte c);
void writeFlash29F1610(const String &inputFilePath);
void writeFlash29F1601(const String &inputFilePath);
void idFlash29F1610();
void eraseFlash29F1610();
void busyCheck29F1610();
void writeFlash29LV640(const String &inputFilePath);
void writeFlash29GL(const String &inputFilePath, unsigned long sectorSize, byte bufferSize);
void writeFlash29F800(const String &inputFilePath);
void idFlash28FXXX();
void eraseFlash28FXXX();
void writeFlash28FXXX(const String &inputFilePath);
void writeFlashE28FXXXJ3A(SafeSDFile &inputFile);
void writeFlashLH28F0XX(SafeSDFile &inputFile);
void blankcheck_Flash();
void verifyFlash(const String &inputFilePath);
void readFlash();
void printFlash(int numBytes);
void resetFlash8();
void resetFlash16();
void writeFlash16(const String &inputFilePath);
void writeFlash16_29F1601(const String &inputFilePath);
void idFlash16();
void eraseFlash16();
void blankcheck16();
void verifyFlash16(const String &inputFilePath);
void readFlash16();
void printFlash16(int numBytes);
void busyCheck16();
void writeFlash16_29LV640(const String &inputFilePath);
void blankcheck_Eprom();
void read_Eprom();
void write_Eprom(const String &inputFilePath);
void verify_Eprom(const String &inputFilePath);
void print_Eprom(int numBytes);

void flashMenu() {
  while (true) {
    const __FlashStringHelper *item_8bit = F("8bit Flash adapter");
    const __FlashStringHelper *item_Eprom = F("Eprom adapter");
    const __FlashStringHelper *item_MX = F("MX26L6420 adapter");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_8bit,
      item_Eprom,
      item_MX,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("Select adapter PCB"), menu, ARRAY_LENGTH(menu), item_8bit);

    if (answer == item_8bit) {
      ui.clearOutput();
      ui.flushOutput();
      hiROM = 1;
      setup_Flash8();
      id_Flash8();
      ui.waitForUserInput();
      mode = CartReaderMode::FLASH8;
      flashromMenu8();
    }
    else if (answer == item_Eprom) {
      ui.clearOutput();
      ui.flushOutput();
      setup_Eprom();
      mode = CartReaderMode::EPROM;
      epromMenu();
    }
    else if (answer == item_MX) {
      ui.clearOutput();
      ui.flushOutput();
      setup_Flash16();
      id_Flash16();
      ui.waitForUserInput();
      mode = CartReaderMode::FLASH16;
      flashromMenu16();
    }
    else if (answer == item_Back) {
      break;
    }
  }
}

void flashromMenu8() {
  while (true) {
    const __FlashStringHelper *item_BlankCheck = F("Blankcheck");
    const __FlashStringHelper *item_Erase = F("Erase");
    const __FlashStringHelper *item_Read = F("Read");
    const __FlashStringHelper *item_Write = F("Write");
    const __FlashStringHelper *item_ID = F("ID");
    const __FlashStringHelper *item_Print = F("Print");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_BlankCheck,
      item_Erase,
      item_Read,
      item_Write,
      item_ID,
      item_Print,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("Flashrom Writer 8"), menu, ARRAY_LENGTH(menu), item_BlankCheck);

    if (answer == item_BlankCheck) {
      ui.clearOutput();
      ui.printlnMsg(F("Blankcheck"));
      ui.flushOutput();
      time = millis();
      resetFlash8();
      blankcheck_Flash();
    }
    else if (answer == item_Erase) {
      ui.clearOutput();
      ui.printlnMsg(F("Erasing Flashrom"));
      ui.printlnMsg(F("Please wait..."));
      ui.flushOutput();
      time = millis();

      switch (flashromType) {
        case 1:
          eraseFlash29F032();
          break;
        case 2:
          eraseFlash29F1610();
          break;
        case 3:
          eraseFlash28FXXX();
          break;
      }

      ui.printlnMsg(F("Flashrom erased"));
      ui.flushOutput();
      resetFlash8();
    }
    else if (answer == item_Read) {
      time = millis();
      resetFlash8();
      readFlash();
    }
    else if (answer == item_Write) {
      String inputFilePath = fileBrowser(F("Select file"));
      ui.clearOutput();
      time = millis();

      switch (flashromType) {
        case 1: {
          writeFlash29F032(inputFilePath);
          break;
        }
        case 2: {
          if (strcmp(flashid, "C2F3") == 0)
            writeFlash29F1601(inputFilePath);
          else if ((strcmp(flashid, "C2F1") == 0) || (strcmp(flashid, "C2F9") == 0))
            writeFlash29F1610(inputFilePath);
          else if ((strcmp(flashid, "C2C4") == 0) || (strcmp(flashid, "C249") == 0) ||
                   (strcmp(flashid, "C2A7") == 0) || (strcmp(flashid, "C2A8") == 0) ||
                   (strcmp(flashid, "C2C9") == 0) || (strcmp(flashid, "C2CB") == 0))
            writeFlash29LV640(inputFilePath);
          else if (strcmp(flashid, "017E") == 0) {
            // sector size, write buffer size
            writeFlash29GL(inputFilePath, sectorSize, bufferSize);
          }
          else if ((strcmp(flashid, "0458") == 0) || (strcmp(flashid, "0158") == 0) ||
                   (strcmp(flashid, "01AB") == 0))
            writeFlash29F800(inputFilePath);

          break;
        }
        case 3: {
          writeFlash28FXXX(inputFilePath);
          break;
        }
      }

      delay(100);

      // Reset twice just to be sure
      resetFlash8();
      resetFlash8();

      verifyFlash(inputFilePath);
    }
    else if (answer == item_ID) {
      time = 0;
      ui.clearOutput();
      ui.printlnMsg(F("ID Flashrom"));
      switch (flashromType) {
        case 1:
          idFlash29F032();
          break;
        case 2:
          idFlash29F1610();
          break;
        case 3:
          idFlash28FXXX();
          break;
      }

      ui.printlnMsg("");
      printFlash(40);
      ui.printlnMsg("");
      ui.flushOutput();

      resetFlash8();
    }
    else if (answer == item_Print) {
      time = 0;
      ui.clearOutput();
      ui.printlnMsg(F("Print first 70Bytes"));
      ui.flushOutput();
      resetFlash8();
      printFlash(70);
    }
    else if (answer == item_Back) {
      time = 0;
      ui.clearOutput();
      ui.flushOutput();
      resetFlash8();
      break;
    }

    if (time != 0) {
      ui.printMsg(F("Operation took : "));
      ui.printMsg((millis() - time) / 1000, DEC);
      ui.printlnMsg(F("s"));
      ui.flushOutput();
    }
    ui.printMsg(F("Press Button..."));
    ui.flushOutput();
    ui.waitForUserInput();
  }
}

void flashromMenu16() {
  while (true) {
    const __FlashStringHelper *item_BlankCheck = F("Blankcheck");
    const __FlashStringHelper *item_Erase = F("Erase");
    const __FlashStringHelper *item_Read = F("Read");
    const __FlashStringHelper *item_Write = F("Write");
    const __FlashStringHelper *item_ID = F("ID");
    const __FlashStringHelper *item_Print = F("Print");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_BlankCheck,
      item_Erase,
      item_Read,
      item_Write,
      item_ID,
      item_Print,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("Flashrom Writer 16"), menu, ARRAY_LENGTH(menu), item_BlankCheck);

    if (answer == item_BlankCheck) {
      ui.clearOutput();
      ui.printlnMsg(F("Blankcheck"));
      ui.flushOutput();
      time = millis();
      resetFlash16();
      blankcheck16();
    }
    else if (answer == item_Erase) {
      ui.clearOutput();
      ui.printlnMsg(F("Erase Flashrom"));
      ui.flushOutput();
      time = millis();
      resetFlash16();
      eraseFlash16();
      ui.printlnMsg(F("Flashrom erased."));
      ui.flushOutput();
    }
    else if (answer == item_Read) {
      ui.clearOutput();
      time = millis();
      resetFlash16();
      readFlash16();
    }
    else if (answer == item_Write) {
      String inputFilePath = fileBrowser(F("Select file"));
      ui.clearOutput();
      time = millis();
      if (strcmp(flashid, "C2F3") == 0) {
        writeFlash16_29F1601(inputFilePath);
      }
      else if ((strcmp(flashid, "C2C4") == 0) || (strcmp(flashid, "C249") == 0) || (strcmp(flashid, "C2A7") == 0) || (strcmp(flashid, "C2A8") == 0) || (strcmp(flashid, "C2C9") == 0) || (strcmp(flashid, "C2CB") == 0) || (strcmp(flashid, "C2FC") == 0)) {
        writeFlash16_29LV640(inputFilePath);
      }
      else {
        writeFlash16(inputFilePath);
      }
      delay(100);
      resetFlash16();
      delay(100);
      verifyFlash16(inputFilePath);
    }
    else if (answer == item_ID) {
      time = 0;
      ui.clearOutput();
      ui.printlnMsg(F("ID Flashrom"));
      idFlash16();
      ui.printlnMsg("");
      printFlash16(40);
      ui.printlnMsg("");
      ui.flushOutput();
      resetFlash16();
    }
    else if (answer == item_Print) {
      time = 0;
      ui.clearOutput();
      ui.printlnMsg(F("Print first 70Bytes"));
      ui.flushOutput();
      resetFlash16();
      printFlash16(70);
    }
    else if (answer == item_Back) {
      time = 0;
      ui.clearOutput();
      ui.flushOutput();
      resetFlash16();
      break;
    }

    if (time != 0) {
      ui.printMsg(F("Operation took: "));
      ui.printMsg((millis() - time) / 1000, DEC);
      ui.printlnMsg("s");
      ui.flushOutput();
    }
    ui.waitForUserInput();
  }
}

void epromMenu() {
  while (true) {
    const __FlashStringHelper *item_BlankCheck = F("Blankcheck");
    const __FlashStringHelper *item_Read = F("Read");
    const __FlashStringHelper *item_Write = F("Write");
    const __FlashStringHelper *item_Verify = F("Verify");
    const __FlashStringHelper *item_Print = F("Print");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_BlankCheck,
      item_Read,
      item_Write,
      item_Verify,
      item_Print,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("Eprom Writer"), menu, ARRAY_LENGTH(menu), item_BlankCheck);

    if (answer == item_BlankCheck) {
      ui.clearOutput();
      ui.printlnMsg(F("Blankcheck"));
      ui.flushOutput();
      time = millis();
      blankcheck_Eprom();
    }
    else if (answer == item_Read) {
      ui.clearOutput();
      time = millis();
      read_Eprom();
    }
    else if (answer == item_Write) {
      String inputFilePath = fileBrowser(F("Select file"));
      ui.clearOutput();
      time = millis();
      write_Eprom(inputFilePath);
      delay(1000);
      verify_Eprom(inputFilePath);
    }
    else if (answer == item_Verify) {
      String inputFilePath = fileBrowser(F("Verify against"));
      ui.clearOutput();
      time = millis();
      verify_Eprom(inputFilePath);
    }
    else if (answer == item_Print) {
      ui.clearOutput();
      time = millis();
      print_Eprom(80);
    }
    else if (answer == item_Back) {
      time = 0;
      ui.clearOutput();
      ui.flushOutput();
      break;
    }

    if (time != 0) {
      ui.printMsg(F("Operation took: "));
      ui.printMsg((millis() - time) / 1000, DEC);
      ui.printlnMsg("s");
      ui.flushOutput();
    }
    ui.waitForUserInput();
  }
}

String getNextFlashOutputPathAndPrintMessage() {
  return getNextOutputPathWithNumberedFilename(F("FLASH"), F("FL"), F(".bin"));
}

/******************************************
   Flash IDs
 *****************************************/
void id_Flash8() {
  // ID flash
  idFlash29F032();

  // Print start screen
idtheflash:
  ui.clearOutput();
  ui.flushOutput();
  ui.printlnMsg(F("Flashrom Writer 8bit"));
  ui.printlnMsg(" ");
  ui.printlnMsg(" ");
  ui.printMsg(F("Flash ID: "));
  ui.printlnMsg(flashid);

  if (strcmp(flashid, "C2F1") == 0) {
    ui.printlnMsg(F("MX29F1610 detected"));
    flashSize = 2097152;
    flashromType = 2;
  }
  else if (strcmp(flashid, "C2F3") == 0) {
    ui.printlnMsg(F("MX29F1601 detected"));
    flashSize = 2097152;
    flashromType = 2;
  }
  else if (strcmp(flashid, "C2F9") == 0) {
    ui.printlnMsg(F("MX29L3211 detected"));
    ui.printlnMsg(F("ATTENTION 3.3V"));
    flashSize = 4194304;
    flashromType = 2;
  }
  else if ((strcmp(flashid, "C2C4") == 0) || (strcmp(flashid, "C249") == 0)) {
    ui.printlnMsg(F("MX29LV160 detected"));
    ui.printlnMsg(F("ATTENTION 3.3V"));
    flashSize = 2097152;
    flashromType = 2;
  }
  else if ((strcmp(flashid, "C2A7") == 0) || (strcmp(flashid, "C2A8") == 0)) {
    ui.printlnMsg(F("MX29LV320 detected"));
    ui.printlnMsg(F("ATTENTION 3.3V"));
    flashSize = 4194304;
    flashromType = 2;
  }
  else if ((strcmp(flashid, "C2C9") == 0) || (strcmp(flashid, "C2CB") == 0)) {
    ui.printlnMsg(F("MX29LV640 detected"));
    ui.printlnMsg(F("ATTENTION 3.3V"));
    flashSize = 8388608;
    flashromType = 2;
  }
  else if (strcmp(flashid, "0141") == 0) {
    ui.printlnMsg(F("AM29F032B detected"));
    flashSize = 4194304;
    flashromType = 1;
  }
  else if (strcmp(flashid, "01AD") == 0) {
    ui.printlnMsg(F("AM29F016B detected"));
    flashSize = 2097152;
    flashromType = 1;
  }
  else if (strcmp(flashid, "20AD") == 0) {
    ui.printlnMsg(F("AM29F016D detected"));
    flashSize = 2097152;
    flashromType = 1;
  }
  else if (strcmp(flashid, "04AD") == 0) {
    ui.printlnMsg(F("AM29F016D detected"));
    flashSize = 2097152;
    flashromType = 1;
  }
  else if (strcmp(flashid, "04D4") == 0) {
    ui.printlnMsg(F("MBM29F033C detected"));
    flashSize = 4194304;
    flashromType = 1;
  }
  else if (strcmp(flashid, "0458") == 0) {
    ui.printlnMsg(F("MBM29F800BA detected"));
    flashSize = 1048576;
    flashromType = 2;
  }
  else if (strcmp(flashid, "01AB") == 0) {
    ui.printlnMsg(F("AM29F400AB detected"));
    flashSize = 131072 * 4;
    flashromType = 2;
  }
  else if (strcmp(flashid, "0158") == 0) {
    ui.printlnMsg(F("AM29F800BB detected"));
    flashSize = 1048576;
    flashromType = 2;
  }
  else if (strcmp(flashid, "01A3") == 0) {
    ui.printlnMsg(F("AM29LV033C detected"));
    flashSize = 131072 * 32;
    flashromType = 1;
  }
  else if (strcmp(flashid, "017E") == 0) {
    // S29GL032M
    if (readByte_Flash(28) == 0x1A) {
      ui.printlnMsg(F("S29GL032M detected"));
      flashSize = 4194304;
      sectorSize = 65536;
      bufferSize = 32;
    }
    // Unknown S29GL type
    else {
      ui.printlnMsg(F("Unknown S29GL Type"));
      flashSize = 4194304;
      sectorSize = 65536;
      bufferSize = 32;
    }
    ui.printlnMsg(F("ATTENTION 3.3V"));
    flashromType = 2;
  }
  else if (strcmp(flashid, "B088") == 0) {
    // LH28F016SUT
    ui.printlnMsg(F("LH28F016SUT detected"));
    ui.printlnMsg(F("ATTENTION 3/5 setting"));
    flashSize = 2097152;
    sectorSize = 65536;
    bufferSize = 256;
    flashromType = 3;
  }
  else if ((strcmp(flashid, "8916") == 0) ||
           (strcmp(flashid, "8917") == 0) ||
           (strcmp(flashid, "8918") == 0)) {
    // E28FXXXJ3A
    ui.printMsg(F("E28F"));

    switch (flashid[3]) {
      case '6':
        flashSize = 131072 * 32;
        ui.printMsg(F("320"));
        break;
      case '7':
        flashSize = 131072 * 64;
        ui.printMsg(F("640"));
        break;
      case '8':
        flashSize = 131072 * 128;
        ui.printMsg(F("128"));
        break;
    }

    ui.printlnMsg(F("J3A detected"));
    sectorSize = 131072;
    bufferSize = 32;
    flashromType = 3;
  }
  else if (secondID == 1) {
    // Backup first ID read-out
    strncpy(vendorID, flashid, 5);
    // Read ID a second time using a different command
    idFlash29F1610();
    secondID = 2;
    goto idtheflash;
  }
  else if (secondID == 2) {
    // test if 28FXXX series flash
    idFlash28FXXX();
    secondID = 0;
    goto idtheflash;
  }
  else {
    // ID not found
    ui.clearOutput();
    ui.printlnMsg(F("Flashrom Writer 8bit"));
    ui.printlnMsg(" ");
    ui.printMsg(F("ID Type 1: "));
    ui.printlnMsg(vendorID);
    ui.printMsg(F("ID Type 2: "));
    ui.printlnMsg(flashid);
    ui.printlnMsg(" ");
    ui.printErrorAndAbort(F("UNKNOWN FLASHROM"), false);
  }
  ui.printlnMsg(" ");
  ui.printlnMsg(F("Press Button..."));
  ui.flushOutput();

  resetFlash8();
}

void id_Flash16() {
  // ID flash
  idFlash16();
  resetFlash16();

  ui.printlnMsg(F("Flashrom Writer 16bit"));
  ui.printlnMsg(" ");
  ui.printMsg(F("Flash ID: "));
  ui.printlnMsg(flashid);
  if (strcmp(flashid, "C2F1") == 0) {
    ui.printlnMsg(F("MX29F1610 detected"));
    ui.printlnMsg(" ");
    flashSize = 2097152;
    flashromType = 2;
  }
  else if (strcmp(flashid, "C2F3") == 0) {
    ui.printlnMsg(F("MX29F1601 detected"));
    flashSize = 2097152;
    flashromType = 2;
  }
  else if (strcmp(flashid, "C2F9") == 0) {
    ui.printlnMsg(F("MX29L3211 detected"));
    ui.printlnMsg(F("ATTENTION 3.3V"));
    flashSize = 4194304;
    flashromType = 2;
  }
  else if ((strcmp(flashid, "C2C4") == 0) || (strcmp(flashid, "C249") == 0)) {
    ui.printlnMsg(F("MX29LV160 detected"));
    ui.printlnMsg(F("ATTENTION 3.3V"));
    flashSize = 2097152;
    flashromType = 2;
  }
  else if ((strcmp(flashid, "C2A7") == 0) || (strcmp(flashid, "C2A8") == 0)) {
    ui.printlnMsg(F("MX29LV320 detected"));
    ui.printlnMsg(F("ATTENTION 3.3V"));
    flashSize = 4194304;
    flashromType = 2;
  }
  else if ((strcmp(flashid, "C2C9") == 0) || (strcmp(flashid, "C2CB") == 0)) {
    ui.printlnMsg(F("MX29LV640 detected"));
    ui.printlnMsg(F("ATTENTION 3.3V"));
    flashSize = 8388608;
    flashromType = 2;
  }
  else if (strcmp(flashid, "C2FC") == 0) {
    ui.printlnMsg(F("MX26L6420 detected"));
    ui.printlnMsg(F("ATTENTION 3.3V"));
    flashSize = 8388608;
    flashromType = 2;
  }
  else {
    ui.printErrorAndAbort(F("Unknown flashrom"), false);
  }
  ui.printlnMsg(" ");
  ui.printlnMsg(F("Press Button..."));
  ui.flushOutput();
}

/******************************************
   Setup
 *****************************************/
void setup_Flash8() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output RST(PH0) OE(PH1) OE_SNS(PH3) WE(PH4) WE_SNS(PH5) CE(PH6)
  DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
  // Setting RST(PH0) OE(PH1) OE_SNS(PH3) WE(PH4) WE_SNS(PH5) HIGH
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5);
  // Setting CE(PH6) LOW
  PORTH &= ~(1 << 6);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Disable Internal Pullups
  PORTC = 0x00;
}

void setup_Flash16() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output RST(PH0) OE(PH1) BYTE(PH3) WE(PH4) WP(PH5) CE(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Data Pins (D0-D15) to Input
  DDRC = 0x00;
  DDRA = 0x00;
  // Disable Internal Pullups
  PORTC = 0x00;
  PORTA = 0x00;

  // Setting RST(PH0) OE(PH1) BYTE(PH3) WE(PH4) WP(PH5) HIGH
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5);
  // Setting CE(PH6) LOW
  PORTH &= ~(1 << 6);

  delay(100);
}

void setup_Eprom() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Data Pins (D0-D15) to Input
  DDRC = 0x00;
  DDRA = 0x00;
  // Disable Internal Pullups
  PORTC = 0x00;
  PORTA = 0x00;

  // Set Control Pins to Output VPP/OE(PH5) CE(PH6)
  DDRH |= (1 << 5) | (1 << 6);

  // Setting CE(PH6) HIGH
  PORTH |= (1 << 6);
  // Setting VPP/OE(PH5) LOW
  PORTH &= ~(1 << 5);

  // 27C322 is a 4MB eprom
  flashSize = 4194304;
}

/******************************************
   I/O Functions
 *****************************************/
// Switch data pins to read
void dataIn8() {
  // Set to Input
  DDRC = 0x00;
}

// Switch data pins to write
void dataOut16() {
  DDRC = 0xFF;
  DDRA = 0xFF;
}

// Switch data pins to read
void dataIn16() {
  DDRC = 0x00;
  DDRA = 0x00;
}

/******************************************
   Low level functions
 *****************************************/
void writeByte_Flash(unsigned long myAddress, byte myData) {
  PORTF = myAddress & 0xFF;
  if (hiROM) {
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;
  }
  else {
    PORTK = (myAddress >> 8) & 0x7F;
    // Set A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    PORTL = (myAddress >> 15) & 0xFF;
  }
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch WE(PH4) WE_SNS(PH5) to LOW
  PORTH &= ~((1 << 4) | (1 << 5));

  // Leave WE low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch WE(PH4) WE_SNS(PH5) to HIGH
  PORTH |= (1 << 4) | (1 << 5);

  // Leave WE high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}

byte readByte_Flash(unsigned long myAddress) {
  PORTF = myAddress & 0xFF;
  if (hiROM) {
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;
  }
  else {
    PORTK = (myAddress >> 8) & 0x7F;
    // Set A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    PORTL = (myAddress >> 15) & 0xFF;
  }

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Setting OE(PH1) OE_SNS(PH3) LOW
  PORTH &= ~((1 << 1) | (1 << 3));

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  byte tempByte = PINC;

  // Setting OE(PH1) OE_SNS(PH3) HIGH
  PORTH |= (1 << 1) | (1 << 3);
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempByte;
}

void writeWord_Flash(unsigned long myAddress, word myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;
  PORTC = myData;
  PORTA = (myData >> 8) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t");

  // Switch WE(PH4) to LOW
  PORTH &= ~(1 << 4);

  // Leave WE low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch WE(PH4) to HIGH
  PORTH |= (1 << 4);

  // Leave WE high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}

word readWord_Flash(unsigned long myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t");

  // Setting OE(PH1) LOW
  PORTH &= ~(1 << 1);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  word tempWord = ( ( PINA & 0xFF ) << 8 ) | ( PINC & 0xFF );

  __asm__("nop\n\t");

  // Setting OE(PH1) HIGH
  PORTH |= (1 << 1);
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempWord;
}

/******************************************
  29F032 flashrom functions
*****************************************/
void resetFlash29F032() {
  // Set data pins to output
  dataOut();

  // Reset command sequence
  writeByte_Flash(0x555, 0xf0);

  // Set data pins to input again
  dataIn8();

  delay(500);
}

void idFlash29F032() {
  // Set data pins to output
  dataOut();

  // ID command sequence
  writeByte_Flash(0x555, 0xaa);
  writeByte_Flash(0x2aa, 0x55);
  writeByte_Flash(0x555, 0x90);

  // Set data pins to input again
  dataIn8();

  // Read the two id bytes into a string
  sprintf(flashid, "%02X%02X", readByte_Flash(0), readByte_Flash(1));
}

void eraseFlash29F032() {
  // Set data pins to output
  dataOut();

  // Erase command sequence
  writeByte_Flash(0x555, 0xaa);
  writeByte_Flash(0x2aa, 0x55);
  writeByte_Flash(0x555, 0x80);
  writeByte_Flash(0x555, 0xaa);
  writeByte_Flash(0x2aa, 0x55);
  writeByte_Flash(0x555, 0x10);

  // Set data pins to input again
  dataIn8();

  // Read the status register
  byte statusReg = readByte_Flash(0);

  // After a completed erase D7 will output 1
  while ((statusReg & 0x80) != 0x80) {
    // Blink led
    PORTB ^= (1 << 4);
    delay(100);
    // Update Status
    statusReg = readByte_Flash(0);
  }
}

void writeFlash29F032(const String &inputFilePath) {
  ui.printMsg(F("Flashing file "));
  ui.printMsg(inputFilePath);
  ui.printlnMsg(F("..."));
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  // Set data pins to output
  dataOut();

  // Fill sdBuffer
  for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
    inputFile.read(sdBuffer, 512);
    // Blink led
    if (currByte % 2048 == 0)
      PORTB ^= (1 << 4);

    for (int c = 0; c < 512; c++) {
      // Write command sequence
      writeByte_Flash(0x555, 0xaa);
      writeByte_Flash(0x2aa, 0x55);
      writeByte_Flash(0x555, 0xa0);
      // Write current byte
      writeByte_Flash(currByte + c, sdBuffer[c]);
      busyCheck29F032(sdBuffer[c]);
    }
  }
  // Set data pins to input again
  dataIn8();

  // Close the file:
  inputFile.close();
}

void busyCheck29F032(byte c) {
  // Set data pins to input
  dataIn8();

  // Setting OE(PH1) OE_SNS(PH3) CE(PH6)LOW
  PORTH &= ~((1 << 1) | (1 << 3) | (1 << 6));
  // Setting WE(PH4) WE_SNES(PH5) HIGH
  PORTH |=  (1 << 4) | (1 << 5);

  //When the Embedded Program algorithm is complete, the device outputs the datum programmed to D7
  while ((PINC & 0x80) != (c & 0x80)) {}

  // Set data pins to output
  dataOut();

  // Setting OE(PH1) OE_SNS(PH3) HIGH
  PORTH |= (1 << 1) | (1 << 3);
}
/******************************************
  29F1610 flashrom functions
*****************************************/

void resetFlash29F1610() {
  // Set data pins to output
  dataOut();

  // Reset command sequence
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0xf0);

  // Set data pins to input again
  dataIn8();

  delay(500);
}

void writeFlash29F1610(const String &inputFilePath) {
  ui.printlnMsg(F("Flashing file "));
  ui.printlnMsg(inputFilePath);
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  // Set data pins to output
  dataOut();

  for (unsigned long currByte = 0; currByte < fileSize; currByte += 128) {
    // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
    inputFile.read(sdBuffer, 128);

    // Blink led
    if (currByte % 3072 == 0)
      PORTB ^= (1 << 4);

    // Check if write is complete
    delayMicroseconds(100);
    busyCheck29F1610();

    // Write command sequence
    writeByte_Flash(0x5555 << 1, 0xaa);
    writeByte_Flash(0x2aaa << 1, 0x55);
    writeByte_Flash(0x5555 << 1, 0xa0);

    // Write one full page at a time
    for (byte c = 0; c < 128; c++) {
      writeByte_Flash(currByte + c, sdBuffer[c]);
    }
  }

  // Check if write is complete
  busyCheck29F1610();

  // Set data pins to input again
  dataIn8();

  // Close the file:
  inputFile.close();
}

void writeFlash29F1601(const String &inputFilePath) {
  ui.printlnMsg(F("Flashing file "));
  ui.printlnMsg(inputFilePath);
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  // Set data pins to output
  dataOut();

  for (unsigned long currByte = 0; currByte < fileSize; currByte += 128) {
    // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
    inputFile.read(sdBuffer, 128);

    // Blink led
    if (currByte % 3072 == 0)
      PORTB ^= (1 << 4);

    // Check if write is complete
    delayMicroseconds(100);
    busyCheck29F1610();

    // Write command sequence
    writeByte_Flash(0x5555 << 1, 0xaa);
    writeByte_Flash(0x2aaa << 1, 0x55);
    writeByte_Flash(0x5555 << 1, 0xa0);

    // Write one full page at a time
    for (byte c = 0; c < 128; c++) {
      writeByte_Flash(currByte + c, sdBuffer[c]);

      if (c == 127) {
        // Write the last byte twice or else it won't write at all
        writeByte_Flash(currByte + c, sdBuffer[c]);
      }
    }
  }

  // Check if write is complete
  busyCheck29F1610();

  // Set data pins to input again
  dataIn8();

  // Close the file:
  inputFile.close();
}

void idFlash29F1610() {
  // Set data pins to output
  dataOut();

  // ID command sequence
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0x90);

  // Set data pins to input again
  dataIn8();

  // Read the two id bytes into a string
  sprintf(flashid, "%02X%02X", readByte_Flash(0), readByte_Flash(2));
}

byte readStatusReg() {
  // Set data pins to output
  dataOut();

  // Status reg command sequence
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0x70);

  // Set data pins to input again
  dataIn8();

  // Read the status register
  byte statusReg = readByte_Flash(0);
  return statusReg;
}

void eraseFlash29F1610() {
  // Set data pins to output
  dataOut();

  // Erase command sequence
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0x80);
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0x10);

  // Set data pins to input again
  dataIn8();

  busyCheck29F1610();
}

// Delay between write operations based on status register
void busyCheck29F1610() {
  // Set data pins to input
  dataIn8();

  // Read the status register
  byte statusReg = readByte_Flash(0);

  while ((statusReg & 0x80) != 0x80) {
    statusReg = readByte_Flash(0);
  }

  // Set data pins to output
  dataOut();
}

/******************************************
  MX29LV flashrom functions
*****************************************/
void busyCheck29LV640(unsigned long myAddress, byte myData) {
  // Set data pins to input
  dataIn8();

  // Read the status register
  byte statusReg = readByte_Flash(myAddress);
  while ((statusReg & 0x80) != (myData & 0x80)) {
    statusReg = readByte_Flash(myAddress);
  }

  // Set data pins to output
  dataOut();
}

void writeFlash29LV640(const String &inputFilePath) {
  ui.printlnMsg(F("Flashing file "));
  ui.printlnMsg(inputFilePath);
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  // Set data pins to output
  dataOut();

  for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
    // Fill sdBuffer
    inputFile.read(sdBuffer, 512);
    // Blink led
    if (currByte % 4096 == 0)
      PORTB ^= (1 << 4);
    for (int c = 0; c < 512; c++) {
      // Write command sequence
      writeByte_Flash(0x555 << 1, 0xaa);
      writeByte_Flash(0x2aa << 1, 0x55);
      writeByte_Flash(0x555 << 1, 0xa0);
      // Write current byte
      writeByte_Flash(currByte + c, sdBuffer[c]);
      // Check if write is complete
      busyCheck29LV640(currByte + c, sdBuffer[c]);
    }
  }
  // Set data pins to input again
  dataIn8();
  // Close the file:
  inputFile.close();
}

/******************************************
  S29GL flashrom functions
*****************************************/
void writeFlash29GL(const String &inputFilePath, unsigned long sectorSize, byte bufferSize) {
  ui.printlnMsg(F("Flashing file "));
  ui.printlnMsg(inputFilePath);
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  // Set data pins to output
  dataOut();

  for (unsigned long currSector = 0; currSector < fileSize; currSector += sectorSize) {
    // Blink led
    PORTB ^= (1 << 4);

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < sectorSize; currSdBuffer += 512) {
      // Fill SD buffer
      inputFile.read(sdBuffer, 512);

      // Write bufferSize bytes at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += bufferSize) {
        // 2 unlock commands
        writeByte_Flash(0x555 << 1, 0xaa);
        writeByte_Flash(0x2aa << 1, 0x55);
        // Write buffer load command at sector address
        writeByte_Flash(currSector + currSdBuffer + currWriteBuffer, 0x25);
        // Write byte count (minus 1) at sector address
        writeByte_Flash(currSector + currSdBuffer + currWriteBuffer, bufferSize - 1);

        // Load bytes into buffer
        for (byte currByte = 0; currByte < bufferSize; currByte++) {
          writeByte_Flash(currSector + currSdBuffer + currWriteBuffer + currByte, sdBuffer[currWriteBuffer + currByte]);
        }

        // Write Buffer to Flash
        writeByte_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1, 0x29);

        // Read the status register at last written address
        dataIn8();
        byte statusReg = readByte_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1);
        while ((statusReg & 0x80) != (sdBuffer[currWriteBuffer + bufferSize - 1] & 0x80)) {
          statusReg = readByte_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1);
        }
        dataOut();
      }
    }
  }
  // Set data pins to input again
  dataIn8();
  // Close the file:
  inputFile.close();
}

/******************************************
  29F800 functions
*****************************************/
void writeFlash29F800(const String &inputFilePath) {
  ui.printlnMsg(F("Flashing file "));
  ui.printlnMsg(inputFilePath);
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  // Set data pins to output
  dataOut();

  // Fill sdBuffer
  for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
    inputFile.read(sdBuffer, 512);
    // Blink led
    if (currByte % 2048 == 0)
      PORTB ^= (1 << 4);

    for (int c = 0; c < 512; c++) {
      // Write command sequence
      writeByte_Flash(0x5555 << 1, 0xaa);
      writeByte_Flash(0x2aaa << 1, 0x55);
      writeByte_Flash(0x5555 << 1, 0xa0);
      // Write current byte
      writeByte_Flash(currByte + c, sdBuffer[c]);
      busyCheck29F032(sdBuffer[c]);
    }
  }

  // Set data pins to input again
  dataIn8();

  // Close the file:
  inputFile.close();
}

/******************************************
  28FXXX series flashrom functions
*****************************************/
void idFlash28FXXX() {
  dataOut();
  writeByte_Flash(0x0, 0x90);

  dataIn8();

  // Read the two id bytes into a string
  sprintf(flashid, "%02X%02X", readByte_Flash(0), readByte_Flash(1));
}

void resetFlash28FXXX() {
  dataOut();
  writeByte_Flash(0x0, 0xff);

  dataIn();
  delay(500);
}

uint8_t statusFlash28FXXX() {
  dataOut();

  writeByte_Flash(0x0, 0x70);
  dataIn8();
  return readByte_Flash(0x0);
}

void eraseFlash28FXXX() {
  // only can erase block by block
  for (uint32_t ba = 0; ba < flashSize; ba += sectorSize)
  {
    dataOut();
    writeByte_Flash(ba, 0x20);
    writeByte_Flash(ba, 0xd0);

    dataIn8();
    while ((readByte_Flash(ba) & 0x80) == 0x00);

    // blink LED
    PORTB ^= (1 << 4);
  }
}

void writeFlash28FXXX(const String &inputFilePath) {
  ui.printMsg(F("Flashing file "));
  ui.printlnMsg(inputFilePath);
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

  if ((strcmp(flashid, "B088") == 0))
    writeFlashLH28F0XX(inputFile);
  else if ((strcmp(flashid, "8916") == 0) ||
            (strcmp(flashid, "8917") == 0) ||
            (strcmp(flashid, "8918") == 0)) {
    writeFlashE28FXXXJ3A(inputFile);
  }

  inputFile.close();
}

void writeFlashE28FXXXJ3A(SafeSDFile &inputFile) {
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printError(F("File size exceeds flash size."));
    return;
  }

  uint32_t block_addr;
  uint32_t block_addr_mask = ~(sectorSize - 1);

  // Fill sdBuffer
  for (uint32_t currByte = 0; currByte < fileSize; currByte += 512) {
    inputFile.read(sdBuffer, 512);

    // Blink led
    if (currByte % 2048 == 0)
      PORTB ^= (1 << 4);

    block_addr = currByte & block_addr_mask;

    for (uint32_t c = 0; c < 512; c += bufferSize) {
      // write to buffer start
      dataOut();
      writeByte_Flash(block_addr, 0xe8);

      // waiting for buffer available
      dataIn8();
      while ((readByte_Flash(block_addr) & 0x80) == 0x00);
      dataOut();

      // set write byte count
      writeByte_Flash(block_addr, bufferSize - 1);

      // filling buffer
      for (uint32_t d = 0; d < bufferSize; d++)
        writeByte_Flash(currByte + c + d, sdBuffer[c + d]);

      // start flashing page
      writeByte_Flash(block_addr, 0xd0);

      // waiting for finishing
      dataIn8();
      while ((readByte_Flash(block_addr) & 0x80) == 0x00);
    }
  }

  dataIn8();
}

void writeFlashLH28F0XX(SafeSDFile &inputFile) {
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printError(F("File size exceeds flash size."));
    return;
  }

  // Fill sdBuffer
  for (uint32_t currByte = 0; currByte < fileSize; currByte += 512) {
    inputFile.read(sdBuffer, 512);
    // Blink led
    if (currByte % 2048 == 0)
      PORTB ^= (1 << 4);

    for (uint32_t c = 0; c < 512; c += bufferSize) {
      // sequence load to page
      dataOut();
      writeByte_Flash(0x0, 0xe0);
      writeByte_Flash(0x0, bufferSize - 1); // BCL
      writeByte_Flash(0x0, 0x00); // BCH should be 0x00

      for (uint32_t d = 0; d < bufferSize; d++)
        writeByte_Flash(d, sdBuffer[c + d]);

      // start flashing page
      writeByte_Flash(0x0, 0x0c);
      writeByte_Flash(0x0, bufferSize - 1); // BCL
      writeByte_Flash(currByte + c, 0x00); // BCH should be 0x00

      // waiting for finishing
      dataIn8();
      while ((readByte_Flash(currByte + c) & 0x80) == 0x00);
    }
  }

  dataIn8();
}

/******************************************
  Common flashrom functions
*****************************************/
void blankcheck_Flash() {
  ui.printlnMsg(F("Please wait..."));
  ui.flushOutput();

  blank = 1;
  for (unsigned long currByte = 0; currByte < flashSize; currByte++) {
    // Check if all bytes are 0xFF
    if (readByte_Flash(currByte) != 0xFF) {
      currByte = flashSize;
      blank = 0;
    }
  }
  if (blank) {
    ui.printlnMsg(F("Flashrom is empty"));
    ui.flushOutput();
  }
  else {
    ui.printError(F("Error: Not blank"));
  }
}

void verifyFlash(const String &inputFilePath) {
  ui.printlnMsg(F("Verifying..."));
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  blank = 0;
  for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
    //fill sdBuffer
    inputFile.read(sdBuffer, 512);
    for (int c = 0; c < 512; c++) {
      if (readByte_Flash(currByte + c) != sdBuffer[c]) {
        blank++;
      }
    }
  }
  if (blank == 0) {
    ui.printlnMsg(F("Flashrom verified OK"));
    ui.flushOutput();
  }
  else {
    ui.printMsg(F("Error: "));
    ui.printMsg(blank);
    ui.printlnMsg(F(" bytes "));
    ui.printError(F("did not verify."));
  }
  // Close the file:
  inputFile.close();
}

void readFlash() {
  String outputFilePath = getNextFlashOutputPathAndPrintMessage();

  // Open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);
  for (unsigned long currByte = 0; currByte < flashSize; currByte += 512) {
    for (int c = 0; c < 512; c++) {
      sdBuffer[c] = readByte_Flash(currByte + c);
    }
    outputFile.write(sdBuffer, 512);
  }

  // Close the file:
  outputFile.close();
  ui.printlnMsg(F("Finished reading"));
  ui.flushOutput();
}

void printFlash(int numBytes) {
  char myBuffer[3];

  for (int currByte = 0; currByte < numBytes; currByte += 10) {
    for (int c = 0; c < 10; c++) {
      itoa (readByte_Flash(currByte + c), myBuffer, 16);
      for (size_t i = 0; i < 2 - strlen(myBuffer); i++) {
        ui.printMsg("0");
      }
      // Now print the significant bits
      ui.printMsg(myBuffer);
    }
    ui.printlnMsg("");
  }
  ui.flushOutput();
}

void resetFlash8() {
  switch (flashromType) {
    case 1: resetFlash29F032(); break;
    case 2: resetFlash29F1610(); break;
    case 3: resetFlash28FXXX(); break;
  }
}

/******************************************
  29L3211 16bit flashrom functions
*****************************************/
void resetFlash16() {
  // Set data pins to output
  dataOut16();

  // Reset command sequence
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0xf0);

  // Set data pins to input again
  dataIn16();

  delay(500);
}

void writeFlash16(const String &inputFilePath) {
  ui.printlnMsg(F("Flashing file "));
  ui.printlnMsg(inputFilePath);
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  // Set data pins to output
  dataOut16();

  // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
  int d = 0;
  for (unsigned long currByte = 0; currByte < fileSize / 2; currByte += 64) {
    inputFile.read(sdBuffer, 128);

    // Blink led
    if (currByte % 2048 == 0)
      PORTB ^= (1 << 4);

    // Check if write is complete
    delayMicroseconds(100);
    busyCheck16();

    // Write command sequence
    writeWord_Flash(0x5555, 0xaa);
    writeWord_Flash(0x2aaa, 0x55);
    writeWord_Flash(0x5555, 0xa0);

    // Write one full page at a time
    for (byte c = 0; c < 64; c++) {
      word currWord = ( ( sdBuffer[d + 1] & 0xFF ) << 8 ) | ( sdBuffer[d] & 0xFF );
      writeWord_Flash(currByte + c, currWord);
      d += 2;
    }
    d = 0;
  }

  // Check if write is complete
  busyCheck16();

  // Set data pins to input again
  dataIn16();

  // Close the file:
  inputFile.close();
}

void writeFlash16_29F1601(const String &inputFilePath) {
  ui.printlnMsg(F("Flashing file "));
  ui.printlnMsg(inputFilePath);
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  // Set data pins to output
  dataOut16();

  // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
  int d = 0;
  for (unsigned long currByte = 0; currByte < fileSize / 2; currByte += 64) {
    inputFile.read(sdBuffer, 128);

    // Blink led
    if (currByte % 2048 == 0)
      PORTB ^= (1 << 4);

    // Check if write is complete
    delayMicroseconds(100);
    busyCheck16();

    // Write command sequence
    writeWord_Flash(0x5555, 0xaa);
    writeWord_Flash(0x2aaa, 0x55);
    writeWord_Flash(0x5555, 0xa0);

    // Write one full page at a time
    for (byte c = 0; c < 64; c++) {
      word currWord = ( ( sdBuffer[d + 1] & 0xFF ) << 8 ) | ( sdBuffer[d] & 0xFF );
      writeWord_Flash(currByte + c, currWord);

      if (c == 63) {
        // Write the last byte twice or else it won't write at all
        writeWord_Flash(currByte + c, sdBuffer[d + 1]);
      }
      d += 2;
    }
    d = 0;
  }

  // Check if write is complete
  busyCheck16();

  // Set data pins to input again
  dataIn16();

  // Close the file:
  inputFile.close();
}

void idFlash16() {
  // Set data pins to output
  dataOut16();

  // ID command sequence
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0x90);

  // Set data pins to input again
  dataIn16();

  // Read the two id bytes into a string
  sprintf(flashid, "%02X%02X", readWord_Flash(0) & 0xFF, readWord_Flash(1) & 0xFF);
}

byte readStatusReg16() {
  // Set data pins to output
  dataOut16();

  // Status reg command sequence
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0x70);

  // Set data pins to input again
  dataIn16();

  // Read the status register
  byte statusReg = readWord_Flash(0);
  return statusReg;
}

void eraseFlash16() {
  // Set data pins to output
  dataOut16();

  // Erase command sequence
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0x80);
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0x10);

  // Set data pins to input again
  dataIn16();

  busyCheck16();
}

void blankcheck16() {

  ui.printlnMsg(F("Please wait..."));
  ui.flushOutput();

  blank = 1;
  for (unsigned long currByte = 0; currByte < flashSize / 2; currByte++) {
    if (readWord_Flash(currByte) != 0xFFFF) {
      currByte = flashSize / 2;
      blank = 0;
    }
  }
  if (blank) {
    ui.printlnMsg(F("Flashrom is empty."));
    ui.flushOutput();
  }
  else {
    ui.printError(F("Error: Not blank"));
  }
}

void verifyFlash16(const String &inputFilePath) {
  ui.printlnMsg(F("Verifying..."));
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  blank = 0;
  word d = 0;
  for (unsigned long currByte = 0; currByte < fileSize / 2; currByte += 256) {
    //fill sdBuffer
    inputFile.read(sdBuffer, 512);
    for (int c = 0; c < 256; c++) {
      word currWord = ((sdBuffer[d + 1] << 8) | sdBuffer[d]);

      if (readWord_Flash(currByte + c) != currWord) {
        blank++;
      }
      d += 2;
    }
    d = 0;
  }
  if (blank == 0) {
    ui.printlnMsg(F("Flashrom verified OK"));
    ui.flushOutput();
  }
  else {
    ui.printlnMsg(F("Verification ERROR!"));
    ui.printMsg(blank);
    ui.printError(F("B did not verify."));
    ui.flushOutput();
  }
  // Close the file:
  inputFile.close();
}

void readFlash16() {
  String outputFilePath = getNextFlashOutputPathAndPrintMessage();

  // Open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);
  word d = 0;
  for (unsigned long currByte = 0; currByte < flashSize / 2; currByte += 256) {
    for (word c = 0; c < 256; c++) {
      word currWord = readWord_Flash(currByte + c);
      // Split word into two bytes
      // Right
      sdBuffer[d + 1] = (( currWord >> 8 ) & 0xFF);
      // Left
      sdBuffer[d] = (currWord & 0xFF);
      d += 2;
    }
    outputFile.write(sdBuffer, 512);
    d = 0;
  }

  // Close the file:
  outputFile.close();
  ui.printlnMsg(F("Finished reading."));
  ui.flushOutput();
}

void printFlash16(int numBytes) {
  /*
    right_byte = short_val & 0xFF;
    left_byte = ( short_val >> 8 ) & 0xFF
    short_val = ( ( left_byte & 0xFF ) << 8 ) | ( right_byte & 0xFF );
  */

  char buf[3];

  for (int currByte = 0; currByte < numBytes / 2; currByte += 5) {
    // 5 words per line
    for (int c = 0; c < 5; c++) {
      word currWord = readWord_Flash(currByte + c);

      // Split word into two bytes
      byte left_byte = currWord & 0xFF;
      byte right_byte = ( currWord >> 8 ) & 0xFF;


      sprintf (buf, "%x", left_byte);
      for (size_t i = 0; i < 2 - strlen(buf); i++) {
        ui.printMsg("0");
      }
      // Now print the significant bits
      ui.printMsg(buf);

      sprintf (buf, "%x", right_byte);
      for (size_t i = 0; i < 2 - strlen(buf); i++) {
        ui.printMsg("0");
      }
      // Now print the significant bits
      ui.printMsg(buf);
    }
    ui.printlnMsg("");
  }
  ui.flushOutput();
}

// Delay between write operations based on status register
void busyCheck16() {
  // Set data pins to input
  dataIn16();

  // Read the status register
  word statusReg = readWord_Flash(0);

  while ((statusReg | 0xFF7F) != 0xFFFF) {
    statusReg = readWord_Flash(0);
  }

  // Set data pins to output
  dataOut16();
}

/******************************************
  MX29LV flashrom functions 16bit
*****************************************/
// Delay between write operations based on status register
void busyCheck16_29LV640(unsigned long myAddress, word myData) {
  // Set data pins to input
  dataIn16();

  // Read the status register
  word statusReg = readWord_Flash(myAddress);
  while ((statusReg & 0x80) != (myData & 0x80)) {
    statusReg = readWord_Flash(myAddress);
  }

  // Set data pins to output
  dataOut16();
}

void writeFlash16_29LV640(const String &inputFilePath) {
  ui.printlnMsg(F("Flashing file "));
  ui.printlnMsg(inputFilePath);
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  // Set data pins to output
  dataOut16();

  int d = 0;
  for (unsigned long currWord = 0; currWord < fileSize / 2; currWord += 256) {
    // Fill sdBuffer
    inputFile.read(sdBuffer, 512);

    // Blink led
    if (currWord % 4096 == 0)
      PORTB ^= (1 << 4);

    for (int c = 0; c < 256; c++) {
      // Write command sequence
      writeWord_Flash(0x5555, 0xaa);
      writeWord_Flash(0x2aaa, 0x55);
      writeWord_Flash(0x5555, 0xa0);

      // Write current word
      word myWord = ( ( sdBuffer[d + 1] & 0xFF ) << 8 ) | ( sdBuffer[d] & 0xFF );
      writeWord_Flash(currWord + c, myWord);
      d += 2;
      // Check if write is complete
      busyCheck16_29LV640(currWord + c, myWord);
    }
    d = 0;
  }
  // Set data pins to input again
  dataIn16();

  // Close the file:
  inputFile.close();
}

/******************************************
  Eprom functions
*****************************************/
word writeWord_Eprom(unsigned long myAddress, word myData) {
  // Data out
  DDRC = 0xFF;
  DDRA = 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t");

  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;
  // Set data
  PORTC = myData;
  PORTA = (myData >> 8) & 0xFF;

  __asm__("nop\n\t");

  // Switch VPP/OE(PH5) to HIGH
  PORTH |= (1 << 5);
  // Wait 1us for VPP High to Chip Enable Low
  delayMicroseconds(1);
  // Setting CE(PH6) LOW
  PORTH &= ~(1 << 6);

  // Leave VPP HIGH for 50us Chip Enable Program Pulse Width
  delayMicroseconds(55);

  // Setting CE(PH6) HIGH
  PORTH |= (1 << 6);
  // Wait 2us for Chip Enable High to VPP Transition
  delayMicroseconds(2);
  // Switch VPP/OE(PH5) to LOW
  PORTH &= ~(1 << 5);

  // Leave CE High for 1us for VPP Low to Chip Enable Low
  delayMicroseconds(1);

  // Data in
  DDRC = 0x00;
  DDRA = 0x00;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Setting CE(PH6) LOW
  PORTH &= ~(1 << 6);

  // Wait 1us for Chip Enable Low to Output Valid while program verify
  delayMicroseconds(3);

  // Read
  word tempWord = ( ( PINA & 0xFF ) << 8 ) | ( PINC & 0xFF );

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Setting CE(PH6) HIGH
  PORTH |= (1 << 6);

  // Delay 130ns for Chip Enable High to Output Hi-Z
  __asm__("nop\n\t""nop\n\t""nop\n\t");

  return tempWord;
}

word readWord_Eprom(unsigned long myAddress) {
  // Data in
  DDRC = 0x00;
  DDRA = 0x00;
  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t");

  // Setting CE(PH6) LOW
  PORTH &= ~(1 << 6);

  // Delay for 100ns for Address Valid/Chip Enable Low to Output Valid
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  word tempWord = ( ( PINA & 0xFF ) << 8 ) | ( PINC & 0xFF );

  // Setting CE(PH6) HIGH
  PORTH |= (1 << 6);

  return tempWord;
}

void blankcheck_Eprom() {
  ui.printlnMsg(F("Please wait..."));
  ui.flushOutput();

  blank = 1;
  for (unsigned long currWord = 0; currWord < flashSize / 2; currWord++) {
    if (readWord_Eprom(currWord) != 0xFFFF) {
      currWord = flashSize / 2;
      blank = 0;
    }
  }
  if (blank) {
    ui.printlnMsg(F("Flashrom is empty."));
    ui.flushOutput();
  }
  else {
    ui.printError(F("Error: Not blank"));
  }
}

void read_Eprom() {
  String outputFilePath = getNextFlashOutputPathAndPrintMessage();

  // Open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);
  word d = 0;
  for (unsigned long currWord = 0; currWord < flashSize / 2; currWord += 256) {
    for (word c = 0; c < 256; c++) {
      word myWord = readWord_Eprom(currWord + c);
      // Split word into two bytes
      // Right
      sdBuffer[d + 1] = ((myWord >> 8 ) & 0xFF);
      // Left
      sdBuffer[d] = (myWord & 0xFF);
      d += 2;
    }
    outputFile.write(sdBuffer, 512);
    d = 0;
  }

  // Close the file:
  outputFile.close();
  ui.printlnMsg(F("Finished reading."));
  ui.flushOutput();
}

void write_Eprom(const String& inputFilePath) {
  ui.printlnMsg(F("Flashing file "));
  ui.printlnMsg(inputFilePath);
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  // Switch VPP/OE(PH5) to HIGH
  PORTH |= (1 << 5);
  delay(1000);

  for (unsigned long currWord = 0; currWord < fileSize / 2; currWord += 256) {
    // Fill SD buffer
    inputFile.read(sdBuffer, 512);
    int d = 0;

    // Blink led
    if (currWord % 2048 == 0)
      PORTB ^= (1 << 4);

    // Work through SD buffer
    for (int c = 0; c < 256; c++) {
      word checkWord;
      word myWord = ( ( sdBuffer[d + 1] & 0xFF ) << 8 ) | ( sdBuffer[d] & 0xFF );

      // Error counter
      byte n = 0;

      // Presto III allows up to 25 rewrites per word
      do {
        // Write word
        checkWord = writeWord_Eprom(currWord + c, myWord);
        // Check for fail
        if (n == 25) {
          ui.printMsg(F("Program Error 0x"));
          ui.printlnMsg(currWord + c, HEX);
          ui.printMsg(F("0x"));
          ui.printMsg(readWord_Eprom(currWord + c), HEX);
          ui.printMsg(F(" != 0x"));
          ui.printlnMsg(myWord, HEX);
          ui.printErrorAndAbort(F("Press button to reset"), false);
        }
        n++;
      }
      while (checkWord != myWord);
      d += 2;
    }
  }
  // Close the file:
  inputFile.close();
}

void verify_Eprom(const String &inputFilePath) {
  ui.printlnMsg(F("Verifying..."));
  ui.flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  // Get rom size from file
  fileSize = inputFile.fileSize();
  if (fileSize > flashSize) {
    ui.printErrorAndAbort(F("File size exceeds flash size."), false);
  }

  blank = 0;
  word d = 0;
  for (unsigned long currWord = 0; currWord < (fileSize / 2); currWord += 256) {
    //fill sdBuffer
    inputFile.read(sdBuffer, 512);
    for (int c = 0; c < 256; c++) {
      word myWord = (((sdBuffer[d + 1] & 0xFF) << 8) | (sdBuffer[d] & 0xFF));

      if (readWord_Eprom(currWord + c) != myWord) {
        blank++;
      }
      d += 2;
    }
    d = 0;
  }
  if (blank == 0) {
    ui.printlnMsg(F("Eprom verified OK"));
    ui.flushOutput();
  }
  else {
    ui.printlnMsg(F("Verification ERROR!"));
    ui.printMsg(blank);
    ui.printError(F(" words did not verify."));
    ui.flushOutput();
  }
  // Close the file:
  inputFile.close();
}

void print_Eprom(int numBytes) {
  char buf[3];

  for (int currByte = 0; currByte < numBytes / 2; currByte += 5) {
    // 5 words per line
    for (int c = 0; c < 5; c++) {
      word currWord = readWord_Eprom(currByte + c);

      // Split word into two bytes
      byte left_byte = currWord & 0xFF;
      byte right_byte = ( currWord >> 8 ) & 0xFF;


      sprintf (buf, "%x", left_byte);
      for (size_t i = 0; i < 2 - strlen(buf); i++) {
        ui.printMsg("0");
      }
      // Now print the significant bits
      ui.printMsg(buf);

      sprintf (buf, "%x", right_byte);
      for (size_t i = 0; i < 2 - strlen(buf); i++) {
        ui.printMsg("0");
      }
      // Now print the significant bits
      ui.printMsg(buf);
    }
    ui.printlnMsg("");
  }
  ui.flushOutput();
}

//******************************************
// End of File
//******************************************
