//******************************************
// SUPER NINTENDO MODULE
//******************************************

#include <Arduino.h>
#include <avr/io.h>
#include "SNES.h"
#include "NP.h"
#include "SV.h"
#include "FLASH.h"
#include "RGB_LED.h"
#include "filebrowser.h"
#include "ui.h"
#include "globals.h"
#include "utils.h"
#include "SD.h"

/******************************************
  Defines
 *****************************************/
// SNES Hi and LoRom, SA is HI with different Sram dumping
#define EX 4
#define SA 3
#define HI 1
#define LO 0

/******************************************
   Variables
 *****************************************/
// Define SNES Cart Reader Variables
int romSpeed = 0;      // 0 = SlowROM, 3 = FastROM
int romChips = 0;      // 0 = ROM only, 1 = ROM & RAM, 2 = ROM & Save RAM,  3 = ROM & DSP1, 4 = ROM & RAM & DSP1, 5 = ROM & Save RAM & DSP1, 19 = ROM & SFX
// 227 = ROM & RAM & GameBoy data, 243 = CX4, 246 = ROM & DSP2
byte romSizeExp = 0;   // ROM-Size Exponent
boolean NP = false;
byte cx4Type = 0;
byte cx4Map = 0;

/******************************************
   Function prototypes
 *****************************************/
void regularSNESMenu();
void stopSnesClocks_resetCic_resetCart();
void setup_Snes();
void getCartInfo_SNES();
boolean checkcart_SNES();
void readROM_SNES(const String &outputFilePath);
boolean eraseSRAM(byte b);

/******************************************
  Menu
*****************************************/
// SNES start menu
void snesMenu() {
  while (true) {
    const __FlashStringHelper *item_Regular = F("Super Nintendo");
    const __FlashStringHelper *item_NP = F("NPower SF Memory");
    const __FlashStringHelper *item_SV = F("Satellaview BS-X");
    const __FlashStringHelper *item_HiROMRepro = F("HiROM repro");
    const __FlashStringHelper *item_LoROMRepro = F("LoROM repro");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_Regular,
      item_NP,
      item_SV,
      item_HiROMRepro,
      item_LoROMRepro,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("Select Cart Type"), menu, ARRAY_LENGTH(menu), item_Regular);

    if (answer == item_Regular) {
      ui.clearOutput();
      ui.flushOutput();
      setup_Snes();
      mode = CartReaderMode::SNES;
      regularSNESMenu();
    }
    else if (answer == item_NP) {
      ui.clearOutput();
      ui.flushOutput();
      setup_SFM();
      mode = CartReaderMode::SFM;
      sfmMenu();
    }
    else if (answer == item_SV) {
      ui.clearOutput();
      ui.flushOutput();
      setup_SV();
      mode = CartReaderMode::SV;
      svMenu();
    }
    else if (answer == item_HiROMRepro) {
      ui.clearOutput();
      ui.flushOutput();
      hiROM = 1;
      setup_Flash8();
      id_Flash8();
      ui.waitForUserInput();
      mode = CartReaderMode::FLASH8;
      flashromMenu8();
    }
    else if (answer == item_LoROMRepro) {
      ui.clearOutput();
      ui.flushOutput();
      hiROM = 0;
      setup_Flash8();
      id_Flash8();
      ui.waitForUserInput();
      mode = CartReaderMode::FLASH8;
      flashromMenu8();
    }
    else if (answer == item_Back) {
      break;
    }
  }
}

// SNES Menu
void regularSNESMenu() {
  while (true) {
    const __FlashStringHelper *item_ReadROM = F("Read Rom");
    const __FlashStringHelper *item_ReadSave = F("Read Save");
    const __FlashStringHelper *item_WriteSave = F("Write Save");
    const __FlashStringHelper *item_TestSRAM = F("Test SRAM");
    const __FlashStringHelper *item_CycleCart = F("Cycle cart");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_ReadROM,
      item_ReadSave,
      item_WriteSave,
      item_TestSRAM,
      item_CycleCart,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("SNES Cart Reader"), menu, ARRAY_LENGTH(menu), item_ReadROM);

    if (answer == item_ReadROM) {
      if (numBanks > 0) {
        ui.clearOutput();
        // get current time
        unsigned long startTime = millis();
        // start reading from cart
        String outputFilePath = getNextOutputPathWithNumberedFolderAndPrintMessage(F("SNES"), F("ROM"), romName, F(".sfc"));
        readROM_SNES(outputFilePath);
        compare_checksum(outputFilePath);
        // print elapsed time
        ui.printMsg(F("Time elapsed: "));
        ui.printMsg((millis() - startTime) / 1000);
        ui.printlnMsg(F("s"));
        ui.flushOutput();
      }
      else {
        ui.clearOutput();
        ui.printError(F("Does not have ROM"));
      }
    }
    else if (answer == item_ReadSave) {
      if (sramSize > 0) {
        ui.clearOutput();
        String outputFilePath = getNextSnesSRAMOutputFilePathAndPrintMessage(romName);
        readSRAM(outputFilePath);
      }
      else {
        ui.clearOutput();
        ui.printError(F("Does not have SRAM"));
      }
    }
    else if (answer == item_WriteSave) {
      if (sramSize > 0) {
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
      else {
        ui.clearOutput();
        ui.printError(F("Does not have SRAM"));
      }
    }
    else if (answer == item_TestSRAM) {
      if (sramSize > 0) {
        ui.clearOutput();
        ui.printlnMsg(F("Warning:"));
        ui.printlnMsg(F("This can erase"));
        ui.printlnMsg(F("your save games"));
        ui.printlnMsg("");
        ui.printlnMsg("");
        ui.printlnMsg(F("Press any button to"));
        ui.printlnMsg(F("start sram testing"));
        ui.flushOutput();
        ui.waitForUserInput();
        ui.clearOutput();
        String outputFilePath = getNextSnesSRAMOutputFilePathAndPrintMessage(romName);
        readSRAM(outputFilePath);
        eraseSRAM(0x00);
        eraseSRAM(0xFF);
        writeSRAM(outputFilePath);
        uint32_t writeErrors = verifySRAM(outputFilePath);
        if (writeErrors == 0) {
          ui.printlnMsg(F("Restored OK"));
          ui.flushOutput();
        }
        else {
          ui.printMsg(F("Error: "));
          ui.printMsg(writeErrors);
          ui.printlnMsg(F(" bytes "));
          ui.printError(F("did not verify."));
        }
      }
      else {
        ui.clearOutput();
        ui.printError(F("Does not have SRAM"));
      }
    }
    else if (answer == item_CycleCart) {
      // For arcademaster1 (Markfrizb) multi-game carts
      // Set reset pin to output (PH0)
      DDRH |= (1 << 0);
      // Switch RST(PH0) to LOW
      PORTH &= ~(1 << 0);

      // Note: It is probably not intended to reset CIC or clocks here
      // But if that's false, uncomment this:
      // stopSnesClocks_resetCic_resetCart();

      ui.clearOutput();
      ui.printMsg(F("Resetting..."));
      ui.flushOutput();
      delay(3000); // wait 3 secs to switch to next game
      resetArduino();
    }
    else if (answer == item_Back) {
      stopSnesClocks_resetCic_resetCart();
      break;
    }

    ui.printlnMsg("");
    ui.printlnMsg(F("Press Button..."));
    ui.flushOutput();
    ui.waitForUserInput();
  }
}

