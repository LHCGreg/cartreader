//******************************************
// SEGA MASTER SYSTEM MODULE
//******************************************

#include <Arduino.h>
#include "SMS.h"
#include "RGB_LED.h"
#include "ui.h"
#include "globals.h"
#include "utils.h"
#include "filebrowser.h"
#include "SD.h"

/******************************************
   Variables
 *****************************************/

/******************************************
   Function prototypes
 *****************************************/
void setup_SMS();
void writeByte_SMS(word myAddress, byte myData);
void getCartInfo_SMS();
void readROM_SMS();

/******************************************
   Menu
 *****************************************/
void smsMenu() {
  while (true) {
    const __FlashStringHelper *item_Read = F("Read Rom");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_Read,
      item_Back,
    };

    const __FlashStringHelper *answer = ui->askMultipleChoiceQuestion(
      F("Sega Master System"), menu, ARRAY_LENGTH(menu), item_Read);

    if (answer == item_Read) {
      ui->clearOutput();
      mode = CartReaderMode::SMS;
      setup_SMS();
      readROM_SMS();
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

/******************************************
   Setup
 *****************************************/
void setup_SMS() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A14
  DDRK = 0xFF;
  //A15
  DDRH |= (1 << 3);

  // Set Control Pins to Output RST(PH0) WR(PH5) OE(PH6)
  DDRH |= (1 << 0) | (1 << 5) | (1 << 6);
  // CE(PL1)
  DDRL |= (1 << 1);

  // Setting RST(PH0) WR(PH5) OE(PH6) HIGH
  PORTH |= (1 << 0) | (1 << 5) | (1 << 6);
  // CE(PL1)
  PORTL |= (1 << 1);

  // ROM has 16KB banks which can be mapped to one of three slots via register writes
  // Register Slot Address space
  // $fffd    0    $0000-$3fff
  // $fffe    1    $4000-$7fff
  // $ffff    2    $8000-$bfff
  // Disable sram
  writeByte_SMS(0xFFFC, 0);
  // Map first 3 banks so we can read-out the header info
  writeByte_SMS(0xFFFD, 0);
  writeByte_SMS(0xFFFE, 1);
  writeByte_SMS(0xFFFF, 2);

  delay(400);

  // Print all the info
  getCartInfo_SMS();
}

/******************************************
  Low level functions
*****************************************/
void writeByte_SMS(word myAddress, byte myData) {
  // Set Data Pins (D0-D7) to Output
  DDRC = 0xFF;

  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTH = (PORTH & 0b11110111) | ((myAddress >> 12) & 0b00001000);
  // Output data
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch CE(PL1) and WR(PH5) to LOW
  PORTL &= ~(1 << 1);
  PORTH &= ~(1 << 5);

  // Leave WE low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch CE(PL1) and WR(PH5) to HIGH
  PORTH |= (1 << 5);
  PORTL |= (1 << 1);

  // Leave WE high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
}

byte readByte_SMS(word myAddress) {
  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;

  // Set Address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTH = (PORTH & 0b11110111) | ((myAddress >> 12) & 0b00001000);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch CE(PL1) and OE(PH6) to LOW
  PORTL &= ~(1 << 1);
  PORTH &= ~(1 << 6);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  byte tempByte = PINC;

  // Switch CE(PL1) and OE(PH6) to HIGH
  PORTH |= (1 << 6);
  PORTL |= (1 << 1);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempByte;
}

//unsigned char hex2bcd (unsigned char x) {
//  unsigned char y;
//  y = (x / 10) << 4;
//  y = y | (x % 10);
//return (y);
//}

byte readNibble(byte data, byte number) {
  return ((data >> (number * 4)) & 0xf);
}

/******************************************
  MASTER SYSTEM functions
*****************************************/
void getCartInfo_SMS() {
  // Rom size
  switch (readNibble(readByte_SMS(0x7fff), 0)) {
    case 0xa:
      // Adding UL gets rid of integer overflow compiler warning
      cartSize =  8 * 1024UL;
      break;
    case 0xb:
      cartSize =  16 * 1024UL;
      break;
    case 0xc:
      cartSize =  32 * 1024UL;
      break;
    case 0xd:
      cartSize =  48 * 1024UL;
      break;
    case 0xe:
      cartSize =  64 * 1024UL;
      break;
    case 0xf:
      cartSize =  128 * 1024UL;
      break;
    case 0x0:
      cartSize =  256 * 1024UL;
      break;
    case 0x1:
      cartSize =  512 * 1024UL;
      break;
    case 0x2:
      cartSize =  512 * 1024UL;
      break;
    default:
      cartSize =  48 * 1024UL;
      // LED Error
      rgb.setColor(0, 0, 255);
      break;

  }

  // Read TMR Sega string
  for (byte i = 0; i < 8; i++) {
    romName[i] = char(readByte_SMS(0x7ff0 + i));
  }
  romName[8] = '\0';

  ui->clearOutput();
  ui->printlnMsg(F("Cart Info"));
  ui->printlnMsg(F(" "));
  ui->printMsg(F("Name: "));
  ui->printlnMsg(romName);
  ui->printMsg(F("Size: "));
  ui->printMsg(cartSize / 1024);
  ui->printlnMsg(F("KB"));
  ui->printlnMsg(F(" "));

  if (strcmp(romName, "TMR SEGA") != 0) {
    ui->printError(F("Not working yet"));
    sprintf(romName, "ERROR");
    cartSize =  48 * 1024UL;
  }

  // Wait for user input
  ui->printlnMsg(F("Press Button..."));
  ui->flushOutput();
  ui->waitForUserInput();
  // Turn off LED
  rgb.setColor(0, 0, 0);
}

// Read rom and save to the SD card
void readROM_SMS() {
  String outputFilePath = getNextOutputPathWithNumberedFolderAndPrintMessage(F("SMS"), F("ROM"), romName, F(".SMS"));

  // Open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);
  word bankSize = 16 * 1024UL;
  for (byte currBank = 0x0; currBank < (cartSize / bankSize); currBank++) {
    // Write current 16KB bank to slot 2 register 0xFFFF
    writeByte_SMS(0xFFFF, currBank);

    // Blink led
    PORTB ^= (1 << 4);
    // Read 16KB from slot 2 which starts at 0x8000
    for (word currBuffer = 0; currBuffer < bankSize; currBuffer += 512) {
      // Fill SD buffer
      for (int currByte = 0; currByte < 512; currByte++) {
        sdBuffer[currByte] = readByte_SMS(0x8000 + currBuffer + currByte);
      }
      outputFile.write(sdBuffer, 512);
    }
  }
  // Close the file:
  outputFile.close();
}

//******************************************
// End of File
//******************************************
