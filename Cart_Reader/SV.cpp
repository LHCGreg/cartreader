//******************************************
// SNES Satellaview 8M Memory pack code by tamanegi_taro
// Revision 1.0.0 October 22nd 2018
// Added BSX Sram, copied from skamans enhanced sketch //sanni
//******************************************

#include <Arduino.h>
#include "SV.h"
#include "SNES.h"
#include "filebrowser.h"
#include "ui.h"
#include "globals.h"
#include "utils.h"
#include "SD.h"

/******************************************
   Satellaview 8M Memory Pack
******************************************/
/******************************************
   Prototype Declarations
 *****************************************/

void readROM_SV();
void writeROM_SV (void);
void eraseCheck_SV(void);
void supplyCheck_SV(void);
void writeCheck_SV(void);
void detectCheck_SV(void);
void eraseAll_SV(void);

/******************************************
   Variables
 *****************************************/
//No global variables

/******************************************
   Function prototypes
 *****************************************/
void readSRAM_SV();
void writeSRAM_SV(const String &inputfilePath);
unsigned long verifySRAM_SV(const String &filePath);

/******************************************
  Menu
*****************************************/
void svMenu() {
  while (true) {
    const __FlashStringHelper *item_ReadMemPack = F("Read Memory Pack");
    const __FlashStringHelper *item_WriteMemPack = F("Write Memory Pack");
    const __FlashStringHelper *item_ReadSRAM = F("Read BS-X Sram");
    const __FlashStringHelper *item_WriteSRAM = F("Write BS-X Sram");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_ReadMemPack,
      item_WriteMemPack,
      item_ReadSRAM,
      item_WriteSRAM,
      item_Back,
    };

    const __FlashStringHelper *answer = ui->askMultipleChoiceQuestion(
      F("Satellaview 8M Memory"), menu, ARRAY_LENGTH(menu), item_ReadMemPack);

    if (answer == item_ReadMemPack) {
      readROM_SV();
    }
    else if (answer == item_WriteMemPack) {
      writeROM_SV();
    }
    else if (answer == item_ReadSRAM) {
      readSRAM_SV();
    }
    else if (answer == item_WriteSRAM) {
      String inputFilePath = fileBrowser(F("Select srm file"));
      writeSRAM_SV(inputFilePath);
      uint32_t writeErrors = verifySRAM_SV(inputFilePath);
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
      ui->waitForUserInput();
    }
    else if (answer == item_Back) {
      break;
    }
  }
}

/******************************************
   Setup
 *****************************************/
void setup_SV() {
  // Set cicrstPin(PG1) to Output
  DDRG |= (1 << 1);
  // Output a high signal until we're ready to start
  PORTG |= (1 << 1);
  // Set cichstPin(PG0) to Input
  DDRG &= ~(1 << 0);

  // Adafruit Clock Generator
  clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
  clockgen.set_freq(2147727200ULL, SI5351_CLK0);
  clockgen.set_freq(307200000ULL, SI5351_CLK2);
  clockgen.output_enable(SI5351_CLK0, 1);
  clockgen.output_enable(SI5351_CLK1, 0);
  clockgen.output_enable(SI5351_CLK2, 1);

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

  // Set expand(PG5) to output
  DDRG |= (1 << 5);
  // Output High
  PORTG |= (1 << 5);

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

  // Start CIC by outputting a low signal to cicrstPin(PG1)
  PORTG  &= ~(1 << 1);

  // Wait for CIC reset
  delay(1000);
}

/******************************************
   Low level functions
 *****************************************/