// Menu for manual configuration
void confMenuManual() {
  while (true) {
    const __FlashStringHelper *item_Header = F("Use header info");
    const __FlashStringHelper *item_4MBLoRom = F("4MB LoRom 256K Sram");
    const __FlashStringHelper *item_4MBHiRom = F("4MB HiRom 64K Sram");
    const __FlashStringHelper *item_6MBExRom = F("6MB ExRom 256K Sram");
    const __FlashStringHelper *item_Reset = F("Reset");
    const __FlashStringHelper *menu[] = {
      item_Header,
      item_4MBLoRom,
      item_4MBHiRom,
      item_6MBExRom,
      item_Reset,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("Choose mapping"), menu, ARRAY_LENGTH(menu), item_Header);

    if (answer == item_Header) {
      break;
    }
    else if (answer == item_4MBLoRom) {
      romType = LO;
      numBanks = 128;
      sramSize = 256;
      strcpy(romName, "LOROM");
    }
    else if (answer == item_4MBHiRom) {
      romType = HI;
      numBanks = 64;
      sramSize = 64;
      strcpy(romName, "HIROM");
    }
    else if (answer == item_6MBExRom) {
      romType = EX;
      numBanks = 96;
      sramSize = 256;
      strcpy(romName, "EXROM");
    }
    else if (answer == item_Reset) {
      stopSnesClocks_resetCic_resetCart();
      resetArduino();
    }
  }
}

String getNextSnesSRAMOutputFilePathAndPrintMessage(const String &romName) {
  return getNextOutputPathWithNumberedFolderAndPrintMessage(F("SNES"), F("SAVE"), romName, F(".srm"));
}

void stopSnesClocks_resetCic_resetCart() {
  DDRG |= (1 << 1);   // Set cicrstPin(PG1) to Output
  PORTG |= (1 << 1);  // pull high = reset CIC
  DDRH |= (1 << 0);   // Set RST(PH0) pin to Output
  PORTH &= ~(1 << 0); // Switch RST(PH0) to LOW
  clockgen.output_enable(SI5351_CLK1, 0); // CPU clock
  clockgen.output_enable(SI5351_CLK2, 0); // CIC clock
  clockgen.output_enable(SI5351_CLK0, 0); // master clock
}

/******************************************
   Setup
 *****************************************/
void setup_Snes() {
  // Set cicrstPin(PG1) to Output
  DDRG |= (1 << 1);
  // Output a high signal until we're ready to start
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
  //PA0-PA7
  DDRA = 0xFF;

  // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) RD(PH6)
  DDRH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);
  // Switch RST(PH0) and WR(PH5) to HIGH
  PORTH |= (1 << 0) | (1 << 5);
  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));

  // Set Refresh(PE5) to Output
  DDRE |= (1 << 5);
  // Switch Refresh(PE5) to LOW (needed for SA-1)
  PORTE &= ~(1 << 5);

  // Set CPU Clock(PH1) to Output
  DDRH |= (1 << 1);
  //PORTH &= ~(1 << 1);

  // Set IRQ(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  //PORTH |= (1 << 4);

  // Set expand(PG5) to Input
  DDRG &= ~(1 << 5);
  // Activate Internal Pullup Resistors
  //PORTG |= (1 << 5);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Enable Internal Pullups
  //PORTC = 0xFF;

  // Unused pins
  // Set wram(PE4) to Output
  DDRE |= (1 << 4);
  //PORTE &= ~(1 << 4);
  // Set pawr(PJ1) to Output
  DDRJ |= (1 << 1);
  //PORTJ &= ~(1 << 1);
  // Set pard(PJ0) to Output
  DDRJ |= (1 << 0);
  //PORTJ &= ~(1 << 0);

  // Adafruit Clock Generator
  // last number is the clock correction factor which is custom for each clock generator
  clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, -16000);

  // Set clocks to 4Mhz/1Mhz for better SA-1 unlocking
  clockgen.set_freq(100000000ULL, SI5351_CLK1); // CPU
  clockgen.set_freq(100000000ULL, SI5351_CLK2); // CIC
  clockgen.set_freq(400000000ULL, SI5351_CLK0); // EXT

  // Start outputting master clock, CIC clock
  clockgen.output_enable(SI5351_CLK1, 0); // no CPU clock yet; seems to affect SA-1 success a lot
  clockgen.output_enable(SI5351_CLK2, 1); // CIC clock (should go before master clock)
  clockgen.output_enable(SI5351_CLK0, 1); // master clock

  // Wait for clock generator
  clockgen.update_status();
  delay(500);

  // Start CIC by outputting a low signal to cicrstPin(PG1)
  PORTG &= ~(1 << 1);

  // Wait for CIC reset
  delay(500);

  // Print all the info
  getCartInfo_SNES();

  //Set clocks to standard or else SA-1 sram writing will fail
  clockgen.set_freq(2147727200ULL, SI5351_CLK0);
  clockgen.set_freq(357954500ULL, SI5351_CLK1);
  clockgen.set_freq(307200000ULL, SI5351_CLK2);
}

/******************************************
   I/O Functions
 *****************************************/
// Switch control pins to write
void controlOut_SNES() {
  // Switch RD(PH6) and WR(PH5) to HIGH
  PORTH |= (1 << 6) | (1 << 5);
  // Switch CS(PH3) to LOW
  PORTH &= ~(1 << 3);
}

// Switch control pins to read
void controlIn_SNES() {
  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);
  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));
}

/******************************************
   Low level functions
 *****************************************/
// Write one byte of data to a location specified by bank and address, 00:0000
void writeBank_SNES(byte myBank, word myAddress, byte myData) {
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
byte readBank_SNES(byte myBank, word myAddress) {
  PORTL = myBank;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  // Wait for the Byte to appear on the data bus
  // Arduino running at 16Mhz -> one nop = 62.5ns
  // slowRom is good for 200ns, fastRom is <= 120ns; S-CPU best case read speed: 3.57MHz / 280ns
  // let's be conservative and use 6 x 62.5 = 375ns
  NOP; NOP; NOP; NOP; NOP; NOP;

  // Read
  byte tempByte = PINC;
  return tempByte;
}

void readLoRomBanks( unsigned int start, unsigned int total, SafeSDFile &file)
{
  byte buffer[1024] = { 0 };

  uint16_t c = 0;
  uint16_t currByte = 32768;

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(total - start) * 1024;
  ui.drawProgressBar(0, totalProgressBar);

  for (unsigned int currBank = start; currBank < total; currBank++) {
    PORTL = currBank;

    // Blink led
    PORTB ^= (1 << 4);

    currByte = 32768;
    while (1) {
      c = 0;
      while (c < 1024) {
        PORTF = (currByte & 0xFF);
        PORTK = ((currByte >> 8) & 0xFF);

        // Wait for the Byte to appear on the data bus
        // Arduino running at 16Mhz -> one nop = 62.5ns
        // slowRom is good for 200ns, fastRom is <= 120ns; S-CPU best case read speed: 3.57MHz / 280ns
        // let's be conservative and use 6 x 62.5 = 375ns
        NOP; NOP; NOP; NOP; NOP; NOP;

        buffer[c] = PINC;
        c++;
        currByte++;
      }
      file.write(buffer, 1024);

      // exit while(1) loop once the uint16_t currByte overflows from 0xffff to 0 (current bank is done)
      if (currByte == 0) break;
    }

    // update progress bar
    processedProgressBar += 1024;
    ui.drawProgressBar(processedProgressBar, totalProgressBar);
  }
}

void readHiRomBanks( unsigned int start, unsigned int total, SafeSDFile &file)
{
  byte buffer[1024] = { 0 };

  uint16_t c = 0;
  uint16_t currByte = 0;

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(total - start) * 1024;
  ui.drawProgressBar(0, totalProgressBar);

  for (unsigned int currBank = start; currBank < total; currBank++) {
    PORTL = currBank;

    // Blink led
    PORTB ^= (1 << 4);

    currByte = 0;
    while (1) {
      c = 0;
      while (c < 1024) {
        PORTF = (currByte & 0xFF);
        PORTK = ((currByte >> 8) & 0xFF);

        // Wait for the Byte to appear on the data bus
        // Arduino running at 16Mhz -> one nop = 62.5ns
        // slowRom is good for 200ns, fastRom is <= 120ns; S-CPU best case read speed: 3.57MHz / 280ns
        // let's be conservative and use 6 x 62.5 = 375ns
        NOP; NOP; NOP; NOP; NOP; NOP;

        buffer[c] = PINC;
        c++;
        currByte++;
      }
      file.write(buffer, 1024);

      // exit while(1) loop once the uint16_t currByte overflows from 0xffff to 0 (current bank is done)
      if (currByte == 0) break;
    }

    // update progress bar
    processedProgressBar += 1024;
    ui.drawProgressBar(processedProgressBar, totalProgressBar);
  }
}

/******************************************
  SNES ROM Functions
******************************************/
void getCartInfo_SNES() {
  boolean manualConfig = 0;

  //Prime SA1 cartridge
  uint16_t c = 0;
  uint16_t currByte = 0;
  PORTL = 192;
  while (c < 1024) {
    PORTF = (currByte & 0xFF);
    PORTK = ((currByte >> 8) & 0xFF);

    // Wait for the Byte to appear on the data bus
    // Arduino running at 16Mhz -> one nop = 62.5ns
    // slowRom is good for 200ns, fastRom is <= 120ns; S-CPU best case read speed: 3.57MHz / 280ns
    // let's be conservative and use 6 x 62.5 = 375ns
    NOP; NOP; NOP; NOP; NOP; NOP;

    byte value [[gnu::unused]] = PINC;
    c++;
    currByte++;
  }

  // Print start page
  if (checkcart_SNES() == 0) {
    // Checksum either corrupt or 0000
    manualConfig = 1;
    errorLvl = 1;
    rgb.setColor(255, 0, 0);

    ui.clearOutput();
    ui.printlnMsg(F("ERROR"));
    ui.printlnMsg(F("Rom header corrupt"));
    ui.printlnMsg(F("or missing"));
    ui.printlnMsg("");
    ui.printlnMsg("");
    ui.printlnMsg(F("Press button for"));
    ui.printlnMsg(F("manual configuration"));
    ui.printlnMsg(F("or powercycle if SA1"));
    ui.flushOutput();
    ui.waitForUserInput();
    // ui.waitForUserInput() clears errors but in this case we still have an error
    errorLvl = 1;
  }

  ui.clearOutput();
  ui.printMsg(F("Name: "));
  ui.printlnMsg(romName);

  ui.printMsg(F("Type: "));
  if (romType == HI)
    ui.printMsg(F("HiROM"));
  else if (romType == LO)
    ui.printMsg(F("LoROM"));
  else if (romType == EX)
    ui.printMsg(F("ExHiRom"));
  else
    ui.printMsg(romType);
  ui.printMsg(F(" "));
  if (romSpeed == 0)
    ui.printlnMsg(F("SlowROM"));
  else if (romSpeed == 2)
    ui.printlnMsg(F("SlowROM"));
  else if (romSpeed == 3)
    ui.printlnMsg(F("FastROM"));
  else
    ui.printlnMsg(romSpeed);

  ui.printMsg(F("ICs: ROM "));
  if (romChips == 0)
    ui.printlnMsg(F("ONLY"));
  else if (romChips == 1)
    ui.printlnMsg(F("RAM"));
  else if (romChips == 2)
    ui.printlnMsg(F("SAVE"));
  else if (romChips == 3)
    ui.printlnMsg(F("DSP1"));
  else if (romChips == 4)
    ui.printlnMsg(F("DSP1 RAM"));
  else if (romChips == 5)
    ui.printlnMsg(F("DSP1 SAVE"));
  else if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26))
    ui.printlnMsg(F("SuperFX"));
  else if (romChips == 52) {
    ui.printlnMsg(F("SA1 RAM"));
    romType = SA;
  }
  else if (romChips == 53) {
    ui.printlnMsg(F("SA1 RAM BATT"));
    romType = SA;
  }
  else if (romChips == 69) {
    ui.printlnMsg(F("SDD1 BATT"));
  }
  else if (romChips == 227)
    ui.printlnMsg(F("RAM GBoy"));
  else if (romChips == 243)
    ui.printlnMsg(F("CX4"));
  else if (romChips == 246)
    ui.printlnMsg(F("DSP2"));
  else if (romChips == 245)
    ui.printlnMsg(F("SPC RAM BATT"));
  else if (romChips == 249)
    ui.printlnMsg(F("SPC RAM RTC"));
  else
    ui.printlnMsg(F(""));

  ui.printMsg(F("Rom Size: "));
  ui.printMsg(romSize);
  ui.printlnMsg(F("Mbit"));

  ui.printMsg(F("Banks: "));
  ui.printMsg(numBanks);
  ui.printMsg(F(" Chips: "));
  ui.printlnMsg(romChips);

  ui.printMsg(F("Sram Size: "));
  ui.printMsg(sramSize);
  ui.printlnMsg(F("Kbit"));

  ui.printMsg(F("ROM Version: 1."));
  ui.printlnMsg(romVersion);

  ui.printMsg(F("Checksum: "));
  ui.printlnMsg(checksumStr);
  ui.flushOutput();

  // Wait for user input
  ui.printlnMsg(F(" "));
  ui.printlnMsg(F(" "));
  ui.printlnMsg(F("Press Button..."));
  ui.flushOutput();
  ui.waitForUserInput();

  // Start manual config
  if (manualConfig == 1) {
    confMenuManual();
  }
}

void checkAltConf() {
  char tempStr1[2];
  char tempStr2[5];

  if (fileExists(F("/snes.txt"))) {
    SafeSDFile snesFile = SafeSDFile::openForReading(F("/snes.txt"));
    while (snesFile.bytesAvailable() > 0) {
      // Read 4 bytes into String, do it one at a time so byte order doesn't get mixed up
      sprintf(tempStr1, "%c", snesFile.readByteOrDie());
      strcpy(tempStr2, tempStr1);
      sprintf(tempStr1, "%c", snesFile.readByteOrDie());
      strcat(tempStr2, tempStr1);
      sprintf(tempStr1, "%c", snesFile.readByteOrDie());
      strcat(tempStr2, tempStr1);
      sprintf(tempStr1, "%c", snesFile.readByteOrDie());
      strcat(tempStr2, tempStr1);

      // Check if string is a match
      if (strcmp(tempStr2, checksumStr) == 0) {

        // Skip the , in the file
        snesFile.seekCur(1);

        // Read next two bytes into a string
        romSize = snesFile.readByteOrDie() - 48;
        romSize = romSize * 10 + snesFile.readByteOrDie() - 48;

        // Skip the , in the file
        snesFile.seekCur(1);

        // Add next two bytes to the string
        numBanks = snesFile.readByteOrDie() - 48;
        numBanks = numBanks * 10 + snesFile.readByteOrDie() - 48;
      }
      // If no match empty string advance by 8 and try again
      else {
        snesFile.seekCur(8);
      }
    }

    snesFile.close();
  }
}