// Write one byte of data to a location specified by bank and address, 00:0000
void writeBank_SV(byte myBank, word myAddress, byte myData) {
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
byte readBank_SV(byte myBank, word myAddress) {
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
   SatellaView BS-X Sram functions
 *****************************************/
void readSRAM_SV () {
  // set control
  controlIn_SNES();

  String outputFilePath = getNextOutputPathWithNumberedFolder(F("SNES"), F("SAVE"), F("BSX"), F(".srm"));

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  readBank_SV(0x10, 0); // Preconfigure to fix corrupt 1st byte

  //startBank = 0x10; endBank = 0x17; CS low
  for (byte BSBank = 0x10; BSBank < 0x18; BSBank++) {
    //startAddr = 0x5000
    for (long currByte = 0x5000; currByte < 0x6000; currByte += 512) {
      for (unsigned long c = 0; c < 512; c++) {
        sdBuffer[c] = readBank_SV(BSBank, currByte + c);
      }
      outputFile.write(sdBuffer, 512);
    }
  }
  // Close the file:
  outputFile.close();

  // Signal end of process
  ui->clearOutput();
  ui->printMsg(F("Saved to "));
  ui->printMsg(outputFilePath);
  ui->printlnMsg(F("..."));
  ui->flushOutput();
  ui->waitForUserInput();
}

void writeSRAM_SV(const String &inputFilePath) {
  //clear the screen
  ui->clearOutput();

  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

  // Set pins to output
  dataOut();

  // Set RST RD WR to High and CS to Low
  controlOut_SNES();

  ui->printlnMsg(F("Writing sram..."));
  ui->flushOutput();

  // Write to sram bank
  for (byte currBank = 0x10; currBank < 0x18; currBank++) {
    //startAddr = 0x5000
    for (long currByte = 0x5000; currByte < 0x6000; currByte += 512) {
      inputFile.read(sdBuffer, 512);
      for (unsigned long c = 0; c < 512; c++) {
        //startBank = 0x10; CS low
        writeBank_SV(currBank, currByte + c, sdBuffer[c]);
      }
    }
    ui->drawProgressBar(((currBank - 0x10) * 0x1000), 32768);
  }
  // Finish progressbar
  ui->drawProgressBar(32768, 32768);
  delay(100);
  // Set pins to input
  dataIn();

  // Close the file:
  inputFile.close();
  ui->printlnMsg("");
  ui->printlnMsg(F("SRAM writing finished"));
  ui->flushOutput();
}

// Check if the SRAM was written without any error
unsigned long verifySRAM_SV(const String &filePath) {
  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(filePath);

  // Variable for errors
  writeErrors = 0;

  // Set control
  controlIn_SNES();

  //startBank = 0x10; endBank = 0x17; CS low
  for (byte BSBank = 0x10; BSBank < 0x18; BSBank++) {
    //startAddr = 0x5000
    for (long currByte = 0x5000; currByte < 0x6000; currByte += 512) {
      //fill sdBuffer
      inputFile.read(sdBuffer, 512);
      for (unsigned long c = 0; c < 512; c++) {
        if ((readBank_SV(BSBank, currByte + c)) != sdBuffer[c]) {
          writeErrors++;
        }
      }
    }
  }
  // Close the file:
  inputFile.close();
  return writeErrors;
}

/******************************************
   SatellaView 8M Memory Pack functions
 *****************************************/
// Read memory pack to SD card
void readROM_SV() {
  // Set control
  dataIn();
  controlIn_SNES();

  String outputFilePath = getNextOutputPathWithNumberedFolderAndPrintMessage(F("SNES"), F("ROM"), F("MEMPACK"), F(".bs"));

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  // Read Banks
  for (int currBank = 0x40; currBank < 0x50; currBank++) {
    // Dump the bytes to SD 512B at a time
    for (long currByte = 0; currByte < 65536; currByte += 512) {
      ui->drawProgressBar((currBank - 0x40) * 0x10000 + currByte, 0x100000);
      for (int c = 0; c < 512; c++) {
        sdBuffer[c] = readBank_SV(currBank, currByte + c);
      }
      outputFile.write(sdBuffer, 512);
    }
  }
  ui->drawProgressBar(0x100000, 0x100000); //Finish drawing progress bar

  // Close the file:
  outputFile.close();
  ui->printlnMsg(F("Read pack completed"));
  ui->flushOutput();
  ui->waitForUserInput();
}

void writeROM_SV(void) {
  // Get Checksum as string to make sure that BS-X cart is inserted
  dataIn();
  controlIn_SNES();
  sprintf(checksumStr, "%02X%02X", readBank_SV(0, 65503), readBank_SV(0, 65502));

  //if CRC is not 8B86, BS-X cart is not inserted. Display error and reset
  if (strcmp("8B86", checksumStr) != 0)
  {
    ui->printErrorAndAbort(F("Error: Must use BS-X cart"), true);
  }

  //Display file Browser and wait user to select a file. Size must be 1MB.
  String inputFilePath = fileBrowser(F("Select BS file"));

  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

  fileSize = inputFile.fileSize();
  if (fileSize != 0x100000) {
    ui->printlnMsg(F("File must be 1MB"));
    ui->flushOutput();
    inputFile.close();
    ui->waitForUserInput();
    return;
  }

  //Disable 8M memory pack write protection
  dataOut();
  controlOut_SNES();
  writeBank_SV(0x0C, 0x5000, 0x80); //Modify write enable register
  writeBank_SV(0x0E, 0x5000, 0x80); //Commit register modification

  //Erase memory pack
  ui->printlnMsg(F("Erasing pack..."));
  ui->flushOutput();
  eraseAll_SV();

  //Blank check
  //Set pins to input
  dataIn();
  controlIn_SNES();
  ui->printlnMsg(F("Blank check..."));
  ui->flushOutput();
  for (int currBank = 0xC0; currBank < 0xD0; currBank++) {
    ui->drawProgressBar(((currBank - 0xC0) * 0x10000), 0x100000);
    for (long currByte = 0; currByte < 65536; currByte++) {
      if (0xFF != readBank_SV(currBank, currByte))
      {
        ui->printlnMsg(F(""));
        ui->printlnMsg(F("Erase failed"));
        ui->flushOutput();
        inputFile.close();
        ui->waitForUserInput();
        return;
      }
    }
  }
  ui->drawProgressBar(0x100000, 0x100000);

  //Write memory pack
  dataOut();
  controlOut_SNES();
  ui->printlnMsg(F("Writing pack..."));
  ui->flushOutput();
  for (int currBank = 0xC0; currBank < 0xD0; currBank++) {
    ui->drawProgressBar(((currBank - 0xC0) * 0x10000), 0x100000);
    for (long currByte = 0; currByte < 65536; currByte++) {

      writeBank_SV(0xC0, 0x0000, 0x10); //Program Byte
      writeBank_SV(currBank, currByte, inputFile.readByteOrDie());
      writeBank_SV(0xC0, 0x0000, 0x70); //Status Mode
      writeCheck_SV();
    }
  }

  writeBank_SV(0xC0, 0x0000, 0x70); //Status Mode
  writeCheck_SV();
  writeBank_SV(0xC0, 0x0000, 0xFF); //Terminate write
  ui->drawProgressBar(0x100000, 0x100000);


  //Verify
  dataIn();    //Set pins to input
  controlIn_SNES();
  inputFile.seekSet(0);    // Go back to file beginning
  ui->printlnMsg(F("Verifying..."));
  ui->flushOutput();
  for (int currBank = 0xC0; currBank < 0xD0; currBank++) {
    ui->drawProgressBar(((currBank - 0xC0) * 0x10000), 0x100000);
    for (long currByte = 0; currByte < 65536; currByte++) {
      if (inputFile.readByteOrDie() != readBank_SV(currBank, currByte))
      {
        ui->printlnMsg(F(""));
        ui->printlnMsg(F("Verify failed"));
        ui->flushOutput();
        inputFile.close();
        ui->waitForUserInput();
        return;
      }
    }
  }

  // Close the file:
  inputFile.close();
  ui->drawProgressBar(0x100000, 0x100000);
  ui->printlnMsg(F("Finished successfully"));
  ui->flushOutput();
  ui->waitForUserInput();
}

void eraseCheck_SV(void) {
  byte ret;
  dataIn();
  controlIn_SNES();

  // Read register
  ret = readBank_SV(0xC0, 0x0004);

  // CE or OE must be toggled with each subsequent status read or the
  // completion of a program or erase operation will not be evident.
  while ((ret & 0x80) == 0x00) { //Wait until X.bit7 = 1
    controlOut_SNES();
    // Switch CS(PH3) High
    PORTH |= (1 << 3);
    // Leave CE high for at least 60ns
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    controlIn_SNES();
    // Leave CE low for at least 50ns
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    // Read register
    ret = readBank_SV(0xC0, 0x0004);
  }
  // Switch to write
  dataOut();
  controlOut_SNES();
}

void supplyCheck_SV(void) {
  byte ret;
  dataIn();
  controlIn_SNES();

  // Read register
  ret = readBank_SV(0xC0, 0x0004);

  // CE or OE must be toggled with each subsequent status read or the
  // completion of a program or erase operation will not be evident.
  while ((ret & 0x08) == 0x08) { //Wait until X.bit3 = 0
    controlOut_SNES();
    // Switch CS(PH3) High
    PORTH |= (1 << 3);
    // Leave CE high for at least 60ns
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    controlIn_SNES();
    // Leave CE low for at least 50ns
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    // Read register
    ret = readBank_SV(0xC0, 0x0004);
  }
  // Switch to write
  dataOut();
  controlOut_SNES();
}

void writeCheck_SV(void) {
  byte ret;
  dataIn();
  controlIn_SNES();

  // Read register
  ret = readBank_SV(0xC0, 0x0000);

  // CE or OE must be toggled with each subsequent status read or the
  // completion of a program or erase operation will not be evident.
  while ((ret & 0x80) == 0x00) {
    controlOut_SNES();
    // Switch CS(PH3) High
    PORTH |= (1 << 3);
    // Leave CE high for at least 60ns
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    controlIn_SNES();
    // Leave CE low for at least 50ns
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    // Read register
    ret = readBank_SV(0xC0, 0x0000);
  }
  // Switch to write
  dataOut();
  controlOut_SNES();
}


void detectCheck_SV(void) {
  int i = 0;
  byte ret;
  dataIn();
  controlIn_SNES();

  // Read register
  ret = readBank_SV(0xC0, 0x0002);

  // CE or OE must be toggled with each subsequent status read or the
  // completion of a program or erase operation will not be evident.
  while ((ret & 0x80) == 0x00) {
    i++;
    if ( i > 10000)
    {
      //timeout
      break;
    }
    controlOut_SNES();
    // Switch CS(PH3) High
    PORTH |= (1 << 3);
    // Leave CE high for at least 60ns
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    controlIn_SNES();
    // Leave CE low for at least 50ns
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    // Read register
    ret = readBank_SV(0xC0, 0x0002);
  }
  // Switch to write
  dataOut();
  controlOut_SNES();
}


void eraseAll_SV(void)
{
  dataOut();
  controlOut_SNES();
  writeBank_SV(0xC0, 0x0000, 0x50); //Clear Status Registers
  writeBank_SV(0xC0, 0x0000, 0x71); //Status Mode
  supplyCheck_SV();
  writeBank_SV(0xC0, 0x0000, 0xA7); //Chip Erase
  writeBank_SV(0xC0, 0x0000, 0xD0); //Confirm
  writeBank_SV(0xC0, 0x0000, 0x71); //Status Mode
  eraseCheck_SV();
  writeBank_SV(0xC0, 0x0000, 0xFF); //Teriminate
}

//******************************************
// End of File
//******************************************