// Read header
boolean checkcart_SNES() {
  // set control to read
  dataIn();

  uint16_t c = 0;
  uint16_t headerStart = 0xFFC0;
  uint16_t currByte = headerStart;
  byte snesHeader[64] = { 0 };
  PORTL = 0;
  while (c < 64) {
    PORTF = (currByte & 0xFF);
    PORTK = ((currByte >> 8) & 0xFF);

    NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;

    snesHeader[c] = PINC;
    c++;
    currByte++;
  }

  // Get Checksum as string
  sprintf(checksumStr, "%02X%02X", snesHeader[0xFFDF - headerStart], snesHeader[0xFFDE - headerStart]);

  romType = snesHeader[0xFFD5 - headerStart];
  if ((romType >> 5) != 1) {  // Detect invalid romType byte due to too long ROM name (22 chars)
    romType = LO; // LoROM   // Krusty's Super Fun House (U) 1.0 & Contra 3 (U)
  }
  else if (romType == 0x35) {
    romType = EX; // Check if ExHiROM
  }
  else if (romType == 0x3A) {
    romType = HI; // Check if SPC7110
  }
  else {
    romType &= 1; // Must be LoROM or HiROM
  }

  // Check RomSpeed
  romSpeed = (snesHeader[0xFFD5 - headerStart] >> 4);

  // Check RomChips
  romChips = snesHeader[0xFFD6 - headerStart];

  if (romChips == 69) {
    romSize = 48;
    numBanks = 96;
    romType = HI;
  }
  else if (romChips == 243) {
    cx4Type = snesHeader[0xFFC9 - headerStart] & 0xF;
    if (cx4Type == 2) { // X2
      romSize = 12;
      numBanks = 48;
    }
    else if (cx4Type == 3) { // X3
      romSize = 16;
      numBanks = 64;
    }
  }
  else if ((romChips == 245) && (romType == HI)) {
    romSize = 24;
    numBanks = 48;
  }
  else if ((romChips == 249) && (romType == HI)) {
    romSize = 40;
    numBanks = 80;
  }
  else {
    // Check RomSize
    byte romSizeExp = snesHeader[0xFFD7 - headerStart] - 7;
    romSize = 1;
    while (romSizeExp--)
      romSize *= 2;

    if ((romType == EX) || (romType == SA)) {
      numBanks = long(romSize) * 2;
    }
    else {
      numBanks = (long(romSize) * 1024 * 1024 / 8) / (32768 + (long(romType) * 32768));
    }
  }

  //Check SD card for alt config
  checkAltConf();

  // Get name
  byte myByte = 0;
  byte myLength = 0;
  for (unsigned int i = 0xFFC0; i < 0xFFD4; i++) {
    myByte = snesHeader[i - headerStart];
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
      myByte = snesHeader[0xFFB2 + i - headerStart];
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
  byte sramSizeExp;
  if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) {
    // SuperFX
    if (snesHeader[0x7FDA - headerStart] == 0x33) {
      sramSizeExp = snesHeader[0x7FBD - headerStart];
    }
    else {
      if (strncmp(romName, "STARFOX2", 8) == 0) {
        sramSizeExp = 6;
      }
      else {
        sramSizeExp = 5;
      }
    }
  }
  else {
    // No SuperFX
    sramSizeExp = snesHeader[0xFFD8 - headerStart];
  }

  // Calculate sramSize
  // Fail states usually have sramSizeExp at 255 (no cart inserted, SA-1 failure, etc)
  if (sramSizeExp != 0 && sramSizeExp != 255) {
    sramSizeExp = sramSizeExp + 3;
    sramSize = 1;
    while (sramSizeExp--)
      sramSize *= 2;
  }
  else {
    sramSize = 0;
  }

  // Check Cart Country
  //int cartCountry = snesHeader[0xFFD9 - headerStart];

  // ROM Version
  romVersion = snesHeader[0xFFDB - headerStart];

  // Test if checksum is equal to reverse checksum
  if (((static_cast<word>(snesHeader[0xFFDC - headerStart]) + (static_cast<word>(snesHeader[0xFFDD - headerStart]) * 256)) + (static_cast<word>(snesHeader[0xFFDE - headerStart]) + (static_cast<word>(snesHeader[0xFFDF - headerStart]) * 256))) == 65535) {
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

unsigned int calc_checksum(const String &filePath) {
  unsigned int calcChecksum = 0;
  unsigned int calcChecksumChunk = 0;
  int calcFilesize = 0;
  unsigned int c = 0;
  unsigned long i = 0;
  unsigned long j = 0;

  SafeSDFile fileToChecksum = SafeSDFile::openForReading(filePath);
  calcFilesize = fileToChecksum.fileSize() * 8 / 1024 / 1024;

  // Nintendo Power (SF Memory Cassette)
  // Read up to 0x60000 then add FFs to 0x80000
  if (NP == true) {
    for (i = 0; i < (0x60000 / 512); i++) {
      fileToChecksum.read(sdBuffer, 512);
      for (c = 0; c < 512; c++) {
        calcChecksumChunk += sdBuffer[c];
      }
      calcChecksum = calcChecksumChunk;
    }
    calcChecksum += 0xF47C; // FFs from 0x60000-0x80000
  }
  else if ((calcFilesize == 10) || (calcFilesize == 12) || (calcFilesize == 20) || (calcFilesize == 24)) {
    unsigned long calcBase = 0;
    unsigned long calcMirror = 0;
    byte calcMirrorCount = 0;
    if (calcFilesize > 16)
      calcBase = 2097152;
    else
      calcBase = 1048576;
    calcMirror = fileToChecksum.fileSize() - calcBase;
    calcMirrorCount = calcBase / calcMirror;

    // Momotarou Dentetsu Happy Fix 3MB (24Mbit)
    if ((calcFilesize == 24) && (romChips == 245)) {
      for (i = 0; i < (fileToChecksum.fileSize() / 512); i++) {
        fileToChecksum.read(sdBuffer, 512);
        for (c = 0; c < 512; c++) {
          calcChecksumChunk += sdBuffer[c];
        }
      }
      calcChecksum = 2 * calcChecksumChunk;
    }
    else {
      // Base 8/16 Mbit chunk
      for (j = 0; j < (calcBase / 512); j++) {
        fileToChecksum.read(sdBuffer, 512);
        for (c = 0; c < 512; c++) {
          calcChecksumChunk += sdBuffer[c];
        }
      }
      calcChecksum = calcChecksumChunk;
      calcChecksumChunk = 0;
      // Add the mirrored chunk
      for (j = 0; j < (calcMirror / 512); j++) {
        fileToChecksum.read(sdBuffer, 512);
        for (c = 0; c < 512; c++) {
          calcChecksumChunk += sdBuffer[c];
        }
      }
      calcChecksum +=  calcMirrorCount * calcChecksumChunk;
    }
  }
  else if ((calcFilesize == 40) && (romChips == 85)) {
    // Daikaijuu Monogatari 2 Fix 5MB (40Mbit)
    // Add the 4MB (32Mbit) start
    for (j = 0; j < (4194304 / 512); j++) {
      fileToChecksum.read(sdBuffer, 512);
      for (c = 0; c < 512; c++) {
        calcChecksumChunk += sdBuffer[c];
      }
      calcChecksum = calcChecksumChunk;
    }
    calcChecksumChunk = 0;
    // Add the 1MB (8Mbit) end
    for (j = 0; j < (1048576 / 512); j++) {
      fileToChecksum.read(sdBuffer, 512);
      for (c = 0; c < 512; c++) {
        calcChecksumChunk += sdBuffer[c];
      }
    }
    calcChecksum +=  4 * calcChecksumChunk;
  }
  else if (calcFilesize == 48) {
    // Star Ocean/Tales of Phantasia Fix 6MB (48Mbit)
    // Add the 4MB (32Mbit) start
    for (j = 0; j < (4194304 / 512); j++) {
      fileToChecksum.read(sdBuffer, 512);
      for (c = 0; c < 512; c++) {
        calcChecksumChunk += sdBuffer[c];
      }
      calcChecksum = calcChecksumChunk;
    }
    calcChecksumChunk = 0;
    // Add the 2MB (16Mbit) end
    for (j = 0; j < (2097152 / 512); j++) {
      fileToChecksum.read(sdBuffer, 512);
      for (c = 0; c < 512; c++) {
        calcChecksumChunk += sdBuffer[c];
      }
    }
    calcChecksum +=  2 * calcChecksumChunk;
  }
  else {
    //calcFilesize == 2 || 4 || 8 || 16 || 32 || 40 || etc
    for (i = 0; i < (fileToChecksum.fileSize() / 512); i++) {
      fileToChecksum.read(sdBuffer, 512);
      for (c = 0; c < 512; c++) {
        calcChecksumChunk += sdBuffer[c];
      }
      calcChecksum = calcChecksumChunk;
    }
  }
  fileToChecksum.close();
  return (calcChecksum);
}

boolean compare_checksum(const String &filePath) {
  ui.printlnMsg(F("Calculating Checksum"));
  ui.flushOutput();

  char calcsumStr[5];
  sprintf(calcsumStr, "%04X", calc_checksum(filePath));

  if (strcmp(calcsumStr, checksumStr) == 0) {
    ui.printMsg(F("Checksum OK: "));
    ui.printlnMsg(calcsumStr);
    ui.flushOutput();
    return 1;
  }
  else {
    ui.printMsg(F("Checksum Error: "));
    ui.printlnMsg(calcsumStr);
    ui.printError("");
    ui.flushOutput();
    return 0;
  }
}

// Read rom to SD card
void readROM_SNES(const String &outputFilePath) {
  // Set control
  dataIn();
  controlIn_SNES();

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  //Dump Derby Stallion '96 (Japan) Actual Size is 24Mb
  if ((romType == LO) && (numBanks == 128) && (strcmp("CC86", checksumStr) == 0)) {
    // Read Banks 0x00-0x3F for the 1st/2nd MB
    for (int currBank = 0; currBank < 64; currBank++) {
      // Dump the bytes to SD 512B at a time
      for (long currByte = 32768; currByte < 65536; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SNES(currBank, currByte + c);
        }
        outputFile.write(sdBuffer, 512);
      }
    }
    //Read Bank 0x80-9F for the 3rd MB
    for (int currBank = 128; currBank < 160; currBank++) {
      // Dump the bytes to SD 512B at a time
      for (long currByte = 32768; currByte < 65536; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SNES(currBank, currByte + c);
        }
        outputFile.write(sdBuffer, 512);
      }
    }
  }

  //Dump Low-type ROM
  else if (romType == LO) {
    if (romChips == 243) { //0xF3
      cx4Map = readBank_SNES(0, 32594); //0x7F52
      if ((cx4Type == 2) && (cx4Map != 0)) { //X2
        dataOut();
        controlOut_SNES();
        writeBank_SNES(0, 32594, 0); // Set 0x7F52 to 0
        dataIn();
        controlIn_SNES();
      }
      else if ((cx4Type == 3) && (cx4Map == 0)) { //X3
        dataOut();
        controlOut_SNES();
        writeBank_SNES(0, 32594, 1); // Set 0x7F52 to 1
        dataIn();
        controlIn_SNES();
      }
    }
    if (romSize > 24) {
      // ROM > 96 banks (up to 128 banks)
      readLoRomBanks( 0x80, numBanks + 0x80, outputFile);
    } else {
      // Read up to 96 banks starting at bank 0×00.
      readLoRomBanks( 0, numBanks, outputFile);
    }
    if (romChips == 243) { //0xF3
      // Restore CX4 Mapping Register
      dataOut();
      controlOut_SNES();
      writeBank_SNES(0, 32594, cx4Map); // 0x7F52
      dataIn();
      controlIn_SNES();
    }
  }

  // Dump SDD1 High-type ROM
  else if ((romType == HI) && (romChips == 69)) {
    ui.printlnMsg(F("Dumping SDD1 HiRom"));
    ui.flushOutput();
    controlIn_SNES();
    byte initialSOMap = readBank_SNES(0, 18439);

    for (int currMemmap = 0; currMemmap < (numBanks / 16); currMemmap++) {

      dataOut();
      controlOut_SNES();

      writeBank_SNES(0, 18439, currMemmap);

      dataIn();
      controlIn_SNES();

      readHiRomBanks( 240, 256, outputFile);
      if (currMemmap == 2) ui.clearOutput();  // need more space for the progress bars
    }

    dataOut();
    controlOut_SNES();

    writeBank_SNES(0, 18439, initialSOMap);

    dataIn();
    controlIn_SNES();
  }

  // Dump SPC7110 High-type ROM
  else if ((romType == HI) && ((romChips == 245) || (romChips == 249))) {
    ui.printlnMsg(F("Dumping SPC7110 HiRom"));
    ui.flushOutput();

    // 0xC00000-0xDFFFFF
    //ui.printMsg(F("Part 1"));
    ui.flushOutput();
    readHiRomBanks( 192, 224, outputFile);

    if (numBanks > 32) {
      dataOut();
      controlOut_SNES();
      // Set 0x4834 to 0xFF
      writeBank_SNES( 0, 0x4834, 0xFF );

      dataIn();
      controlIn_SNES();

      // 0xE00000-0xEFFFFF
      //ui.printMsg(F(" 2"));
      ui.flushOutput();
      readHiRomBanks( 224, 240, outputFile);

      if (numBanks > 48) {
        // 0xF00000-0xFFFFFF
        //ui.printMsg(F(" 3"));
        ui.flushOutput();
        readHiRomBanks( 240, 256, outputFile);

        dataOut();
        controlOut_SNES();

        // Set 0x4833 to 3
        writeBank_SNES( 0, 0x4833, 3 );

        dataIn();
        controlIn_SNES();

        // 0xF00000-0xFFFFFF
        //ui.printMsg(F(" 4"));
        ui.flushOutput();
        readHiRomBanks( 240, 256, outputFile);
      }
      //ui.printlnMsg(F(""));
      ui.clearOutput();  // need more space due to the 4 progress bars

      // Return mapping registers to initial settings...
      dataOut();
      controlOut_SNES();

      writeBank_SNES( 0, 0x4833, 2 );
      writeBank_SNES( 0, 0x4834, 0 );

      dataIn();
      controlIn_SNES();
    }
  }

  // Dump standard High-type ROM
  else if ((romType == HI) || (romType == SA) || (romType == EX)) {
    ui.printlnMsg(F("Dumping HiRom..."));
    ui.flushOutput();

    readHiRomBanks( 192, numBanks + 192, outputFile);
  }

  // Close the file:
  outputFile.close();
}

/******************************************
  SNES SRAM Functions
*****************************************/
// Write file to SRAM
void writeSRAM(const String &inputFilePath) {
  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

  // Set pins to output
  dataOut();

  // Set RST RD WR to High and CS to Low
  controlOut_SNES();

  // LoRom
  if (romType == LO) {
    // Sram size
    long lastByte = (long(sramSize) * 128);

    if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) { // SuperFX
      if (lastByte > 0x10000) { // Large SuperFX SRAM (no known carts)
        int sramBanks = lastByte / 0x10000; // TODO: Changed this to local instead of GB global, is it ok?
        for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
          for (long currByte = 0x0000; currByte < 0x10000; currByte++) {
            writeBank_SNES(currBank, currByte, inputFile.readByteOrDie());
          }
        }
      }
      else { // SuperFX SRAM
        for (long currByte = 0; currByte < lastByte; currByte++) {
          writeBank_SNES(0x70, currByte, inputFile.readByteOrDie());
        }
      }
    }
    else if (lastByte > 0x8000) { // Large SRAM Fix
      int sramBanks = lastByte / 0x8000; // TODO: Changed this to local instead of GB global, is it ok?
      for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
        for (long currByte = 0x0000; currByte < 0x8000; currByte++) {
          writeBank_SNES(currBank, currByte, inputFile.readByteOrDie());
        }
      }
    }
    else {
      for (long currByte = 0; currByte <  lastByte; currByte++) {
        writeBank_SNES(0x70, currByte, inputFile.readByteOrDie());
      }
    }
  }
  // HiRom
  else if (romType == HI) {
    if ((romChips == 245) || (romChips == 249)) { // SPC7110 SRAM
      // Configure SPC7110 SRAM Register
      // Set 0x4830 to 0x80
      writeBank_SNES(0, 0x4830, 0x80);
      // Sram size
      long lastByte = (long(sramSize) * 128) + 0x6000;
      // Write to sram bank
      for (long currByte = 0x6000; currByte < lastByte; currByte++) {
        writeBank_SNES(0x30, currByte, inputFile.readByteOrDie());
      }
      // Reset SPC7110 SRAM Register
      dataOut();
      // Reset 0x4830 to 0x0
      writeBank_SNES(0, 0x4830, 0);
      dataIn();
    }
    else {
      // Writing SRAM on HiRom needs CS(PH3) to be high
      PORTH |=  (1 << 3);
      // Sram size
      long lastByte = (long(sramSize) * 128);
      if (lastByte > 0x2000) { // Large SRAM Fix
        int sramBanks = lastByte / 0x2000;
        for (int currBank = 0x30; currBank < sramBanks + 0x30; currBank++) {
          for (long currByte = 0x6000; currByte < 0x8000; currByte++) {
            writeBank_SNES(currBank, currByte, inputFile.readByteOrDie());
          }
        }
      }
      else {
        lastByte += 0x6000;
        // Write to sram bank
        for (long currByte = 0x6000; currByte < lastByte; currByte++) {
          writeBank_SNES(0x30, currByte, inputFile.readByteOrDie());
        }
      }
    }
  }
  // ExHiRom
  else if (romType == EX) {
    // Writing SRAM on HiRom needs CS(PH3) to be high
    PORTH |=  (1 << 3);
    // Sram size
    long lastByte = (long(sramSize) * 128) + 0x6000;
    for (long currByte = 0x6000; currByte < lastByte; currByte++) {
      writeBank_SNES(0xB0, currByte, inputFile.readByteOrDie());
    }
  }
  // SA1
  else if (romType == SA) {
    long lastByte = (long(sramSize) * 128);
    // Enable CPU Clock
    clockgen.output_enable(SI5351_CLK1, 1);

    // Direct writes to BW-RAM (SRAM) in banks 0x40-0x43 don't work
    // Break BW-RAM (SRAM) into 0x2000 blocks
    byte lastBlock = 0;
    lastBlock = lastByte / 0x2000;

    // Writing SRAM on SA1 needs CS(PH3) to be high
    // PORTH |=  (1 << 3);

    // Setup BW-RAM
    // Set 0x2224 (SNES BMAPS) to map SRAM Block 0 to 0x6000-0x7FFF
    writeBank_SNES(0, 0x2224, 0);
    // Set 0x2226 (SNES SBWE) to 0x80 Write Enable
    writeBank_SNES(0, 0x2226, 0x80);
    // Set 0x2228 (SNES BWPA) to 0x00 BW-RAM Write-Protected Area
    writeBank_SNES(0, 0x2228, 0);
    delay(1000);

    // Use $2224 (SNES) to map BW-RAM block to 0x6000-0x7FFF
    // Use $2226 (SNES) to write enable the BW-RAM
    byte firstByte = 0;
    for (byte currBlock = 0; currBlock < lastBlock; currBlock++) {
      // Set 0x2224 (SNES BMAPS) to map SRAM Block to 0x6000-0x7FFF
      writeBank_SNES(0, 0x2224, currBlock);
      // Set 0x2226 (SNES SBWE) to 0x80 Write Enable
      writeBank_SNES(0, 0x2226, 0x80);
      for (long currByte = 0x6000; currByte < 0x8000; currByte += 512) {
        inputFile.read(sdBuffer, 512);
        if ((currBlock == 0) && (currByte == 0x6000)) {
          firstByte = sdBuffer[0];
        }
        for (int c = 0; c < 512; c++) {
          writeBank_SNES(0, currByte + c, sdBuffer[c]);
        }
      }
    }
    // Rewrite First Byte
    writeBank_SNES(0, 0x2224, 0);
    writeBank_SNES(0, 0x2226, 0x80);
    writeBank_SNES(0, 0x6000, firstByte);
    // Disable CPU clock
    clockgen.output_enable(SI5351_CLK1, 0);
  }

  // Set pins to input
  dataIn();

  // Close the file:
  inputFile.close();
  ui.printlnMsg(F("SRAM writing finished"));
  ui.flushOutput();
}

void readSRAM(const String &outputFilePath) {
  // set control
  controlIn_SNES();

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);
  int sramBanks = 0;
  if (romType == LO) {
    // Sram size
    long lastByte = (long(sramSize) * 128);
    if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) { // SuperFX
      if (lastByte > 0x10000) { // Large SuperFX SRAM (no known carts)
        sramBanks = lastByte / 0x10000;
        for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
          for (long currByte = 0x0000; currByte < 0x10000; currByte++) {
            outputFile.writeByte(readBank_SNES(currBank, currByte));
          }
        }
      }
      else { // SuperFX SRAM
        for (long currByte = 0; currByte < lastByte; currByte++) {
          outputFile.writeByte(readBank_SNES(0x70, currByte));
        }
      }
    }
    else if (lastByte > 0x8000) { // Large SRAM Fix
      sramBanks = lastByte / 0x8000;
      for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
        for (long currByte = 0x0000; currByte < 0x8000; currByte++) {
          outputFile.writeByte(readBank_SNES(currBank, currByte));
        }
      }
    }
    else {
      for (long currByte = 0; currByte < lastByte; currByte++) {
        outputFile.writeByte(readBank_SNES(0x70, currByte));
      }
    }
  }
  else if (romType == HI) {
    if ((romChips == 245) || (romChips == 249)) { // SPC7110 SRAM
      // Configure SPC7110 SRAM Register
      dataOut();
      // Set 0x4830 to 0x80
      writeBank_SNES(0, 0x4830, 0x80);
      dataIn();
      // Sram size
      long lastByte = (long(sramSize) * 128) + 0x6000;
      for (long currByte = 0x6000; currByte < lastByte; currByte++) {
        outputFile.writeByte(readBank_SNES(0x30, currByte));
      }
      dataOut();
      // Reset 0x4830 to 0x0
      writeBank_SNES(0, 0x4830, 0);
      dataIn();
    }
    else {
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |=  (1 << 3);
      // Sram size
      long lastByte = (long(sramSize) * 128);
      if (lastByte > 0x2000) { // Large SRAM Fix
        sramBanks = lastByte / 0x2000;
        for (int currBank = 0x30; currBank < sramBanks + 0x30; currBank++) {
          for (long currByte = 0x6000; currByte < 0x8000; currByte++) {
            outputFile.writeByte(readBank_SNES(currBank, currByte));
          }
        }
      }
      else {
        lastByte += 0x6000;
        for (long currByte = 0x6000; currByte < lastByte; currByte++) {
          outputFile.writeByte(readBank_SNES(0x30, currByte));
        }
      }
    }
  }
  else if (romType == EX) {
    // Dumping SRAM on HiRom needs CS(PH3) to be high
    PORTH |=  (1 << 3);
    // Sram size
    long lastByte = (long(sramSize) * 128) + 0x6000;
    for (long currByte = 0x6000; currByte < lastByte; currByte++) {
      outputFile.writeByte(readBank_SNES(0xB0, currByte));
    }
  }
  else if (romType == SA) {
    // Dumping SRAM on HiRom needs CS(PH3) to be high
    PORTH |=  (1 << 3);
    // Sram size
    long lastByte = (long(sramSize) * 128);
    if (lastByte > 0x10000) {
      sramBanks = lastByte / 0x10000;
      for (int currBank = 0x40; currBank < sramBanks + 0x40; currBank++) {
        for (long currByte = 0; currByte < 0x10000; currByte++) {
          outputFile.writeByte(readBank_SNES(currBank, currByte));
        }
      }
    }
    else {
      for (long currByte = 0x0; currByte < lastByte; currByte++) {
        outputFile.writeByte(readBank_SNES(0x40, currByte));
      }
    }
  }

  // Close the file:
  outputFile.close();

  // Signal end of process
  ui.clearOutput();
  ui.printMsg(F("Saved to "));
  ui.printlnMsg(outputFilePath);
  ui.flushOutput();
}

// Check if the SRAM was written without any error
unsigned long verifySRAM(const String &filePath) {
  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(filePath);

  // Variable for errors
  writeErrors = 0;

  // Set control
  controlIn_SNES();

  int sramBanks = 0;
  if (romType == LO) {
    // Sram size
    long lastByte = (long(sramSize) * 128);
    if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) { // SuperFX
      if (lastByte > 0x10000) { // Large SuperFX SRAM (no known carts)
        sramBanks = lastByte / 0x10000;
        for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
          for (long currByte = 0; currByte < 0x10000; currByte += 512) {
            //fill sdBuffer
            inputFile.read(sdBuffer, 512);
            for (int c = 0; c < 512; c++) {
              if ((readBank_SNES(currBank, currByte + c)) != sdBuffer[c]) {
                writeErrors++;
              }
            }
          }
        }
      }
      else { // SuperFX SRAM
        for (long currByte = 0; currByte < lastByte; currByte += 512) {
          //fill sdBuffer
          inputFile.read(sdBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(0x70, currByte + c)) != sdBuffer[c]) {
              writeErrors++;
            }
          }
        }
      }
    }
    else if (lastByte > 0x8000) { // Large SRAM Fix
      sramBanks = lastByte / 0x8000;
      for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
        for (long currByte = 0; currByte < 0x8000; currByte += 512) {
          //fill sdBuffer
          inputFile.read(sdBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(currBank, currByte + c)) != sdBuffer[c]) {
              writeErrors++;
            }
          }
        }
      }
    }
    else {
      for (long currByte = 0; currByte < lastByte; currByte += 512) {
        //fill sdBuffer
        inputFile.read(sdBuffer, 512);
        for (int c = 0; c < 512; c++) {
          if ((readBank_SNES(0x70, currByte + c)) != sdBuffer[c]) {
            writeErrors++;
          }
        }
      }
    }
  }
  else if (romType == HI) {
    if ((romChips == 245) || (romChips == 249)) { // SPC7110 SRAM
      // Configure SPC7110 SRAM Register
      dataOut();
      // Set 0x4830 to 0x80
      writeBank_SNES(0, 0x4830, 0x80);
      dataIn();
      // Sram size
      long lastByte = (long(sramSize) * 128) + 0x6000;
      for (long currByte = 0x6000; currByte < lastByte; currByte += 512) {
        //fill sdBuffer
        inputFile.read(sdBuffer, 512);
        for (int c = 0; c < 512; c++) {
          if ((readBank_SNES(0x30, currByte + c)) != sdBuffer[c]) {
            writeErrors++;
          }
        }
      }
      dataOut();
      // Reset 0x4830 to 0x0
      writeBank_SNES(0, 0x4830, 0);
      dataIn();
    }
    else {
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |=  (1 << 3);
      // Sram size
      long lastByte = (long(sramSize) * 128);
      if (lastByte > 0x2000) { // Large SRAM Fix
        sramBanks = lastByte / 0x2000;
        for (int currBank = 0x30; currBank < sramBanks + 0x30; currBank++) {
          for (long currByte = 0x6000; currByte < 0x8000; currByte += 512) {
            //fill sdBuffer
            inputFile.read(sdBuffer, 512);
            for (int c = 0; c < 512; c++) {
              if ((readBank_SNES(currBank, currByte + c)) != sdBuffer[c]) {
                writeErrors++;
              }
            }
          }
        }
      }
      else {
        lastByte += 0x6000;
        for (long currByte = 0x6000; currByte < lastByte; currByte += 512) {
          //fill sdBuffer
          inputFile.read(sdBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(0x30, currByte + c)) != sdBuffer[c]) {
              writeErrors++;
            }
          }
        }
      }
    }
  }
  else if (romType == EX) {
    // Dumping SRAM on HiRom needs CS(PH3) to be high
    PORTH |=  (1 << 3);
    // Sram size
    long lastByte = (long(sramSize) * 128) + 0x6000;
    for (long currByte = 0x6000; currByte < lastByte; currByte += 512) {
      //fill sdBuffer
      inputFile.read(sdBuffer, 512);
      for (int c = 0; c < 512; c++) {
        if ((readBank_SNES(0xB0, currByte + c)) != sdBuffer[c]) {
          writeErrors++;
        }
      }
    }
  }
  else if (romType == SA) {
    // Dumping SRAM on HiRom needs CS(PH3) to be high
    PORTH |=  (1 << 3);
    // Sram size
    long lastByte = (long(sramSize) * 128);

    if (lastByte > 0x10000) {
      sramBanks = lastByte / 0x10000;
      for (int currBank = 0x40; currBank < sramBanks + 0x40; currBank++) {
        for (long currByte = 0x0; currByte < 0x10000; currByte += 512) {
          //fill sdBuffer
          inputFile.read(sdBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(currBank, currByte + c)) != sdBuffer[c]) {
              writeErrors++;
            }
          }
        }
      }
    }
    else {
      for (long currByte = 0x0; currByte < lastByte; currByte += 512) {
        //fill sdBuffer
        inputFile.read(sdBuffer, 512);
        for (int c = 0; c < 512; c++) {
          if ((readBank_SNES(0x40, currByte + c)) != sdBuffer[c]) {
            writeErrors++;
          }
        }
      }
    }
    // Reset SA1
    // Set pins to input
    dataIn();
    // Close the file:
    inputFile.close();
    if (writeErrors == 0) {
      ui.printlnMsg(F("Verified OK"));
    }
    else {
      ui.printMsg(F("Error: "));
      ui.printMsg(writeErrors);
      ui.printlnMsg(F(" bytes "));
      ui.printError(F("did not verify."));
    }
    ui.flushOutput();
    ui.waitForUserInput();

    stopSnesClocks_resetCic_resetCart();

    ui.clearOutput();
    ui.printMsg(F("Resetting..."));
    ui.flushOutput();
    delay(3000);  // wait 3 secs
    resetArduino();
  }
  // Close the file:
  inputFile.close();
  return writeErrors;
}

// Overwrite the entire SRAM
boolean eraseSRAM (byte b) {
  ui.printMsg(F("0x"));
  ui.printMsg(b, HEX);
  ui.printMsg(F(": "));
  ui.flushOutput();

  // Set pins to output
  dataOut();

  // Set control pins
  controlOut_SNES();

  int sramBanks = 0;
  if (romType == LO) {
    // Sram size
    long lastByte = (long(sramSize) * 128);

    if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) { // SuperFX
      if (lastByte > 0x10000) { // Large SuperFX SRAM (no known carts)
        sramBanks = lastByte / 0x10000;
        for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
          for (long currByte = 0x0000; currByte < 0x10000; currByte++) {
            writeBank_SNES(currBank, currByte, b);
          }
        }
      }
      else { // SuperFX SRAM
        for (long currByte = 0; currByte < lastByte; currByte++) {
          writeBank_SNES(0x70, currByte, b);
        }
      }
    }
    else if (lastByte > 0x8000) { // Large SRAM Fix
      sramBanks = lastByte / 0x8000;
      for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
        for (long currByte = 0x0000; currByte < 0x8000; currByte++) {
          writeBank_SNES(currBank, currByte, b);
        }
      }
    }
    else {
      for (long currByte = 0; currByte <  lastByte; currByte++) {
        writeBank_SNES(0x70, currByte, b);
      }
    }
  }
  else if (romType == HI) {
    if ((romChips == 245) || (romChips == 249)) { // SPC7110 SRAM
      // Configure SPC7110 SRAM Register
      // Set 0x4830 to 0x80
      writeBank_SNES(0, 0x4830, 0x80);
      // Sram size
      long lastByte = (long(sramSize) * 128) + 0x6000;
      // Write to sram bank
      for (long currByte = 0x6000; currByte < lastByte; currByte++) {
        writeBank_SNES(0x30, currByte, b);
      }
      // Reset SPC7110 SRAM Register
      dataOut();
      // Reset 0x4830 to 0x0
      writeBank_SNES(0, 0x4830, 0);
      dataIn();
    }
    else {
      // Writing SRAM on HiRom needs CS(PH3) to be high
      PORTH |=  (1 << 3);
      // Sram size
      long lastByte = (long(sramSize) * 128);
      if (lastByte > 0x2000) { // Large SRAM Fix
        sramBanks = lastByte / 0x2000;
        for (int currBank = 0x30; currBank < sramBanks + 0x30; currBank++) {
          for (long currByte = 0x6000; currByte < 0x8000; currByte++) {
            writeBank_SNES(currBank, currByte, b);
          }
        }
      }
      else {
        lastByte += 0x6000;
        // Write to sram bank
        for (long currByte = 0x6000; currByte < lastByte; currByte++) {
          writeBank_SNES(0x30, currByte, b);
        }
      }
    }
  }
  // ExHiRom
  else if (romType == EX) {
    // Writing SRAM on HiRom needs CS(PH3) to be high
    PORTH |=  (1 << 3);
    // Sram size
    long lastByte = (long(sramSize) * 128) + 0x6000;
    for (long currByte = 0x6000; currByte < lastByte; currByte++) {
      writeBank_SNES(0xB0, currByte, b);
    }
  }
  // SA1
  else if (romType == SA) {
    long lastByte = (long(sramSize) * 128);
    // Enable CPU Clock
    clockgen.output_enable(SI5351_CLK1, 1);

    // Direct writes to BW-RAM (SRAM) in banks 0x40-0x43 don't work
    // Break BW-RAM (SRAM) into 0x2000 blocks
    // Use $2224 to map BW-RAM block to 0x6000-0x7FFF
    byte lastBlock = 0;
    lastBlock = lastByte / 0x2000;

    // Writing SRAM on SA1 needs CS(PH3) to be high
    // PORTH |=  (1 << 3);

    // Setup BW-RAM
    // Set 0x2224 (SNES BMAPS) to map SRAM Block 0 to 0x6000-0x7FFF
    writeBank_SNES(0, 0x2224, 0);
    // Set 0x2226 (SNES SBWE) to 0x80 Write Enable
    writeBank_SNES(0, 0x2226, 0x80);
    // Set 0x2228 (SNES BWPA) to 0x00 BW-RAM Write-Protected Area
    writeBank_SNES(0, 0x2228, 0);
    delay(1000);

    // Use $2224 (SNES) to map BW-RAM block to 0x6000-0x7FFF
    // Use $2226 (SNES) to write enable the BW-RAM
    for (byte currBlock = 0; currBlock < lastBlock; currBlock++) {
      // Set 0x2224 (SNES BMAPS) to map SRAM Block to 0x6000-0x7FFF
      writeBank_SNES(0, 0x2224, currBlock);
      // Set 0x2226 (SNES SBWE) to 0x80 Write Enable
      writeBank_SNES(0, 0x2226, 0x80);
      for (long currByte = 0x6000; currByte < 0x8000; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          writeBank_SNES(0, currByte + c, b);
        }
      }
    }
    // Rewrite First Byte
    writeBank_SNES(0, 0x2224, 0);
    writeBank_SNES(0, 0x2226, 0x80);
    writeBank_SNES(0, 0x6000, b);
    // Disable CPU clock
    clockgen.output_enable(SI5351_CLK1, 0);
  }

  dataIn();

  // Variable for errors
  writeErrors = 0;

  // Set control
  controlIn_SNES();

  sramBanks = 0;
  if (romType == LO) {
    // Sram size
    long lastByte = (long(sramSize) * 128);
    if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) { // SuperFX
      if (lastByte > 0x10000) { // Large SuperFX SRAM (no known carts)
        sramBanks = lastByte / 0x10000;
        for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
          for (long currByte = 0; currByte < 0x10000; currByte += 512) {
            for (int c = 0; c < 512; c++) {
              if ((readBank_SNES(currBank, currByte + c)) != b) {
                writeErrors++;
              }
            }
          }
        }
      }
      else { // SuperFX SRAM
        for (long currByte = 0; currByte < lastByte; currByte += 512) {
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(0x70, currByte + c)) != b) {
              writeErrors++;
            }
          }
        }
      }
    }
    else if (lastByte > 0x8000) { // Large SRAM Fix
      sramBanks = lastByte / 0x8000;
      for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
        for (long currByte = 0; currByte < 0x8000; currByte += 512) {
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(currBank, currByte + c)) != b) {
              writeErrors++;
            }
          }
        }
      }
    }
    else {
      for (long currByte = 0; currByte < lastByte; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          if ((readBank_SNES(0x70, currByte + c)) != b) {
            writeErrors++;
          }
        }
      }
    }
  }
  else if (romType == HI) {
    if ((romChips == 245) || (romChips == 249)) { // SPC7110 SRAM
      // Configure SPC7110 SRAM Register
      dataOut();
      // Set 0x4830 to 0x80
      writeBank_SNES(0, 0x4830, 0x80);
      dataIn();
      // Sram size
      long lastByte = (long(sramSize) * 128) + 0x6000;
      for (long currByte = 0x6000; currByte < lastByte; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          if ((readBank_SNES(0x30, currByte + c)) != b) {
            writeErrors++;
          }
        }
      }
      dataOut();
      // Reset 0x4830 to 0x0
      writeBank_SNES(0, 0x4830, 0);
      dataIn();
    }
    else {
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |=  (1 << 3);
      // Sram size
      long lastByte = (long(sramSize) * 128);
      if (lastByte > 0x2000) { // Large SRAM Fix
        sramBanks = lastByte / 0x2000;
        for (int currBank = 0x30; currBank < sramBanks + 0x30; currBank++) {
          for (long currByte = 0x6000; currByte < 0x8000; currByte += 512) {
            for (int c = 0; c < 512; c++) {
              if ((readBank_SNES(currBank, currByte + c)) != b) {
                writeErrors++;
              }
            }
          }
        }
      }
      else {
        lastByte += 0x6000;
        for (long currByte = 0x6000; currByte < lastByte; currByte += 512) {
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(0x30, currByte + c)) != b) {
              writeErrors++;
            }
          }
        }
      }
    }
  }
  else if (romType == EX) {
    // Dumping SRAM on HiRom needs CS(PH3) to be high
    PORTH |=  (1 << 3);
    // Sram size
    long lastByte = (long(sramSize) * 128) + 0x6000;
    for (long currByte = 0x6000; currByte < lastByte; currByte += 512) {
      for (int c = 0; c < 512; c++) {
        if ((readBank_SNES(0xB0, currByte + c)) != b) {
          writeErrors++;
        }
      }
    }
  }
  else if (romType == SA) {
    // Dumping SRAM on HiRom needs CS(PH3) to be high
    PORTH |=  (1 << 3);
    // Sram size
    long lastByte = (long(sramSize) * 128);
    if (lastByte > 0x10000) {
      sramBanks = lastByte / 0x10000;
      for (int currBank = 0x40; currBank < sramBanks + 0x40; currBank++) {
        for (long currByte = 0x0; currByte < 0x10000; currByte += 512) {
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(currBank, currByte + c)) != b) {
              writeErrors++;
            }
          }
        }
      }
    }
    else {
      for (long currByte = 0x0; currByte < lastByte; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          if ((readBank_SNES(0x40, currByte + c)) != b) {
            writeErrors++;
          }
        }
      }
    }
  }
  if (writeErrors == 0) {
    ui.printlnMsg(F("OK"));
    return 1;
  }
  else {
    ui.printlnMsg(F("ERROR"));
    return 0;
  }
  ui.flushOutput();
}

//******************************************
// End of File
//******************************************
