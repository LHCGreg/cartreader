//******************************************
// NINTENDO 64 MODULE
//******************************************

#include <Arduino.h>
#include "N64.h"
#include "filebrowser.h"
#include "RGB_LED.h"
#include "ui.h"
#include "globals.h"
#include "utils.h"
#include "SD.h"

/******************************************
  Defines
 *****************************************/
// These two macros toggle the eepDataPin/ControllerDataPin between input and output
// External 1K pull-up resistor from eepDataPin to VCC required
// 0x10 = 00010000 -> Port H Pin 4
#define N64_HIGH DDRH &= ~0x10
#define N64_LOW DDRH |= 0x10
// Read the current state(0/1) of the eepDataPin
#define N64_QUERY (PINH & 0x10)

/******************************************
   Variables
 *****************************************/
// Received N64 Eeprom data bits, 1 page
bool tempBits[65];
int eepPages;

// N64 Controller
// 256 bits of received Controller data
char N64_raw_dump[257];
// Array that holds one Controller Pak block of 32 bytes
byte myBlock[33];
String rawStr = ""; // above char array read into a string
struct {
  char stick_x;
  char stick_y;
}
N64_status;
//stings that hold the buttons
String button = "N/A";
String lastbutton = "N/A";

// Rom base address
unsigned long romBase = 0x10000000;

// Flashram type
byte flashramType = 1;
boolean MN63F81MPN = false;

//ControllerTest
bool quit = 1;

/******************************************
   Function prototypes
 *****************************************/
void setup_N64_Controller();
void setup_N64_Cart();
void controllerTest();
void readMPK();
void writeMPK(const String &inputFilePath);
void verifyMPK(const String &filePath);
void printCartInfo_N64();
void getCartInfo_N64();
void idCart();
void writeEeprom(const String &inputFilePath);
void readEeprom();
unsigned long verifyEeprom(const String &filePath);
void writeSram(const String &inputFilePath, unsigned long sramSize);
void readSram(unsigned long sramSize, byte flashramType);
unsigned long verifySram(const String &FilePath, unsigned long sramSize, byte flashramType);
void writeFram(const String &inputFilePath, byte flashramType);
void eraseFram();
void readFram(byte flashramType);
unsigned long verifyFram(const String &filePath, byte flashramType);
unsigned long blankcheck_N64(byte flashramType);
byte waitForFram(byte flashramType);
void getFramType();
void readRom_N64();
void flashRepro_N64();
void resetIntel4400_N64();
void idFlashrom_N64();
void eraseIntel4400_N64();
void eraseMSP55LV100_N64();
void eraseFlashrom_N64(unsigned long sectorSize);
boolean blankcheckFlashrom_N64();
void writeIntel4400_N64(SafeSDFile &inputFile);
void writeMSP55LV100_N64(SafeSDFile &inputFile, unsigned long sectorSize);
void writeFlashBuffer_N64(SafeSDFile &inputFile, unsigned long sectorSize, byte bufferSize);
void writeFlashrom_N64(SafeSDFile &inputFile);
unsigned long verifyFlashrom_N64(const String &filePath);
void flashGameshark_N64();
void idGameshark_N64();
void resetGameshark_N64();
void backupGameshark_N64();
void eraseGameshark_N64();
void writeGameshark_N64(SafeSDFile &inputFile);
unsigned long verifyGameshark_N64(const String &filePath);

/******************************************
  Menu
*****************************************/

// N64 start menu
void n64Menu() {
  while (true) {
    const __FlashStringHelper *item_Cart = F("Game Cartridge");
    const __FlashStringHelper *item_Controller = F("Controller");
    const __FlashStringHelper *item_FlashRepro = F("Flash Repro");
    const __FlashStringHelper *item_FlashGameshark = F("Flash Gameshark");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_Cart,
      item_Controller,
      item_FlashRepro,
      item_FlashGameshark,
      item_Back,
    };

    const __FlashStringHelper *answer = ui->askMultipleChoiceQuestion(
      F("Select N64 device"), menu, ARRAY_LENGTH(menu), item_Cart);

    if (answer == item_Cart) {
      ui->clearOutput();
      ui->flushOutput();
      setup_N64_Cart();
      printCartInfo_N64();
      mode = CartReaderMode::N64Cart;
      n64CartMenu();
    }
    else if (answer == item_Controller) {
      ui->clearOutput();
      ui->flushOutput();
      setup_N64_Controller();
      mode = CartReaderMode::N64Controller;
      n64ControllerMenu();
    }
    else if (answer == item_FlashRepro) {
      ui->clearOutput();
      ui->flushOutput();
      setup_N64_Cart();
      flashRepro_N64();
      printCartInfo_N64();
      mode = CartReaderMode::N64Cart;
      n64CartMenu();
    }
    else if (answer == item_FlashGameshark) {
      ui->clearOutput();
      ui->flushOutput();
      setup_N64_Cart();
      flashGameshark_N64();
      printCartInfo_N64();
      mode = CartReaderMode::N64Cart;
      n64CartMenu();
    }
    else if (answer == item_Back) {
      break;
    }
  }
}

// N64 Controller Menu
void n64ControllerMenu() {
  while (true) {
    const __FlashStringHelper *item_TestController = F("Test Controller");
    const __FlashStringHelper *item_ReadControllerPak = F("Read ControllerPak");
    const __FlashStringHelper *item_WriteControllerPak = F("Write ControllerPak");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_TestController,
      item_ReadControllerPak,
      item_WriteControllerPak,
      item_Back,
    };

    const __FlashStringHelper *answer = ui->askMultipleChoiceQuestion(
      F("N64 Controller"), menu, ARRAY_LENGTH(menu), item_TestController);

    if (answer == item_TestController) {
      ui->clearOutput();
      ui->flushOutput();
      controllerTest();
      quit = 1;
    }
    else if (answer == item_ReadControllerPak) {
      ui->clearOutput();
      ui->flushOutput();
      readMPK();
      ui->printlnMsg(F(""));
      ui->printlnMsg(F("Press Button."));
      ui->flushOutput();
      ui->waitForUserInput();
    }
    else if (answer == item_WriteControllerPak) {
      ui->clearOutput();
      ui->flushOutput();
      // Launch file browser
      String inputFilePath = fileBrowser(F("Select mpk file"));
      ui->clearOutput();
      ui->flushOutput();
      writeMPK(inputFilePath);
      verifyMPK(inputFilePath);
      ui->printlnMsg(F(""));
      ui->printlnMsg(F("Press Button."));
      ui->flushOutput();
      ui->waitForUserInput();
    }
    else if (answer == item_Back) {
      break;
    }
  }
}

// N64 Cartridge Menu
void n64CartMenu() {
  while (true) {
    const __FlashStringHelper *item_ReadROM = F("Read Rom");
    const __FlashStringHelper *item_ReadSave = F("Read Save");
    const __FlashStringHelper *item_WriteSave = F("Write Save");
    const __FlashStringHelper *item_ForceSaveType = F("Force Savetype");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_ReadROM,
      item_ReadSave,
      item_WriteSave,
      item_ForceSaveType,
      item_Back,
    };

    const __FlashStringHelper *answer = ui->askMultipleChoiceQuestion(
      F("N64 Cart Reader"), menu, ARRAY_LENGTH(menu), item_ReadROM);

    if (answer == item_ReadROM) {
      readRom_N64();
    }
    else if (answer == item_ReadSave) {
      ui->clearOutput();

      if (saveType == 1) {
        ui->printlnMsg(F("Reading Sram..."));
        ui->flushOutput();
        readSram(32768, 1);
      }
      else if (saveType == 4) {
        getFramType();
        ui->printlnMsg(F("Reading Flashram..."));
        ui->flushOutput();
        readFram(flashramType);
      }
      else if ((saveType == 5) || (saveType == 6)) {
        ui->printlnMsg(F("Reading Eep..."));
        ui->flushOutput();
        readEeprom();
      }
      else {
        ui->printError(F("Savetype Error"));
      }
      ui->printlnMsg(F(""));
      ui->printlnMsg(F("Press Button..."));
      ui->flushOutput();
      ui->waitForUserInput();
    }
    else if (answer == item_WriteSave) {
      if (saveType == 1) {
        // Launch file browser
        String inputFilePath = fileBrowser(F("Select sra file"));
        ui->clearOutput();

        writeSram(inputFilePath, 32768);
        uint32_t writeErrors = verifySram(inputFilePath, 32768, 1);
        if (writeErrors == 0) {
          ui->printlnMsg(F("Sram verified OK"));
          ui->flushOutput();
        }
        else {
          ui->printMsg(F("Error: "));
          ui->printMsg(writeErrors);
          ui->printlnMsg(F(" bytes "));
          ui->printError(F("did not verify."));
        }
      }
      else if (saveType == 4) {
        // Launch file browser
        String inputFilePath = fileBrowser(F("Select fla file"));
        ui->clearOutput();
        getFramType();
        writeFram(inputFilePath, flashramType);
        ui->printMsg(F("Verifying..."));
        ui->flushOutput();
        uint32_t writeErrors = verifyFram(inputFilePath, flashramType);
        if (writeErrors == 0) {
          ui->printlnMsg(F("OK"));
          ui->flushOutput();
        }
        else {
          ui->printlnMsg("");
          ui->printMsg(F("Error: "));
          ui->printMsg(writeErrors);
          ui->printlnMsg(F(" bytes "));
          ui->printError(F("did not verify."));
        }
      }
      else if ((saveType == 5) || (saveType == 6)) {
        // Launch file browser
        String inputFilePath = fileBrowser(F("Select eep file"));
        ui->clearOutput();

        writeEeprom(inputFilePath);
        uint32_t writeErrors = verifyEeprom(inputFilePath);
        if (writeErrors == 0) {
          ui->printlnMsg(F("Eeprom verified OK"));
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
        ui->clearOutput();
        ui->printError(F("Savetype Error"));
      }
      ui->printlnMsg(F("Press Button..."));
      ui->flushOutput();
      ui->waitForUserInput();
    }
    else if (answer == item_ForceSaveType) {
      const __FlashStringHelper *saveItem_None = F("None");
      const __FlashStringHelper *saveItem_4KEEPROM = F("4K EEPROM");
      const __FlashStringHelper *saveItem_16KEEPROM = F("16K EEPROM");
      const __FlashStringHelper *saveItem_SRAM = F("SRAM");
      const __FlashStringHelper *saveItem_Flash = F("FLASHRAM");
      const __FlashStringHelper *saveMenu[] = {
        saveItem_None,
        saveItem_4KEEPROM,
        saveItem_16KEEPROM,
        saveItem_SRAM,
        saveItem_Flash,
      };

      const __FlashStringHelper *saveAnswer = ui->askMultipleChoiceQuestion(
        F("Select save type"), saveMenu, ARRAY_LENGTH(saveMenu), saveItem_None);

      if (saveAnswer == saveItem_None) {
        saveType = 0;
      }
      else if (saveAnswer == saveItem_4KEEPROM) {
        saveType = 5;
        eepPages = 64;
      }
      else if (saveAnswer == saveItem_16KEEPROM) {
        saveType = 6;
        eepPages = 256;
      }
      else if (saveAnswer == saveItem_SRAM) {
        saveType = 1;
      }
      else if (saveAnswer == saveItem_Flash) {
        saveType = 4;
      }
    }
    else if (answer == item_Back) {
      break;
    }
  }
}

String getNextN64MPKOutputPath() {
  int16_t folderNumber = loadFolderNumber();

  String fileName = String(folderNumber);
  fileName.concat(F(".mpk"));

  String outputFilePath = F("/N64/MPK/");
  outputFilePath.concat(fileName);

  saveFolderNumber(folderNumber + 1);

  return outputFilePath;
}

String getNextN64EepomOutputPath(const String &gameName) {
  return getNextOutputPath(F("N64"), F("SAVE"), gameName, F(".eep"));
}

String getNextN64SramOutputPath(const String &gameName, uint8_t saveType) {
  const __FlashStringHelper *extension;
  if (saveType == 4) {
    extension = F(".fla");
  }
  else if (saveType == 1) {
    extension = F(".sra");
  }
  else {
    ui->printErrorAndAbort(F("Savetype Error"), true);
  }

  return getNextOutputPath(F("N64"), F("SAVE"), gameName, extension);
}

String getNextN64RomOutputPathAndPrintMessage(const String &gameName) {
  return getNextOutputPathAndPrintMessage(F("N64"), F("ROM"), gameName, F(".Z64"));
}

String getNextN64GamesharkBackupOutputPathAndPrintMessage() {
  int16_t folderNumber = loadFolderNumber();

  String fileName = F("GS");
  fileName.concat(folderNumber);
  fileName.concat(F(".z64"));

  String outputFilePath = F("/N64/ROM/Gameshark/");
  outputFilePath.concat(fileName);

  saveFolderNumber(folderNumber + 1);

  ui->clearOutput();
  ui->printMsg(F("Saving to "));
  ui->printMsg(outputFilePath);
  ui->printlnMsg(F("..."));
  ui->flushOutput();

  return outputFilePath;
}

/******************************************
   Setup
 *****************************************/
void setup_N64_Controller() {
  // Output a low signal
  PORTH &= ~(1 << 4);
  // Set Controller Data Pin(PH4) to Input
  DDRH &= ~(1 << 4);
}

void setup_N64_Cart() {
  // Set Address Pins to Output and set them low
  //A0-A7
  DDRF = 0xFF;
  PORTF = 0x00;
  //A8-A15
  DDRK = 0xFF;
  PORTK = 0x00;

  // Set Control Pins to Output RESET(PH0) WR(PH5) RD(PH6) aleL(PC0) aleH(PC1)
  DDRH |= (1 << 0) | (1 << 5) | (1 << 6);
  DDRC |= (1 << 0) | (1 << 1);
  // Pull RESET(PH0) low until we are ready
  PORTH &= ~(1 << 0);
  // Output a high signal on WR(PH5) RD(PH6), pins are active low therefore everything is disabled now
  PORTH |= (1 << 5) | (1 << 6);
  // Pull aleL(PC0) low and aleH(PC1) high
  PORTC &= ~(1 << 0);
  PORTC |= (1 << 1);

  // Set Eeprom Clock Pin(PH1) to Output
  DDRH |= (1 << 1);
  // Output a high signal
  PORTH |= (1 << 1);

  // Set Eeprom Data Pin(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  //PORTH |= (1 << 4);

  // Set sram base address
  sramBase = 0x08000000;

  // Wait until all is stable
  delay(300);

  // Pull RESET(PH0) high to start eeprom
  PORTH |= (1 << 0);
}

/******************************************
   Low level functions
 *****************************************/
// Switch Cartridge address/data pins to write
void adOut_N64() {
  //A0-A7
  DDRF = 0xFF;
  PORTF = 0x00;
  //A8-A15
  DDRK = 0xFF;
  PORTK = 0x00;
}

// Switch Cartridge address/data pins to read
void adIn_N64() {
  //A0-A7
  DDRF = 0x00;
  //A8-A15
  DDRK = 0x00;
}

// Set Cartridge address
void setAddress_N64(unsigned long myAddress) {
  // Set address pins to output
  adOut_N64();

  // Split address into two words
  word myAdrLowOut = myAddress & 0xFFFF;
  word myAdrHighOut = myAddress >> 16;

  // Switch WR(PH5) RD(PH6) ale_L(PC0) ale_H(PC1) to high (since the pins are active low)
  PORTH |= (1 << 5) | (1 << 6);
  PORTC |= (1 << 1);
  __asm__("nop\n\t");
  PORTC |= (1 << 0);

  // Output high part to address pins
  PORTF = myAdrHighOut & 0xFF;
  PORTK = (myAdrHighOut >> 8) & 0xFF;

  // Leave ale_H high for additional 62.5ns
  __asm__("nop\n\t");

  // Pull ale_H(PC1) low
  PORTC &= ~(1 << 1);

  // Output low part to address pins
  PORTF = myAdrLowOut & 0xFF;
  PORTK = (myAdrLowOut >> 8) & 0xFF;

  // Leave ale_L high for ~125ns
  __asm__("nop\n\t""nop\n\t");

  // Pull ale_L(PC0) low
  PORTC &= ~(1 << 0);

  // Wait ~600ns just to be sure address is set
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Set data pins to input
  adIn_N64();
}

// Read one word out of the cartridge
word readWord_N64() {
  // Pull read(PH6) low
  PORTH &= ~(1 << 6);

  // Wait ~310ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Join bytes from PINF and PINK into a word
  word tempWord = ( ( PINK & 0xFF ) << 8 ) | ( PINF & 0xFF );

  // Pull read(PH6) high
  PORTH |= (1 << 6);

  // Wait 62.5ns
  __asm__("nop\n\t");
  return tempWord;
}

// Write one word to data pins of the cartridge
void writeWord_N64(word myWord) {
  // Set address pins to output
  adOut_N64();

  // Output word to AD0-AD15
  PORTF = myWord & 0xFF;
  PORTK = (myWord >> 8) & 0xFF;

  // Wait ~62.5ns
  __asm__("nop\n\t");

  // Pull write(PH5) low
  PORTH &= ~(1 << 5);

  // Wait ~310ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull write(PH5) high
  PORTH |= (1 << 5);

  // Wait ~125ns
  __asm__("nop\n\t""nop\n\t");

  // Set data pins to input
  adIn_N64();
}

/******************************************
   N64 Controller CRC Functions
 *****************************************/
static word addrCRC(word address) {
  // CRC table
  word xor_table[16] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x15, 0x1F, 0x0B, 0x16, 0x19, 0x07, 0x0E, 0x1C, 0x0D, 0x1A, 0x01 };
  word crc = 0;
  // Make sure we have a valid address
  address &= ~0x1F;
  // Go through each bit in the address, and if set, xor the right value into the output
  for (int i = 15; i >= 5; i--) {
    // Is this bit set?
    if ( ((address >> i) & 0x1)) {
      crc ^= xor_table[i];
    }
  }
  // Just in case
  crc &= 0x1F;
  // Create a new address with the CRC appended
  return address | crc;
}

// unused
//static byte dataCRC(byte * data) {
//  byte ret = 0;
//  for (byte i = 0; i <= 32; i++) {
//    for (byte j = 7; j >= 0; j--) {
//      int tmp = 0;
//      if (ret & 0x80) {
//        tmp = 0x85;
//      }
//      ret <<= 1;
//      if ( i < 32 ) {
//        if (data[i] & (0x01 << j)) {
//          ret |= 0x1;
//        }
//      }
//      ret ^= tmp;
//    }
//  }
//  return ret;
//}

/******************************************
   N64 Controller Protocol Functions
 *****************************************/
void N64_send(unsigned char *buffer, char length) {
  // Send these bytes
  char bits;

  // This routine is very carefully timed by examining the assembly output.
  // Do not change any statements, it could throw the timings off
  //
  // We get 16 cycles per microsecond, which should be plenty, but we need to
  // be conservative. Most assembly ops take 1 cycle, but a few take 2
  //
  // I use manually constructed for-loops out of gotos so I have more control
  // over the outputted assembly. I can insert nops where it was impossible
  // with a for loop

  asm volatile (";Starting outer for loop");
outer_loop:
  {
    asm volatile (";Starting inner for loop");
    bits = 8;
inner_loop:
    {
      // Starting a bit, set the line low
      asm volatile (";Setting line to low");
      N64_LOW; // 1 op, 2 cycles

      asm volatile (";branching");
      if (*buffer >> 7) {
        asm volatile (";Bit is a 1");
        // 1 bit
        // remain low for 1us, then go high for 3us
        // nop block 1
        asm volatile ("nop\nnop\nnop\nnop\nnop\n");

        asm volatile (";Setting line to high");
        N64_HIGH;

        // nop block 2
        // we'll wait only 2us to sync up with both conditions
        // at the bottom of the if statement
        asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                     );

      }
      else {
        asm volatile (";Bit is a 0");
        // 0 bit
        // remain low for 3us, then go high for 1us
        // nop block 3
        asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\n");

        asm volatile (";Setting line to high");
        N64_HIGH;

        // wait for 1us
        asm volatile ("; end of conditional branch, need to wait 1us more before next bit");

      }
      // end of the if, the line is high and needs to remain
      // high for exactly 16 more cycles, regardless of the previous
      // branch path

      asm volatile (";finishing inner loop body");
      --bits;
      if (bits != 0) {
        // nop block 4
        // this block is why a for loop was impossible
        asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\n");
        // rotate bits
        asm volatile (";rotating out bits");
        *buffer <<= 1;

        goto inner_loop;
      } // fall out of inner loop
    }
    asm volatile (";continuing outer loop");
    // In this case: the inner loop exits and the outer loop iterates,
    // there are /exactly/ 16 cycles taken up by the necessary operations.
    // So no nops are needed here (that was lucky!)
    --length;
    if (length != 0) {
      ++buffer;
      goto outer_loop;
    } // fall out of outer loop
  }
}

void N64_stop() {
  // send a single stop (1) bit
  // nop block 5
  asm volatile ("nop\nnop\nnop\nnop\n");
  N64_LOW;
  // wait 1 us, 16 cycles, then raise the line
  // 16-2=14
  // nop block 6
  asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\n");
  N64_HIGH;
}

void N64_get(word bitcount) {
  // listen for the expected bitcount/8 bytes of data back from the controller and
  // blast it out to the N64_raw_dump array, one bit per byte for extra speed.
  asm volatile (";Starting to listen");
  unsigned char timeout;
  char *bitbin = N64_raw_dump;

  // Again, using gotos here to make the assembly more predictable and
  // optimization easier (please don't kill me)
read_loop:
  timeout = 0x3f;
  // wait for line to go low
  while (N64_QUERY) {
    if (!--timeout)
      return;
  }
  // wait approx 2us and poll the line
  asm volatile (
    "nop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\n"
  );
  *bitbin = N64_QUERY;
  ++bitbin;
  --bitcount;
  if (bitcount == 0)
    return;

  // wait for line to go high again
  // it may already be high, so this should just drop through
  timeout = 0x3f;
  while (!N64_QUERY) {
    if (!--timeout)
      return;
  }
  goto read_loop;
}

/******************************************
   N64 Controller Functions
 *****************************************/
void get_button()
{
  // Command to send to the gamecube
  // The last bit is rumble, flip it to rumble
  // yes this does need to be inside the loop, the
  // array gets mutilated when it goes through N64_send
  unsigned char command[] = {
    0x01
  };

  // don't want interrupts getting in the way
  noInterrupts();
  // send those 3 bytes
  N64_send(command, 1);
  N64_stop();
  // read in 32bits of data and dump it to N64_raw_dump
  N64_get(32);
  // end of time sensitive code
  interrupts();

  // The get_N64_status function sloppily dumps its data 1 bit per byte
  // into the get_status_extended char array. It's our job to go through
  // that and put each piece neatly into the struct N64_status
  int i;
  memset(&N64_status, 0, sizeof(N64_status));

  // bits: joystick x value
  // These are 8 bit values centered at 0x80 (128)
  for (i = 0; i < 8; i++) {
    N64_status.stick_x |= N64_raw_dump[16 + i] ? (0x80 >> i) : 0;
  }
  for (i = 0; i < 8; i++) {
    N64_status.stick_y |= N64_raw_dump[24 + i] ? (0x80 >> i) : 0;
  }

  // read char array N64_raw_dump into string rawStr
  rawStr = "";
  for (i = 0; i < 16; i++) {
    rawStr = rawStr + String(N64_raw_dump[i], DEC);
  }

  // Buttons (A,B,Z,S,DU,DD,DL,DR,0,0,L,R,CU,CD,CL,CR)
  if (rawStr.substring(0, 16) == "0000000000000000") {
    lastbutton = button;
    button = String();
  }
  else
  {
    for (int i = 0; i < 16; i++)
    {
      // seems to be 16, 8 or 4 depending on what pin is used
      if (N64_raw_dump[i] == 16)
      {
        switch (i)
        {
          case 7:
            button = F("D-Right");
            break;

          case 6:
            button = F("D-Left");
            break;

          case 5:
            button = F("D-Down");
            break;

          case 4:
            button = F("D-Up");
            break;

          case 3:
            button = F("START");
            break;

          case 2:
            button = F("Z");
            break;

          case 1:
            button = F("B");
            break;

          case 0:
            button = F("A");
            break;

          case 15:
            button = F("C-Right");
            break;

          case 14:
            button = F("C-Left");
            break;

          case 13:
            button = F("C-Down");
            break;

          case 12:
            button = F("C-Up");
            break;

          case 11:
            button = F("R");
            break;

          case 10:
            button = F("L");
            break;
        }
      }
    }
  }
}


/******************************************
  N64 Controller Test
 *****************************************/
void controllerTest() {
  // on which screens do we start
  int startscreen = 1;
  int mode = 0;
  int test = 1;

  //name of the current displayed result
  String anastick = "";
  int prevStickX = 0;

  // variables to display test data of different sticks
  int upx = 0;
  int upy = 0;
  int uprightx = 0;
  int uprighty = 0;
  int rightx = 0;
  int righty = 0;
  int downrightx = 0;
  int downrighty = 0;
  int downx = 0;
  int downy = 0;
  int downleftx = 0;
  int downlefty = 0;
  int leftx = 0;
  int lefty = 0;
  int upleftx = 0;
  int uplefty = 0;

  // variables to save test data
  int bupx = 0;
  int bupy = 0;
  int buprightx = 0;
  int buprighty = 0;
  int brightx = 0;
  int brighty = 0;
  int bdownrightx = 0;
  int bdownrighty = 0;
  int bdownx = 0;
  int bdowny = 0;
  int bdownleftx = 0;
  int bdownlefty = 0;
  int bleftx = 0;
  int blefty = 0;
  int bupleftx = 0;
  int buplefty = 0;
  int results = 0;

  while (quit) {
    // Get Button and analog stick
    get_button();

    switch (startscreen)
    {
      case 1:
        {
          ui->updateN64ButtonTest(button, N64_status.stick_x, N64_status.stick_y);

          // go to next screen
          if (button.length() == 0 && lastbutton == F("START"))
          {
            // reset button
            lastbutton = F("N/A");

            ui->clearOutput();
            if (ui->supportsN64RangeTest()) {
              startscreen = 2;
            }
            else if (ui->supportsN64SkippingTest()) {
              startscreen = 3;
            }
            else {
              startscreen = 4;
            }
          }

          break;
        }
      case 2:
        {
          ui->updateN64RangeTest(N64_status.stick_x, N64_status.stick_y, mode);

          // switch mode
          if (button.length() == 0 && lastbutton == F("Z"))
          {
            if (mode == 0)
            {
              mode = 1;
              ui->clearOutput();
            }
            else
            {
              mode = 0;
              ui->clearOutput();
            }
          }

          // go to next screen
          if (button.length() == 0 && lastbutton == F("START"))
          {
            // reset button
            lastbutton = F("N/A");

            ui->clearOutput();
            if (ui->supportsN64SkippingTest()) {
              startscreen = 3;
            }
            else {
              startscreen = 4;
            }
          }

          break;
        }
      case 3:
        {
          ui->updateN64SkippingTest(prevStickX, N64_status.stick_x);

          if (N64_status.stick_x > 0) {
            prevStickX = N64_status.stick_x;
          }

          if (button.length() == 0 && lastbutton == F("Z")) {
            // reset button
            lastbutton = F("N/A");

            ui->clearOutput();
          }
          // go to next screen
          if (button.length() == 0 && lastbutton == F("START")) {
            // reset button
            lastbutton = F("N/A");

            ui->clearOutput();
          }

          break;
        }
      case 4:
        {
          switch ( test )
          {
            case 0:  // Display results
              {
                switch (results)
                {
                  case 0:
                    {
                      anastick = F("YOURS");
                      upx = bupx;
                      upy = bupy;
                      uprightx = buprightx;
                      uprighty = buprighty;
                      rightx = brightx;
                      righty = brighty;
                      downrightx = bdownrightx;
                      downrighty = bdownrighty;
                      downx = bdownx;
                      downy = bdowny;
                      downleftx = bdownleftx;
                      downlefty = bdownlefty;
                      leftx = bleftx;
                      lefty = blefty;
                      upleftx = bupleftx;
                      uplefty = buplefty;

                      if (button.length() == 0 && lastbutton == F("A"))
                      {
                        // reset button
                        lastbutton = F("N/A");
                        results = 1;
                      }

                      break;
                    }
                  case 1:
                    {
                      anastick = F("ORIG");
                      upx = 1;
                      upy = 84;
                      uprightx = 67;
                      uprighty = 68;
                      rightx = 83;
                      righty = -2;
                      downrightx = 67;
                      downrighty = -69;
                      downx = 3;
                      downy = -85;
                      downleftx = -69;
                      downlefty = -70;
                      leftx = -85;
                      lefty = 0;
                      upleftx = -68;
                      uplefty = 68;

                      if (button.length() == 0 && lastbutton == F("A"))
                      {
                        // reset button
                        lastbutton = F("N/A");
                        results = 0;
                      }
                      break;
                    }

                } //results
                ui->printN64BenchmarkResults(anastick, upx, upy, uprightx, uprightx, rightx, righty, downrightx, downrighty,
                                             downx, downy, downleftx, downlefty, leftx, lefty, upleftx, uplefty);
                
                break;
              } //display results

            case 1:// +y Up
              {
                ui->printN64BenchmarkPrompt(test);

                if (button.length() == 0 && lastbutton == F("A"))
                {
                  bupx = N64_status.stick_x;
                  bupy = N64_status.stick_y;
                  // reset button
                  lastbutton = F("N/A");

                  ui->clearOutput();
                  test = 2;
                }
                break;
              }

            case 2:// +y+x Up-Right
              {
                ui->printN64BenchmarkPrompt(test);

                if (button.length() == 0 && lastbutton == F("A"))
                {
                  buprightx = N64_status.stick_x;
                  buprighty = N64_status.stick_y;
                  test = 3;
                  // reset button
                  lastbutton = F("N/A");

                  ui->clearOutput();
                }
                break;
              }

            case 3:// +x Right
              {
                ui->printN64BenchmarkPrompt(test);

                if (button.length() == 0 && lastbutton == F("A"))
                {
                  brightx = N64_status.stick_x;
                  brighty = N64_status.stick_y;
                  test = 4;
                  // reset button
                  lastbutton = F("N/A");

                  ui->clearOutput();
                }
                break;
              }

            case 4:// -y+x Down-Right
              {
                ui->printN64BenchmarkPrompt(test);

                if (button.length() == 0 && lastbutton == F("A"))
                {
                  bdownrightx = N64_status.stick_x;
                  bdownrighty = N64_status.stick_y;
                  test = 5;
                  // reset button
                  lastbutton = F("N/A");

                  ui->clearOutput();
                }
                break;
              }

            case 5:// -y Down
              {
                ui->printN64BenchmarkPrompt(test);

                if (button.length() == 0 && lastbutton == F("A"))
                {
                  bdownx = N64_status.stick_x;
                  bdowny = N64_status.stick_y;
                  test = 6;
                  // reset button
                  lastbutton = F("N/A");

                  ui->clearOutput();
                }
                break;
              }

            case 6:// -y-x Down-Left
              {
                ui->printN64BenchmarkPrompt(test);

                if (button.length() == 0 && lastbutton == F("A"))
                {
                  bdownleftx = N64_status.stick_x;
                  bdownlefty = N64_status.stick_y;
                  test = 7;
                  // reset button
                  lastbutton = F("N/A");

                  ui->clearOutput();
                }
                break;
              }

            case 7:// -x Left
              {
                ui->printN64BenchmarkPrompt(test);

                if (button.length() == 0 && lastbutton == F("A"))
                {
                  bleftx = N64_status.stick_x;
                  blefty = N64_status.stick_y;
                  test = 8;
                  // reset button
                  lastbutton = F("N/A");

                  ui->clearOutput();
                }
                break;
              }

            case 8:// +y+x Up-Left
              {
                ui->printN64BenchmarkPrompt(test);

                if (button.length() == 0 && lastbutton == F("A"))
                {
                  bupleftx = N64_status.stick_x;
                  buplefty = N64_status.stick_y;
                  test = 0;
                  // reset button
                  lastbutton = F("N/A");

                  ui->clearOutput();
                }
                break;
              }
          }
          ui->flushOutput();
          // go to next screen
          if (button.length() == 0 && lastbutton == F("START"))
          {
            // reset button
            lastbutton = F("N/A");

            ui->clearOutput();

            startscreen = 1;
            test = 1;
          }
          else if (button.length() == 0 && lastbutton == F("Z"))
          {
            // Quit
            quit = 0;
          }
          break;
        }
    }
  }
}
/******************************************
   N64 Controller Pak Functions
   (connected via Controller)
 *****************************************/
// read 32bytes from controller pak
void readBlock(word myAddress) {
  // Calculate the address CRC
  word myAddressCRC = addrCRC(myAddress);

  // Read Controller Pak command
  unsigned char command[] = {0x02};
  // Address Command
  unsigned char addressHigh[] = {(unsigned char)(myAddressCRC >> 8)};
  unsigned char addressLow[] = {(unsigned char)(myAddressCRC & 0xff)};

  // don't want interrupts getting in the way
  noInterrupts();
  // send those 3 bytes
  N64_send(command, 1);
  N64_send(addressHigh, 1);
  N64_send(addressLow, 1);
  N64_stop();
  // read in data
  N64_get(256);
  // end of time sensitive code
  interrupts();

  // Empty N64_raw_dump into myBlock
  for (word i = 0; i < 256; i += 8) {
    boolean byteFlipped[9];

    // Flip byte order
    byteFlipped[0] = N64_raw_dump[i + 7];
    byteFlipped[1] = N64_raw_dump[i + 6];
    byteFlipped[2] = N64_raw_dump[i + 5];
    byteFlipped[3] = N64_raw_dump[i + 4];
    byteFlipped[4] = N64_raw_dump[i + 3];
    byteFlipped[5] = N64_raw_dump[i + 2];
    byteFlipped[6] = N64_raw_dump[i + 1];
    byteFlipped[7] = N64_raw_dump[i + 0];

    // Join bits into one byte
    unsigned char myByte = 0;
    for (byte j = 0; j < 8; ++j) {
      if (byteFlipped[j]) {
        myByte |= 1 << j;
      }
    }
    // Save byte into block array
    myBlock[i / 8] = myByte;
  }
}

// reads the MPK file to the sd card
void readMPK() {
  String outputFilePath = getNextN64MPKOutputPath();

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  ui->printlnMsg(F("Please wait..."));
  ui->flushOutput();

  // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
  for (word i = 0x0000; i < 0x8000; i += 32) {
    // Read one block of the Controller Pak into array myBlock
    readBlock(i);
    // Write block to SD card
    outputFile.write(myBlock, 32);
  }
  // Close the file:
  outputFile.close();
  ui->printMsg(F("Saved as "));
  ui->printlnMsg(outputFilePath);
  ui->flushOutput();
}

void writeMPK(const String &inputFilePath) {
  ui->printlnMsg(F("Writing..."));
  ui->printlnMsg(inputFilePath);
  ui->flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  for (word myAddress = 0x0000; myAddress < 0x8000; myAddress += 32) {
    // Read 32 bytes into SD buffer
    inputFile.read(sdBuffer, 32);

    // Calculate the address CRC
    word myAddressCRC = addrCRC(myAddress);

    // Write Controller Pak command
    unsigned char command[] = {0x03};
    // Address Command
    unsigned char addressHigh[] = {(unsigned char)(myAddressCRC >> 8)};
    unsigned char addressLow[] = {(unsigned char)(myAddressCRC & 0xff)};

    // don't want interrupts getting in the way
    noInterrupts();
    // Send write command
    N64_send(command, 1);
    // Send block number
    N64_send(addressHigh, 1);
    N64_send(addressLow, 1);
    // Send data to write
    N64_send(sdBuffer, 32);
    // Send stop
    N64_stop();
    // Enable interrupts
    interrupts();
  }
  // Close the file:
  inputFile.close();
  ui->printlnMsg(F("Done"));
  ui->flushOutput();
}

// verifies if write was successful
void verifyMPK(const String &filePath) {
  writeErrors = 0;

  ui->printlnMsg(F("Verifying..."));
  ui->flushOutput();

  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(filePath);

  // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
  for (word i = 0x0000; i < 0x8000; i += 32) {
    // Read one block of the Controller Pak into array myBlock
    readBlock(i);
    // Check against file on SD card
    inputFile.readOrDie(sdBuffer, 32);
    for (byte j = 0; j < 32; j++) {
      if (sdBuffer[j] != myBlock[j]) {
        writeErrors++;
      }
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

/******************************************
  N64 Cartridge functions
*****************************************/
void printCartInfo_N64() {
  // Check cart
  getCartInfo_N64();

  // Print start page
  if (cartSize != 0) {
    ui->printlnMsg(F("N64 Cartridge Info"));
    ui->printlnMsg(F(""));
    ui->printMsg(F("Name: "));
    ui->printlnMsg(romName);
    ui->printMsg(F("ID: "));
    ui->printMsg(cartID);
    ui->printMsg(F(" Size: "));
    ui->printMsg(cartSize);
    ui->printlnMsg(F("MB"));
    ui->printMsg(F("Save: "));
    switch (saveType) {
      case 1:
        ui->printlnMsg(F("Sram"));
        break;
      case 4:
        ui->printlnMsg(F("Flashram"));
        break;
      case 5:
        ui->printlnMsg(F("4K Eeprom"));
        eepPages = 64;
        break;
      case 6:
        ui->printlnMsg(F("16K Eeprom"));
        eepPages = 256;
        break;
      default:
        ui->printlnMsg(F("unknown"));
        break;
    }
    ui->printMsg(F("Version: 1."));
    ui->printlnMsg(romVersion);

    // Wait for user input
    ui->printlnMsg(F(" "));
    ui->printlnMsg(F("Press Button..."));
    ui->flushOutput();
    ui->waitForUserInput();
  }
  else {
    // Display error
    ui->printlnMsg(F("GAMEPAK ERROR"));
    ui->printlnMsg("");
    ui->printMsg(F("Name: "));
    ui->printlnMsg(romName);
    ui->printMsg(F("ID: "));
    ui->printlnMsg(cartID);
    ui->printlnMsg("");
    ui->flushOutput();

    strcpy(romName, "GPERROR");
    ui->printError(F("Cartridge unknown"));
    ui->waitForUserInput();

    // Set cartsize manually
    // Set cartsize manually
    const __FlashStringHelper *romItem_4MB = F("4MB");
    const __FlashStringHelper *romItem_8MB = F("8MB");
    const __FlashStringHelper *romItem_12MB = F("12MB");
    const __FlashStringHelper *romItem_16MB = F("16MB");
    const __FlashStringHelper *romItem_32MB = F("32MB");
    const __FlashStringHelper *romItem_64MB = F("64MB");
    const __FlashStringHelper *romMenu[] = {
      romItem_4MB,
      romItem_8MB,
      romItem_12MB,
      romItem_16MB,
      romItem_32MB,
      romItem_64MB,
    };

    const __FlashStringHelper *romAnswer = ui->askMultipleChoiceQuestion(
      F("Select ROM size"), romMenu, ARRAY_LENGTH(romMenu), romItem_4MB);

    if (romAnswer == romItem_4MB) {
      cartSize = 4;
    }
    else if (romAnswer == romItem_8MB) {
      cartSize = 8;
    }
    else if (romAnswer == romItem_12MB) {
      cartSize = 12;
    }
    else if (romAnswer == romItem_16MB) {
      cartSize = 16;
    }
    else if (romAnswer == romItem_32MB) {
      cartSize = 32;
    }
    else if (romAnswer == romItem_64MB) {
      cartSize = 64;
    }
  }
}

// improved strcmp function that ignores case to prevent checksum comparison issues
int strcicmp(char const * a, char const * b)
{
  for (;; a++, b++) {
    int d = tolower((unsigned char) * a) - tolower((unsigned char) * b);
    if (d != 0 || !*a)
      return d;
  }
}

// look-up the calculated crc in the file n64.txt on sd card
boolean searchCRC(char crcStr[9]) {
  boolean result = 0;
  char tempStr2[2];
  char tempStr1[9];
  char tempStr[5];

  SafeSDFile n64File = SafeSDFile::openForReading(F("/n64.txt"));
  // Loop through file
  while (n64File.bytesAvailable() > 0) {
    // Read 8 bytes into String, do it one at a time so byte order doesn't get mixed up
    sprintf(tempStr1, "%c", n64File.readByteOrDie());
    for (byte i = 0; i < 7; i++) {
      sprintf(tempStr2, "%c", n64File.readByteOrDie());
      strcat(tempStr1, tempStr2);
    }

    // Check if string is a match
    if (strcicmp(tempStr1, crcStr) == 0) {
      // Skip the , in the file
      n64File.seekCur(1);

      // Read 4 bytes into String, do it one at a time so byte order doesn't get mixed up
      sprintf(tempStr, "%c", n64File.readByteOrDie());
      for (byte i = 0; i < 3; i++) {
        sprintf(tempStr2, "%c", n64File.readByteOrDie());
        strcat(tempStr, tempStr2);
      }

      if (strcmp(tempStr, cartID) == 0) {
        result = 1;
        break;
      }
      else {
        result = 0;
        break;
      }
    }
    // If no match, empty string, advance by 12 and try again
    else {
      n64File.seekCur(12);
    }
  }
  // Close the file:
  n64File.close();
  return result;
}

// look-up cart id in file n64.txt on sd card
void getCartInfo_N64() {
  char tempStr2[2];
  char tempStr[5];

  // cart not in list
  cartSize = 0;
  saveType = 0;

  // Read cart id
  idCart();

  SafeSDFile n64File = SafeSDFile::openForReading(F("/n64.txt"));
  // Skip over the first crc
  n64File.seekCur(9);
  // Loop through file
  while (n64File.bytesAvailable() > 0) {
    // Read 4 bytes into String, do it one at a time so byte order doesn't get mixed up
    sprintf(tempStr, "%c", n64File.readByteOrDie());
    for (byte i = 0; i < 3; i++) {
      sprintf(tempStr2, "%c", n64File.readByteOrDie());
      strcat(tempStr, tempStr2);
    }

    // Check if string is a match
    if (strcmp(tempStr, cartID) == 0) {
      // Skip the , in the file
      n64File.seekCur(1);

      // Read the next ascii character and subtract 48 to convert to decimal
      cartSize = n64File.readByteOrDie() - 48;
      // Remove leading 0 for single digit cart sizes
      if (cartSize != 0) {
        cartSize = cartSize * 10 +  n64File.readByteOrDie() - 48;
      }
      else {
        cartSize = n64File.readByteOrDie() - 48;
      }

      // Skip the , in the file
      n64File.seekCur(1);

      // Read the next ascii character and subtract 48 to convert to decimal
      saveType = n64File.readByteOrDie() - 48;
    }
    // If no match, empty string, advance by 16 and try again
    else {
      n64File.seekCur(16);
    }
  }
  // Close the file:
  n64File.close();
}

// Read rom ID
void idCart() {
  // Set the address
  setAddress_N64(romBase);
  // Read first 64 bytes of rom
  for (int c = 0; c < 64; c += 2) {
    // split word
    word myWord = readWord_N64();
    byte loByte = myWord & 0xFF;
    byte hiByte = myWord >> 8;

    // write to buffer
    sdBuffer[c] = hiByte;
    sdBuffer[c + 1] = loByte;
  }

  // Get cart id
  cartID[0] = sdBuffer[0x3B];
  cartID[1] = sdBuffer[0x3C];
  cartID[2] = sdBuffer[0x3D];
  cartID[3] = sdBuffer[0x3E];

  // Get rom version
  romVersion = sdBuffer[0x3F];

  // Get name
  byte myLength = 0;
  for (unsigned int i = 0; i < 20; i++) {
    if (((char(sdBuffer[0x20 + i]) >= 48 && char(sdBuffer[0x20 + i]) <= 57) || (char(sdBuffer[0x20 + i]) >= 65 && char(sdBuffer[0x20 + i]) <= 122)) && myLength < 15) {
      romName[myLength] = char(sdBuffer[0x20 + i]);
      myLength++;
    }
  }
}

/******************************************
  Eeprom functions
*****************************************/
// Send one byte of data to eeprom
void sendData(byte data) {
  for (byte i = 0; i < 8; i++) {
    // pull data line low
    N64_LOW;

    // if current bit is 1, pull high after ~1us, 2 cycles
    if (data >> 7) {
      pulseClock_N64(2);
      N64_HIGH;
      pulseClock_N64(6);
    }
    // if current bit is 0 pull high after ~3us, 6 cycles
    else {
      pulseClock_N64(6);
      N64_HIGH;
      pulseClock_N64(2);
    }

    // rotate to the next bit
    data <<= 1;
  }
}

// Send stop bit to eeprom
void sendStop() {
  N64_LOW;
  pulseClock_N64(2);
  N64_HIGH;
  pulseClock_N64(4);
}

// Capture 8 bytes in 64 bits into bit array tempBits
void readData() {
  for (byte i = 0; i < 64; i++) {

    // pulse clock until we get response from eeprom
    while (N64_QUERY) {
      pulseClock_N64(1);
    }

    // Skip over the 1us low part of a high bit
    pulseClock_N64(3);

    // Read bit
    tempBits[i] = N64_QUERY;

    // wait for line to go high again
    while (!N64_QUERY) {
      pulseClock_N64(1);
    }
  }
}

// Write Eeprom to cartridge
void writeEeprom(const String &inputFilePath) {
  if ((saveType == 5) || (saveType == 6)) {
    ui->printlnMsg(F("Writing..."));
    ui->printlnMsg(inputFilePath);
    ui->flushOutput();

    // Open file on sd card
    SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

    for (byte i = 0; i < (eepPages / 64); i++) {
      inputFile.read(sdBuffer, 512);
      // Disable interrupts for more uniform clock pulses
      noInterrupts();

      for (byte pageNumber = 0; pageNumber < 64; pageNumber++) {
        // Blink led
        PORTB ^= (1 << 4);

        // Wait ~50ms between page writes or eeprom will have write errors
        pulseClock_N64(26000);

        // Send write command
        sendData(0x05);
        // Send page number
        sendData(pageNumber + (i * 64));
        // Send data to write
        for (byte j = 0; j < 8; j++) {
          sendData(sdBuffer[(pageNumber * 8) + j]);
        }
        sendStop();
      }
      interrupts();
    }

    // Close the file:
    inputFile.close();
    ui->printlnMsg(F("Done"));
    ui->flushOutput();
    delay(600);
  }
  else {
    ui->printErrorAndAbort(F("Savetype Error"), false);
  }
}

// Dump Eeprom to SD
void readEeprom() {
  if ((saveType == 5) || (saveType == 6)) {

    // Wait 50ms or eeprom might lock up
    pulseClock_N64(26000);

    String outputFilePath = getNextN64EepomOutputPath(romName);

    // Open file on sd card
    SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

    for (byte i = 0; i < (eepPages / 64); i++) {
      // Disable interrupts for more uniform clock pulses
      noInterrupts();

      for (byte pageNumber = 0; pageNumber < 64; pageNumber++) {
        // Blink led
        PORTB ^= (1 << 4);

        // Send read command
        sendData(0x04);
        // Send Page number
        sendData(pageNumber + (i * 64));
        // Send stop bit
        sendStop();

        // read data
        readData();
        sendStop();

        // OR 8 bits into one byte for a total of 8 bytes
        for (byte j = 0; j < 64; j += 8) {
          sdBuffer[(pageNumber * 8) + (j / 8)] = tempBits[0 + j] << 7 | tempBits[1 + j] << 6 | tempBits[2 + j] << 5 | tempBits[3 + j] << 4 | tempBits[4 + j] << 3 | tempBits[5 + j] << 2 | tempBits[6 + j] << 1 | tempBits[7 + j];
        }
        // Wait 50ms between pages or eeprom might lock up
        pulseClock_N64(26000);
      }
      interrupts();

      // Write 64 pages at once to the SD card
      outputFile.write(sdBuffer, 512);
    }
    // Close the file:
    outputFile.close();
    //clear the screen
    ui->clearOutput();
    ui->printMsg(F("Saved to "));
    ui->printlnMsg(outputFilePath);
    ui->flushOutput();
  }
  else {
    ui->printErrorAndAbort(F("Savetype Error"), false);
  }
}

// Check if a write succeeded, returns 0 if all is ok and number of errors if not
unsigned long verifyEeprom(const String &filePath) {
  if ((saveType == 5) || (saveType == 6)) {
    writeErrors = 0;

    // Wait 50ms or eeprom might lock up
    pulseClock_N64(26000);

    ui->clearOutput();
    ui->printMsg(F("Verifying against "));
    ui->printlnMsg(filePath);
    ui->flushOutput();

    // Open file on sd card
    SafeSDFile inputFile = SafeSDFile::openForReading(filePath);

    for (byte i = 0; i < (eepPages / 64); i++) {
      // Disable interrupts for more uniform clock pulses
      noInterrupts();

      for (byte pageNumber = 0; pageNumber < 64; pageNumber++) {
        // Blink led
        PORTB ^= (1 << 4);

        // Send read command
        sendData(0x04);
        // Send Page number
        sendData(pageNumber + (i * 64));
        // Send stop bit
        sendStop();

        // read data
        readData();
        sendStop();

        // OR 8 bits into one byte for a total of 8 bytes
        for (byte j = 0; j < 64; j += 8) {
          sdBuffer[(pageNumber * 8) + (j / 8)] = tempBits[0 + j] << 7 | tempBits[1 + j] << 6 | tempBits[2 + j] << 5 | tempBits[3 + j] << 4 | tempBits[4 + j] << 3 | tempBits[5 + j] << 2 | tempBits[6 + j] << 1 | tempBits[7 + j];
        }
        // Wait 50ms between pages or eeprom might lock up
        pulseClock_N64(26000);
      }
      interrupts();

      // Check sdBuffer content against file on sd card
      for (int c = 0; c < 512; c++) {
        if (inputFile.readByteOrDie() != sdBuffer[c]) {
          writeErrors++;
        }
      }
    }
    // Close the file:
    inputFile.close();
    
    // Return 0 if verified ok, or number of errors
    return writeErrors;
  }
  else {
    ui->printErrorAndAbort(F("Savetype Error"), false);
  }
}

/******************************************
  SRAM functions
*****************************************/
// Write sram to cartridge
void writeSram(const String &inputFilePath, unsigned long sramSize) {
  if (saveType == 1) {
    ui->printlnMsg(F("Writing..."));
    ui->printlnMsg(inputFilePath);
    ui->flushOutput();

    // Open file on sd card
    SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
    for (unsigned long currByte = sramBase; currByte < (sramBase + sramSize); currByte += 512) {

      // Read save from SD into buffer
      inputFile.read(sdBuffer, 512);

      // Set the address for the next 512 bytes
      setAddress_N64(currByte);

      for (int c = 0; c < 512; c += 2) {
        // Join bytes to word
        word myWord = ( ( sdBuffer[c] & 0xFF ) << 8 ) | ( sdBuffer[c + 1] & 0xFF );

        // Write word
        writeWord_N64(myWord);
      }
    }
    // Close the file:
    inputFile.close();
    ui->printlnMsg(F("Done"));
    ui->flushOutput();

  }
  else {
    ui->printErrorAndAbort(F("Savetype Error"), false);
  }
}

// Read sram and save to the SD card
void readSram(unsigned long sramSize, byte flashramType) {
  int offset = 512;
  int bufferSize = 512;
  if (flashramType == 2) {
    offset = 64;
    bufferSize = 128;
  }

  String outputFilePath = getNextN64SramOutputPath(romName, saveType);

  // Open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  for (unsigned long currByte = sramBase; currByte < (sramBase + (sramSize / flashramType)); currByte += offset) {
    // Set the address
    setAddress_N64(currByte);

    for (int c = 0; c < bufferSize; c += 2) {
      // split word
      word myWord = readWord_N64();
      byte loByte = myWord & 0xFF;
      byte hiByte = myWord >> 8;

      // write to buffer
      sdBuffer[c] = hiByte;
      sdBuffer[c + 1] = loByte;
    }
    outputFile.write(sdBuffer, bufferSize);
  }
  // Close the file:
  outputFile.close();
  ui->printMsg(F("Saved to "));
  ui->printlnMsg(outputFilePath);
  ui->flushOutput();
}

unsigned long verifySram(const String &filePath, unsigned long sramSize, byte flashramType) {
  writeErrors = 0;

  int offset = 512;
  int bufferSize = 512;
  if (flashramType == 2) {
    offset = 64;
    bufferSize = 128;
  }

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(filePath);
  for (unsigned long currByte = sramBase; currByte < (sramBase + (sramSize / flashramType)); currByte += offset) {
    // Set the address
    setAddress_N64(currByte);

    for (int c = 0; c < bufferSize; c += 2) {
      // split word
      word myWord = readWord_N64();
      byte loByte = myWord & 0xFF;
      byte hiByte = myWord >> 8;

      // write to buffer
      sdBuffer[c] = hiByte;
      sdBuffer[c + 1] = loByte;
    }
    // Check sdBuffer content against file on sd card
    for (int i = 0; i < bufferSize; i++) {
      if (inputFile.readByteOrDie() != sdBuffer[i]) {
        writeErrors++;
      }
    }
  }
  // Close the file:
  inputFile.close();
  
  // Return 0 if verified ok, or number of errors
  return writeErrors;
}

/******************************************
  Flashram functions
*****************************************/
// Send a command to the flashram command register
void sendFramCmd (unsigned long myCommand) {
  // Split command into two words
  word myComLowOut = myCommand & 0xFFFF;
  word myComHighOut = myCommand >> 16;

  // Set address to command register
  setAddress_N64(0x08010000);
  // Send command
  writeWord_N64(myComHighOut);
  writeWord_N64(myComLowOut);
}

// Init fram
void initFram() {
  // FRAM_EXECUTE_CMD
  sendFramCmd(0xD2000000);
  delay(10);
  // FRAM_EXECUTE_CMD
  sendFramCmd(0xD2000000);
  delay(10);
  //FRAM_STATUS_MODE_CMD
  sendFramCmd(0xE1000000);
  delay(10);
}

void writeFram(const String &inputFilePath, byte flashramType) {
  if (saveType != 4) {
    ui->printErrorAndAbort(F("Savetype Error"), true);
  }
  
  // Erase fram
  eraseFram();

  // Check if empty
  if (blankcheck_N64(flashramType) == 0) {
    ui->printlnMsg(F("OK"));
    ui->flushOutput();
  }
  else {
    ui->printlnMsg("FAIL");
    ui->flushOutput();
  }

  ui->printMsg(F("Writing "));
  ui->printlnMsg(inputFilePath);
  ui->flushOutput();

  // Open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
  
  // Init fram
  initFram();

  // Write all 8 fram banks
  ui->printMsg(F("Bank "));
  for (byte bank = 0; bank < 8; bank++) {
    ui->printMsg(bank);
    ui->printMsg(F(" "));
    ui->flushOutput();

    // Write one bank of 128*128 bytes
    for (byte offset = 0; offset < 128; offset++) {
      // Read save from SD into buffer
      inputFile.read(sdBuffer, 128);

      // FRAM_WRITE_MODE_CMD
      sendFramCmd(0xB4000000);
      delay(1);

      // Set the address for the next 128 bytes
      setAddress_N64(0x08000000);

      // Send 128 bytes, 64 words
      for (byte c = 0; c < 128; c += 2) {
        // Join two bytes into one word
        word myWord = ( ( sdBuffer[c] & 0xFF ) << 8 ) | ( sdBuffer[c + 1] & 0xFF );
        // Write word
        writeWord_N64(myWord);
      }
      // Delay between each "DMA"
      delay(1);

      //FRAM_WRITE_OFFSET_CMD + offset
      sendFramCmd((0xA5000000 | (((bank * 128) + offset) & 0xFFFF)));
      delay(1);

      // FRAM_EXECUTE_CMD
      sendFramCmd(0xD2000000);
      while (waitForFram(flashramType)) {
        delay(1);
      }
    }
    // Delay between banks
    delay(20);
  }
  ui->printlnMsg("");
  // Close the file:
  inputFile.close();
}

// Delete all 8 flashram banks
void eraseFram() {
  if (saveType != 4) {
    ui->printErrorAndAbort(F("Savetype Error"), true);
  }

  ui->printMsg(F("Erasing..."));
  ui->flushOutput();

  // Init fram
  initFram();

  // Erase fram
  // 0x4B00007F 0x4B0000FF 0x4B00017F 0x4B0001FF 0x4B00027F 0x4B0002FF 0x4B00037F 0x4B0003FF
  for (unsigned long bank = 0x4B00007F; bank < 0x4B00047F; bank += 0x80) {
    sendFramCmd(bank);
    delay(10);
    // FRAM_ERASE_MODE_CMD
    sendFramCmd(0x78000000);
    delay(10);
    // FRAM_EXECUTE_CMD
    sendFramCmd(0xD2000000);
    while (waitForFram(flashramType)) {
      delay(1);
    }
  }
}

// Read flashram
void readFram(byte flashramType) {
  if (saveType != 4) {
    ui->printErrorAndAbort(F("Savetype Error"), true);
  }

  // Put flashram into read mode
  // FRAM_READ_MODE_CMD
  sendFramCmd(0xF0000000);
  // Read Flashram
  readSram(131072, flashramType);
}

// Verify flashram
unsigned long verifyFram(const String &filePath, byte flashramType) {
  // Put flashram into read mode
  // FRAM_READ_MODE_CMD
  sendFramCmd(0xF0000000);
  writeErrors = verifySram(filePath, 131072, flashramType);
  return writeErrors;
}

// Blankcheck flashram
unsigned long blankcheck_N64(byte flashramType) {
  writeErrors = 0;

  int offset = 512;
  int bufferSize = 512;
  if (flashramType == 2) {
    offset = 64;
    bufferSize = 128;
  }

  // Put flashram into read mode
  // FRAM_READ_MODE_CMD
  sendFramCmd(0xF0000000);

  // Read Flashram
  for (unsigned long currByte = sramBase; currByte < (sramBase + (131072 / flashramType)); currByte += offset) {
    // Set the address for the next 512 bytes
    setAddress_N64(currByte);

    for (int c = 0; c < bufferSize; c += 2) {
      // split word
      word myWord = readWord_N64();
      byte loByte = myWord & 0xFF;
      byte hiByte = myWord >> 8;

      // write to buffer
      sdBuffer[c] = hiByte;
      sdBuffer[c + 1] = loByte;
    }
    // Check sdBuffer content against file on sd card
    for (int i = 0; i < bufferSize; i++) {
      if (0xFF != sdBuffer[i]) {
        writeErrors++;
      }
    }
  }
  // Return 0 if verified ok, or number of errors
  return writeErrors;
}

// Wait until current operation is done
byte waitForFram(byte flashramType) {
  byte framStatus = 0;
  byte statusMXL1100[] = {0x11, 0x11, 0x80, 0x01, 0x00, 0xC2, 0x00, 0x1E};
  byte statusMXL1101[] = {0x11, 0x11, 0x80, 0x01, 0x00, 0xC2, 0x00, 0x1D};
  byte statusMN63F81[] = {0x11, 0x11, 0x80, 0x01, 0x00, 0x32, 0x00, 0xF1};

  // FRAM_STATUS_MODE_CMD
  sendFramCmd(0xE1000000);
  delay(1);

  // Set address to Fram status register
  setAddress_N64(0x08000000);

  // Read Status
  for (byte c = 0; c < 8; c += 2) {
    // split word
    word myWord = readWord_N64();
    byte loByte = myWord & 0xFF;
    byte hiByte = myWord >> 8;

    // write to buffer
    sdBuffer[c] = hiByte;
    sdBuffer[c + 1] = loByte;
  }

  if (flashramType == 2) {
    for (byte c = 0; c < 8; c++) {
      if (statusMXL1100[c] != sdBuffer[c]) {
        framStatus = 1;
      }
    }
  }
  else if (flashramType == 1) {
    //MX29L1101
    if (MN63F81MPN == false) {
      for (byte c = 0; c < 8; c++) {
        if (statusMXL1101[c] != sdBuffer[c]) {
          framStatus = 1;
        }
      }
    }
    //MN63F81MPN
    else if (MN63F81MPN == true) {
      for (byte c = 0; c < 8; c++) {
        if (statusMN63F81[c] != sdBuffer[c]) {
          framStatus = 1;
        }
      }
    }
  }
  return framStatus;
}

// Get flashram type
void getFramType() {

  // FRAM_STATUS_MODE_CMD
  sendFramCmd(0xE1000000);
  delay(10);

  // Set address to Fram status register
  setAddress_N64(0x08000000);

  // Read Status
  for (byte c = 0; c < 8; c += 2) {
    // split word
    word myWord = readWord_N64();
    byte loByte = myWord & 0xFF;
    byte hiByte = myWord >> 8;

    // write to buffer
    sdBuffer[c] = hiByte;
    sdBuffer[c + 1] = loByte;
  }
  //MX29L1100
  if (sdBuffer[7] == 0x1e ) {
    flashramType = 2;
    ui->printlnMsg(F("Type: MX29L1100"));
    ui->flushOutput();
  }
  //MX29L1101
  else if (sdBuffer[7] == 0x1d )  {
    flashramType = 1;
    MN63F81MPN = false;
    ui->printlnMsg(F("Type: MX29L1101"));
    ui->flushOutput();
  }
  //MN63F81MPN
  else if (sdBuffer[7] == 0xf1 )  {
    flashramType = 1;
    MN63F81MPN = true;
    ui->printlnMsg(F("Type: MN63F81MPN"));
    ui->flushOutput();
  }
  // 29L1100KC-15B0 compat MX29L1101
  else if ((sdBuffer[7] == 0x8e ) || (sdBuffer[7] == 0x84 )) {
    flashramType = 1;
    MN63F81MPN = false;
    ui->printlnMsg(F("Type: 29L1100KC-15B0"));
    ui->printlnMsg(F("(compat. MX29L1101)"));
    ui->flushOutput();
  }
  // Type unknown
  else {
    for (byte c = 0; c < 8; c++) {
      ui->printMsg(sdBuffer[c], HEX);
      ui->printMsg(F(", "));
    }
    ui->printErrorAndAbort(F("Flashram unknown"), false);
  }
}

/******************************************
  Rom functions
*****************************************/
// Read rom and save to the SD card
void readRom_N64() {
  String outputFilePath = getNextN64RomOutputPathAndPrintMessage(romName);

readn64rom:
  // Open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  byte buffer[1024] = { 0 };

  // get current time
  unsigned long startTime = millis();

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(cartSize) * 1024 * 1024;
  ui->drawProgressBar(0, totalProgressBar);

  // prepare crc32
  uint32_t oldcrc32 = 0xFFFFFFFF;
  uint32_t tab_value = 0;
  uint8_t idx = 0;

  // run combined dumper + crc32 routine for better performance, as N64 ROMs are quite large for an 8bit micro
  // currently dumps + checksums a 32MB cart in 170 seconds (down from 347 seconds)
  for (unsigned long currByte = romBase; currByte < (romBase + (cartSize * 1024 * 1024)); currByte += 1024) {
    // Blink led
    if (currByte % 16384 == 0)
      PORTB ^= (1 << 4);

    // Set the address for the first 512 bytes to dump
    setAddress_N64(currByte);
    // Wait 62.5ns (safety)
    NOP;

    for (int c = 0; c < 512; c += 2) {
      // Pull read(PH6) low
      PORTH &= ~(1 << 6);
      // Wait ~310ns
      NOP; NOP; NOP; NOP; NOP;

      // data on PINK and PINF is valid now, read into sd card buffer
      buffer[c] =     PINK; // hiByte
      buffer[c + 1] = PINF; // loByte

      // Pull read(PH6) high
      PORTH |= (1 << 6);

      // crc32 update
      idx = ((oldcrc32) ^ (buffer[c]));
      tab_value = pgm_read_dword(crc_32_tab + idx);
      oldcrc32 = tab_value ^ ((oldcrc32) >> 8);
      idx = ((oldcrc32) ^ (buffer[c + 1]));
      tab_value = pgm_read_dword(crc_32_tab + idx);
      oldcrc32 = tab_value ^ ((oldcrc32) >> 8);
    }

    // Set the address for the next 512 bytes to dump
    setAddress_N64(currByte + 512);
    // Wait 62.5ns (safety)
    NOP;

    for (int c = 512; c < 1024; c += 2) {
      // Pull read(PH6) low
      PORTH &= ~(1 << 6);
      // Wait ~310ns
      NOP; NOP; NOP; NOP; NOP;

      // data on PINK and PINF is valid now, read into sd card buffer
      buffer[c] =     PINK; // hiByte
      buffer[c + 1] = PINF; // loByte

      // Pull read(PH6) high
      PORTH |= (1 << 6);

      // crc32 update
      idx = ((oldcrc32) ^ (buffer[c])) & 0xff;
      tab_value = pgm_read_dword(crc_32_tab + idx);
      oldcrc32 = tab_value ^ ((oldcrc32) >> 8);
      idx = ((oldcrc32) ^ (buffer[c + 1])) & 0xff;
      tab_value = pgm_read_dword(crc_32_tab + idx);
      oldcrc32 = tab_value ^ ((oldcrc32) >> 8);
    }

    processedProgressBar += 1024;
    ui->drawProgressBar(processedProgressBar, totalProgressBar);
    // write out 1024 bytes to file
    outputFile.write(buffer, 1024);
  }

  // Close the file:
  outputFile.close();

  unsigned long timeElapsed = (millis() - startTime) / 1000; // seconds

  ui->printMsg(F("CRC: "));
  ui->flushOutput();
  // convert checksum to string
  char crcStr[9];
  sprintf(crcStr, "%08lx", ~oldcrc32);
  // Print checksum
  ui->printlnMsg(crcStr);
  ui->flushOutput();

  // Search n64.txt for crc
  if (searchCRC(crcStr)) {
    // Dump was a known good rom
    ui->printlnMsg(F("Checksum matches"));
  }
  else {
    // Dump was bad or unknown
    rgb.setColor(255, 0, 0);

    // let bad crc show a short while
    delay(3000);

    const __FlashStringHelper *crcItem_Redump = F("Redump");
    const __FlashStringHelper *crcItem_Ignore = F("Ignore");
    const __FlashStringHelper *crcItem_Reset = F("Reset");
    const __FlashStringHelper *crcMenu[] = {
      crcItem_Redump,
      crcItem_Ignore,
      crcItem_Reset,
    };

    const __FlashStringHelper *crcAnswer = ui->askMultipleChoiceQuestion(
      F("CRC ERROR"), crcMenu, ARRAY_LENGTH(crcMenu), crcItem_Redump);

    if (crcAnswer == crcItem_Redump) {
      // Delete old file
      SafeSDFile oldFile = SafeSDFile::openForWriting(outputFilePath);
      oldFile.remove();
      // Dump again
      ui->clearOutput();
      ui->printlnMsg(F("Reading Rom..."));
      ui->flushOutput();
      rgb.setColor(0, 0, 0);
      goto readn64rom;
    }
    else if (crcAnswer == crcItem_Ignore) {
      // Return to N64 menu
      ;
    }
    else {
      resetArduino();
    }
  }
  ui->flushOutput();

  ui->printMsg(F("Done ("));
  ui->printMsg(timeElapsed); // include elapsed time
  ui->printlnMsg(F("s)"));
  ui->printlnMsg(F(""));
  ui->printlnMsg(F("Press Button..."));
  ui->flushOutput();
  ui->waitForUserInput();
}

/******************************************
   N64 Repro Flashrom Functions
 *****************************************/
void flashRepro_N64() {
  // Check flashrom ID's
  idFlashrom_N64();

  // If the ID is known continue
  if (cartSize != 0) {
    // Print flashrom name
    if ((strcmp(flashid, "227E") == 0)  && (strcmp(cartID, "2201") == 0)) {
      ui->printMsg(F("Spansion S29GL256N"));
      if (cartSize == 64)
        ui->printlnMsg(F(" x2"));
      else
        ui->printlnMsg("");
    }
    else if ((strcmp(flashid, "227E") == 0)  && (strcmp(cartID, "2101") == 0)) {
      ui->printMsg(F("Spansion S29GL128N"));
    }
    else if ((strcmp(flashid, "22C9") == 0) || (strcmp(flashid, "22CB") == 0)) {
      ui->printMsg(F("Macronix MX29LV640"));
      if (cartSize == 16)
        ui->printlnMsg(F(" x2"));
      else
        ui->printlnMsg("");
    }
    else if (strcmp(flashid, "8816") == 0)
      ui->printlnMsg(F("Intel 4400L0ZDQ0"));
    else if (strcmp(flashid, "7E7E") == 0)
      ui->printlnMsg(F("Fujitsu MSP55LV100S"));
    else if ((strcmp(flashid, "227E") == 0) && (strcmp(cartID, "2301") == 0))
      ui->printlnMsg(F("Fujitsu MSP55LV512"));
    else if ((strcmp(flashid, "227E") == 0) && (strcmp(cartID, "3901") == 0))
      ui->printlnMsg(F("Intel 512M29EW"));

    // Print info
    ui->printMsg(F("ID: "));
    ui->printMsg(flashid);
    ui->printMsg(F(" Size: "));
    ui->printMsg(cartSize);
    ui->printlnMsg(F("MB"));
    ui->printlnMsg("");
    ui->printlnMsg(F("This will erase your"));
    ui->printlnMsg(F("Repro Cartridge."));
    ui->printlnMsg(F("Attention: Use 3.3V!"));
    ui->printlnMsg("");
    ui->printlnMsg(F("Press Button"));
    ui->flushOutput();
    ui->waitForUserInput();

    // Launch file browser
    String inputFilePath = fileBrowser(F("Select z64 file"));
    ui->clearOutput();
    ui->flushOutput();

    // Open file on sd card
    SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);
    // Get rom size from file
    fileSize = inputFile.fileSize();
    ui->printMsg(F("File size: "));
    ui->printMsg(fileSize / 1048576);
    ui->printlnMsg(F("MB"));
    ui->flushOutput();

    // Compare file size to flashrom size
    if ((fileSize / 1048576) > cartSize) {
      ui->printErrorAndAbort(F("File too big"), false);
    }

    // Erase needed sectors
    if (strcmp(flashid, "227E") == 0) {
      // Spansion S29GL256N or Fujitsu MSP55LV512 with 0x20000 sector size and 32 byte buffer
      eraseFlashrom_N64(0x20000);
    }
    else if (strcmp(flashid, "7E7E") == 0) {
      // Fujitsu MSP55LV100S
      eraseMSP55LV100_N64();
    }
    else if ((strcmp(flashid, "8813") == 0) || (strcmp(flashid, "8816") == 0)) {
      // Intel 4400L0ZDQ0
      eraseIntel4400_N64();
      resetIntel4400_N64();
    }
    else if ((strcmp(flashid, "22C9") == 0) || (strcmp(flashid, "22CB") == 0)) {
      // Macronix MX29LV640, C9 is top boot and CB is bottom boot block
      eraseFlashrom_N64(0x8000);
    }

    // Check if erase was successful
    if (blankcheckFlashrom_N64()) {
      // Write flashrom
      ui->printlnMsg(F("OK"));
      ui->printMsg(F("Writing "));
      ui->printlnMsg(inputFilePath);
      ui->flushOutput();


      if ((strcmp(cartID, "3901") == 0) && (strcmp(flashid, "227E") == 0)) {
        // Intel 512M29EW(64MB) with 0x20000 sector size and 128 byte buffer
        writeFlashBuffer_N64(inputFile, 0x20000, 128);
      }
      else if (strcmp(flashid, "227E") == 0) {
        // Spansion S29GL128N/S29GL256N or Fujitsu MSP55LV512 with 0x20000 sector size and 32 byte buffer
        writeFlashBuffer_N64(inputFile, 0x20000, 32);
      }
      else if (strcmp(flashid, "7E7E") == 0) {
        //Fujitsu MSP55LV100S
        writeMSP55LV100_N64(inputFile, 0x20000);
      }
      else if ((strcmp(flashid, "22C9") == 0) || (strcmp(flashid, "22CB") == 0)) {
        // Macronix MX29LV640 without buffer
        writeFlashrom_N64(inputFile);
      }
      else if ((strcmp(flashid, "8813") == 0) || (strcmp(flashid, "8816") == 0)) {
        // Intel 4400L0ZDQ0
        writeIntel4400_N64(inputFile);
        resetIntel4400_N64();
      }

      // Close the file:
      inputFile.close();

      // Verify
      ui->printMsg(F("Verifying..."));
      ui->flushOutput();
      writeErrors = verifyFlashrom_N64(inputFilePath);
      if (writeErrors == 0) {
        ui->printlnMsg(F("OK"));
        ui->flushOutput();
      }
      else {
        ui->printMsg(writeErrors);
        ui->printMsg(F(" bytes "));
        ui->printError(F("did not verify."));
      }
    }
    else {
      // Close the file
      inputFile.close();
      ui->printError(F("failed"));
    }
  }
  // If the ID is unknown show error message
  else {
    ui->printMsg(F("Vendor: "));
    ui->printlnMsg(vendorID);
    ui->printMsg(F("ID: "));
    ui->printMsg(flashid);
    ui->printMsg(F(" "));
    ui->printlnMsg(cartID);
    ui->printError(F("Unknown flashrom"));
  }

  ui->printlnMsg(F("Press Button..."));
  ui->flushOutput();
  ui->waitForUserInput();
  ui->clearOutput();
  ui->flushOutput();
}

// Reset to read mode
void resetIntel4400_N64() {
  for (unsigned long currPartition = 0; currPartition < (cartSize * 0x100000); currPartition += 0x20000) {
    setAddress_N64(romBase + currPartition);
    writeWord_N64(0xFF);
  }
}

// Reset Fujitsu MSP55LV100S
void resetMSP55LV100_N64(unsigned long flashBase) {
  // Send reset Command
  setAddress_N64(flashBase);
  writeWord_N64(0xF0F0);
  delay(100);
}

// Common reset command
void resetFlashrom_N64(unsigned long flashBase) {
  // Send reset Command
  setAddress_N64(flashBase);
  writeWord_N64(0xF0);
  delay(100);
}

void idFlashrom_N64() {
  // Set size to 0 if no ID is found
  cartSize = 0;

  // Send flashrom ID command
  setAddress_N64(romBase + (0x555 << 1));
  writeWord_N64(0xAA);
  setAddress_N64(romBase + (0x2AA << 1));
  writeWord_N64(0x55);
  setAddress_N64(romBase + (0x555 << 1));
  writeWord_N64(0x90);

  // Read 1 byte vendor ID
  setAddress_N64(romBase);
  sprintf(vendorID, "%02X", readWord_N64());
  // Read 2 bytes flashrom ID
  sprintf(flashid, "%04X", readWord_N64());
  // Read 2 bytes secondary flashrom ID
  setAddress_N64(romBase + 0x1C);
  sprintf(cartID, "%04X", ((readWord_N64() << 8)  | (readWord_N64() & 0xFF)));

  // Spansion S29GL256N(32MB/64MB) with either one or two flashrom chips
  if ((strcmp(cartID, "2201") == 0) && (strcmp(flashid, "227E") == 0)) {
    cartSize = 32;

    // Reset flashrom
    resetFlashrom_N64(romBase);

    // Test for second flashrom chip at 0x2000000 (32MB)
    setAddress_N64(romBase + 0x2000000 + (0x555 << 1));
    writeWord_N64(0xAA);
    setAddress_N64(romBase + 0x2000000 + (0x2AA << 1));
    writeWord_N64(0x55);
    setAddress_N64(romBase + 0x2000000 + (0x555 << 1));
    writeWord_N64(0x90);

    char tempID[5];
    setAddress_N64(romBase + 0x2000000);
    // Read manufacturer ID
    readWord_N64();
    // Read flashrom ID
    sprintf(tempID, "%04X", readWord_N64());

    // Check if second flashrom chip is present
    if (strcmp(tempID, "227E") == 0)  {
      cartSize = 64;
    }
    resetFlashrom_N64(romBase + 0x2000000);
  }

  // Macronix MX29LV640(8MB/16MB) with either one or two flashrom chips
  else if ((strcmp(flashid, "22C9") == 0) || (strcmp(flashid, "22CB") == 0)) {
    cartSize = 8;

    resetFlashrom_N64(romBase + 0x800000);

    // Test for second flashrom chip at 0x800000 (8MB)
    setAddress_N64(romBase + 0x800000 + (0x555 << 1));
    writeWord_N64(0xAA);
    setAddress_N64(romBase + 0x800000 + (0x2AA << 1));
    writeWord_N64(0x55);
    setAddress_N64(romBase + 0x800000 + (0x555 << 1));
    writeWord_N64(0x90);

    char tempID[5];
    setAddress_N64(romBase + 0x800000);
    // Read manufacturer ID
    readWord_N64();
    // Read flashrom ID
    sprintf(tempID, "%04X", readWord_N64());

    // Check if second flashrom chip is present
    if ((strcmp(tempID, "22C9") == 0) || (strcmp(tempID, "22CB") == 0)) {
      cartSize = 16;
    }
    resetFlashrom_N64(romBase + 0x800000);
  }

  // Intel 4400L0ZDQ0 (64MB)
  else if (strcmp(flashid, "8816") == 0) {
    // Found first flashrom chip, set to 32MB
    cartSize = 32;
    resetIntel4400_N64();

    // Test if second half of the flashrom might be hidden
    setAddress_N64(romBase + 0x2000000 + (0x555 << 1));
    writeWord_N64(0xAA);
    setAddress_N64(romBase + 0x2000000 + (0x2AA << 1));
    writeWord_N64(0x55);
    setAddress_N64(romBase + 0x2000000 + (0x555 << 1));
    writeWord_N64(0x90);

    // Read manufacturer ID
    setAddress_N64(romBase + 0x2000000);
    readWord_N64();
    // Read flashrom ID
    sprintf(cartID, "%04X", readWord_N64());
    if (strcmp(cartID, "8813") == 0) {
      cartSize = 64;
      strncpy(flashid , cartID, 5);
    }
    resetIntel4400_N64();
    // Empty cartID string
    cartID[0] = '\0';
  }

  //Fujitsu MSP55LV512/Spansion S29GL512N (64MB)
  else if ((strcmp(cartID, "2301") == 0) && (strcmp(flashid, "227E") == 0)) {
    cartSize = 64;
    // Reset flashrom
    resetFlashrom_N64(romBase);
  }

  // Spansion S29GL128N(16MB) with one flashrom chip
  else if ((strcmp(cartID, "2101") == 0) && (strcmp(flashid, "227E") == 0)) {
    cartSize = 16;
    // Reset flashrom
    resetFlashrom_N64(romBase);
  }

  // Intel 512M29EW(64MB) with one flashrom chip
  else if ((strcmp(cartID, "3901") == 0) && (strcmp(flashid, "227E") == 0)) {
    cartSize = 64;
    // Reset flashrom
    resetFlashrom_N64(romBase);
  }

  // Unknown 227E type
  else if (strcmp(flashid, "227E") == 0) {
    cartSize = 0;
    // Reset flashrom
    resetFlashrom_N64(romBase);
  }

  //Test for Fujitsu MSP55LV100S (64MB)
  else  {
    // Send flashrom ID command
    setAddress_N64(romBase + (0x555 << 1));
    writeWord_N64(0xAAAA);
    setAddress_N64(romBase + (0x2AA << 1));
    writeWord_N64(0x5555);
    setAddress_N64(romBase + (0x555 << 1));
    writeWord_N64(0x9090);

    setAddress_N64(romBase);
    // Read 1 byte vendor ID
    readWord_N64();
    // Read 2 bytes flashrom ID
    sprintf(cartID, "%04X", readWord_N64());

    if (strcmp(cartID, "7E7E") == 0) {
      resetMSP55LV100_N64(romBase);
      cartSize = 64;
      strncpy(flashid , cartID, 5);
    }
  }
}

// Erase Intel flashrom
void eraseIntel4400_N64() {
  unsigned long flashBase = romBase;

  ui->printMsg(F("Erasing..."));
  ui->flushOutput();

  // If the game is smaller than 32Mbit only erase the needed blocks
  unsigned long lastBlock = 0x1FFFFFF;
  if (fileSize < 0x1FFFFFF)
    lastBlock = fileSize;

  // Erase 4 blocks with 16kwords each
  for (unsigned long currBlock = 0x0; currBlock < 0x1FFFF; currBlock += 0x8000) {
    // Unlock block command
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0x60);
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0xD0);
    // Erase command
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0x20);
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0xD0);

    // Read the status register
    setAddress_N64(flashBase + currBlock);
    word statusReg = readWord_N64();
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      setAddress_N64(flashBase + currBlock);
      statusReg = readWord_N64();
    }
  }

  // Erase up to 255 blocks with 64kwords each
  for (unsigned long currBlock = 0x20000; currBlock < lastBlock; currBlock += 0x1FFFF) {
    // Unlock block command
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0x60);
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0xD0);
    // Erase command
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0x20);
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0xD0);

    // Read the status register
    setAddress_N64(flashBase + currBlock);
    word statusReg = readWord_N64();
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      setAddress_N64(flashBase + currBlock);
      statusReg = readWord_N64();
    }

    // Blink led
    PORTB ^= (1 << 4);
  }

  // Check if we should erase the second chip too
  if ((cartSize = 64) && (fileSize > 0x2000000)) {
    // Switch base address to second chip
    flashBase = romBase + 0x2000000;

    // 255 blocks with 64kwords each
    for (unsigned long currBlock = 0x0; currBlock < 0x1FDFFFF; currBlock += 0x1FFFF) {
      // Unlock block command
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0x60);
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0xD0);
      // Erase command
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0x20);
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0xD0);

      // Read the status register
      setAddress_N64(flashBase + currBlock);
      word statusReg = readWord_N64();
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        setAddress_N64(flashBase + currBlock);
        statusReg = readWord_N64();
      }

      // Blink led
      PORTB ^= (1 << 4);
    }

    // 4 blocks with 16kword each
    for (unsigned long currBlock = 0x1FE0000; currBlock < 0x1FFFFFF; currBlock += 0x8000) {
      // Unlock block command
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0x60);
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0xD0);
      // Erase command
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0x20);
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0xD0);

      // Read the status register
      setAddress_N64(flashBase + currBlock);
      word statusReg = readWord_N64();
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        setAddress_N64(flashBase + currBlock);
        statusReg = readWord_N64();
      }
    }
  }
}

// Erase Fujutsu MSP55LV100S
void eraseMSP55LV100_N64() {
  unsigned long flashBase = romBase;
  unsigned long sectorSize = 0x20000;

  ui->printMsg(F("Erasing..."));
  ui->flushOutput();

  for (unsigned long currSector = 0; currSector < fileSize; currSector += sectorSize) {
    // Blink led
    PORTB ^= (1 << 4);

    // Send Erase Command to first chip
    setAddress_N64(flashBase + (0x555 << 1));
    writeWord_N64(0xAAAA);
    setAddress_N64(flashBase + (0x2AA << 1));
    writeWord_N64(0x5555);
    setAddress_N64(flashBase + (0x555 << 1));
    writeWord_N64(0x8080);
    setAddress_N64(flashBase + (0x555 << 1));
    writeWord_N64(0xAAAA);
    setAddress_N64(flashBase + (0x2AA << 1));
    writeWord_N64(0x5555);
    setAddress_N64(romBase + currSector);
    writeWord_N64(0x3030);

    // Read the status register
    setAddress_N64(romBase + currSector);
    word statusReg = readWord_N64();
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      setAddress_N64(romBase + currSector);
      statusReg = readWord_N64();
    }

    // Read the status register
    setAddress_N64(romBase + currSector);
    statusReg = readWord_N64();
    while ((statusReg | 0x7FFF) != 0xFFFF) {
      setAddress_N64(romBase + currSector);
      statusReg = readWord_N64();
    }
  }
}

// Common sector erase command
void eraseFlashrom_N64(unsigned long sectorSize) {
  unsigned long flashBase = romBase;

  ui->printMsg(F("Erasing..."));
  ui->flushOutput();

  for (unsigned long currSector = 0; currSector < fileSize; currSector += sectorSize) {
    // Blink led
    PORTB ^= (1 << 4);

    // Spansion S29GL256N(32MB/64MB) with two flashrom chips
    if ((currSector == 0x2000000) && (strcmp(cartID, "2201") == 0) && (strcmp(flashid, "227E") == 0)) {
      // Change to second chip
      flashBase = romBase + 0x2000000;
    }
    // Macronix MX29LV640(8MB/16MB) with two flashrom chips
    else if ((currSector == 0x800000) && ((strcmp(flashid, "22C9") == 0) || (strcmp(flashid, "22CB") == 0))) {
      flashBase = romBase + 0x800000;
    }

    // Send Erase Command
    setAddress_N64(flashBase + (0x555 << 1));
    writeWord_N64(0xAA);
    setAddress_N64(flashBase + (0x2AA << 1));
    writeWord_N64(0x55);
    setAddress_N64(flashBase + (0x555 << 1));
    writeWord_N64(0x80);
    setAddress_N64(flashBase + (0x555 << 1));
    writeWord_N64(0xAA);
    setAddress_N64(flashBase + (0x2AA << 1));
    writeWord_N64(0x55);
    setAddress_N64(romBase + currSector);
    writeWord_N64(0x30);

    // Read the status register
    setAddress_N64(romBase + currSector);
    word statusReg = readWord_N64();
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      setAddress_N64(romBase + currSector);
      statusReg = readWord_N64();
    }
  }
}

boolean blankcheckFlashrom_N64() {
  for (unsigned long currByte = romBase; currByte < romBase + fileSize; currByte += 512) {
    // Blink led
    if (currByte % 131072 == 0)
      PORTB ^= (1 << 4);

    // Set the address
    setAddress_N64(currByte);

    for (int c = 0; c < 512; c += 2) {
      if (readWord_N64() != 0xFFFF) {
        return 0;
      }
    }
  }
  return 1;
}

// Write Intel flashrom
void writeIntel4400_N64(SafeSDFile &inputFile) {
  for (unsigned long currSector = 0; currSector < fileSize; currSector += 131072) {
    // Blink led
    PORTB ^= (1 << 4);

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < 131072; currSdBuffer += 512) {
      // Fill SD buffer
      inputFile.read(sdBuffer, 512);

      // Write 32 words at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 64) {
        // Buffered program command
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        writeWord_N64(0xE8);

        // Check Status register
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        word statusReg = readWord_N64();
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
          statusReg = readWord_N64();
        }

        // Write word count (minus 1)
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        writeWord_N64(0x1F);

        // Write buffer
        for (byte currByte = 0; currByte < 64; currByte += 2) {
          // Join two bytes into one word
          word currWord = ( ( sdBuffer[currWriteBuffer + currByte] & 0xFF ) << 8 ) | ( sdBuffer[currWriteBuffer + currByte + 1] & 0xFF );
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + currByte);
          writeWord_N64(currWord);
        }

        // Write Buffer to Flash
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + 62);
        writeWord_N64(0xD0);

        // Read the status register at last written address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + 62);
        statusReg = readWord_N64();
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + 62);
          statusReg = readWord_N64();
        }
      }
    }
  }
}
// Write Fujitsu MSP55LV100S flashrom consisting out of two MSP55LV512 flashroms one used for the high byte the other for the low byte
void writeMSP55LV100_N64(SafeSDFile &inputFile, unsigned long sectorSize) {
  unsigned long flashBase = romBase;

  for (unsigned long currSector = 0; currSector < fileSize; currSector += sectorSize) {
    // Blink led
    PORTB ^= (1 << 4);

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < sectorSize; currSdBuffer += 512) {
      // Fill SD buffer
      inputFile.read(sdBuffer, 512);

      // Write 32 bytes at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 32) {

        // 2 unlock commands
        setAddress_N64(flashBase + (0x555 << 1));
        writeWord_N64(0xAAAA);
        setAddress_N64(flashBase + (0x2AA << 1));
        writeWord_N64(0x5555);

        // Write buffer load command at sector address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        writeWord_N64(0x2525);
        // Write word count (minus 1) at sector address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        writeWord_N64(0x0F0F);

        // Define variable before loop so we can use it later when reading the status register
        word currWord;

        for (byte currByte = 0; currByte < 32; currByte += 2) {
          // Join two bytes into one word
          currWord = ( ( sdBuffer[currWriteBuffer + currByte] & 0xFF ) << 8 ) | ( sdBuffer[currWriteBuffer + currByte + 1] & 0xFF );

          // Load Buffer Words
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + currByte);
          writeWord_N64(currWord);
        }

        // Write Buffer to Flash
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + 30);
        writeWord_N64(0x2929);

        // Read the status register at last written address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + 30);
        word statusReg = readWord_N64();
        while ((statusReg | 0x7F7F) != (currWord | 0x7F7F)) {
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + 30);
          statusReg = readWord_N64();
        }
      }
    }
  }
}

// Write Spansion S29GL256N flashrom using the 32 byte write buffer
void writeFlashBuffer_N64(SafeSDFile &inputFile, unsigned long sectorSize, byte bufferSize) {
  unsigned long flashBase = romBase;

  for (unsigned long currSector = 0; currSector < fileSize; currSector += sectorSize) {
    // Blink led
    PORTB ^= (1 << 4);

    // Spansion S29GL256N(32MB/64MB) with two flashrom chips
    if ((currSector == 0x2000000) && (strcmp(cartID, "2201") == 0)) {
      flashBase = romBase + 0x2000000;
    }

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < sectorSize; currSdBuffer += 512) {
      // Fill SD buffer
      inputFile.read(sdBuffer, 512);

      // Write 32 bytes at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += bufferSize) {

        // 2 unlock commands
        setAddress_N64(flashBase + (0x555 << 1));
        writeWord_N64(0xAA);
        setAddress_N64(flashBase + (0x2AA << 1));
        writeWord_N64(0x55);

        // Write buffer load command at sector address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        writeWord_N64(0x25);
        // Write word count (minus 1) at sector address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        writeWord_N64((bufferSize / 2) - 1);

        // Define variable before loop so we can use it later when reading the status register
        word currWord;

        for (byte currByte = 0; currByte < bufferSize; currByte += 2) {
          // Join two bytes into one word
          currWord = ( ( sdBuffer[currWriteBuffer + currByte] & 0xFF ) << 8 ) | ( sdBuffer[currWriteBuffer + currByte + 1] & 0xFF );

          // Load Buffer Words
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + currByte);
          writeWord_N64(currWord);
        }

        // Write Buffer to Flash
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + bufferSize - 2);
        writeWord_N64(0x29);

        // Read the status register at last written address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + bufferSize - 2);
        word statusReg = readWord_N64();
        while ((statusReg | 0xFF7F) != (currWord | 0xFF7F)) {
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + bufferSize - 2);
          statusReg = readWord_N64();
        }
      }
    }
  }
}

// Write MX29LV640 flashrom without write buffer
void writeFlashrom_N64(SafeSDFile &inputFile) {
  unsigned long flashBase = romBase;

  for (unsigned long currSector = 0; currSector < fileSize; currSector += 0x8000) {
    // Blink led
    PORTB ^= (1 << 4);

    // Macronix MX29LV640(8MB/16MB) with two flashrom chips
    if (currSector == 0x800000) {
      flashBase = romBase + 0x800000;
    }

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < 0x8000; currSdBuffer += 512) {
      // Fill SD buffer
      inputFile.read(sdBuffer, 512);
      for (int currByte = 0; currByte < 512; currByte += 2) {
        // Join two bytes into one word
        word currWord = ( ( sdBuffer[currByte] & 0xFF ) << 8 ) | ( sdBuffer[currByte + 1] & 0xFF );
        // 2 unlock commands
        setAddress_N64(flashBase + (0x555 << 1));
        writeWord_N64(0xAA);
        setAddress_N64(flashBase + (0x2AA << 1));
        writeWord_N64(0x55);
        // Program command
        setAddress_N64(flashBase + (0x555 << 1));
        writeWord_N64(0xA0);
        // Write word
        setAddress_N64(romBase + currSector + currSdBuffer + currByte);
        writeWord_N64(currWord);

        // Read the status register
        setAddress_N64(romBase + currSector + currSdBuffer + currByte);
        word statusReg = readWord_N64();
        while ((statusReg | 0xFF7F) != (currWord | 0xFF7F)) {
          setAddress_N64(romBase + currSector + currSdBuffer + currByte);
          statusReg = readWord_N64();
        }
      }
    }
  }
}

unsigned long verifyFlashrom_N64(const String &filePath) {
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
        word currWord = ( ( sdBuffer[currByte] & 0xFF ) << 8 ) | ( sdBuffer[currByte + 1] & 0xFF );
        // Read flash
        setAddress_N64(romBase + currSector + currSdBuffer + currByte);
        // Compare both
        if (readWord_N64() != currWord) {
          writeErrors++;
          // Abord if too many errors
          if (writeErrors > 20) {
            ui->printMsg(F("More than "));
            // Close the file:
            inputFile.close();
            return writeErrors;
          }
        }
      }
    }
  }
  // Close the file:
  inputFile.close();
  return writeErrors;
}

/******************************************
   N64 Gameshark Flash Functions
 *****************************************/
void flashGameshark_N64() {
  // Check flashrom ID's
  idGameshark_N64();

  // Check for SST 29LE010
  if (strcmp(flashid, "0808") == 0) {
    backupGameshark_N64();
    ui->printlnMsg("");
    ui->printlnMsg(F("This will erase your"));
    ui->printlnMsg(F("Gameshark cartridge"));
    ui->printlnMsg(F("Attention: Use 3.3V!"));
    ui->printlnMsg("");
    ui->printlnMsg(F("Press Button"));
    ui->flushOutput();
    ui->waitForUserInput();

    // Launch file browser
    String inputFilePath = fileBrowser(F("Select z64 file"));
    ui->clearOutput();
    ui->flushOutput();

    // Open file on sd card
    SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

    // Get rom size from file
    fileSize = inputFile.fileSize();
    ui->printMsg(F("File size: "));
    ui->printMsg(fileSize / 1024);
    ui->printlnMsg(F("KB"));
    ui->flushOutput();

    // Compare file size to flashrom size
    if (fileSize > 262144) {
      ui->printErrorAndAbort(F("File too big"), false);
    }

    // SST 29LE010, chip erase not needed as this eeprom automaticly erases during the write cycle
    eraseGameshark_N64();

    // Write flashrom
    ui->printMsg(F("Writing "));
    ui->printlnMsg(inputFilePath);
    ui->flushOutput();
    writeGameshark_N64(inputFile);

    // Close the file:
    inputFile.close();

    // Verify
    ui->printMsg(F("Verifying..."));
    ui->flushOutput();
    writeErrors = verifyGameshark_N64(inputFilePath);

    if (writeErrors == 0) {
      ui->printlnMsg(F("OK"));
      ui->flushOutput();
    }
    else {
      ui->printMsg(writeErrors);
      ui->printMsg(F(" bytes "));
      ui->printError(F("did not verify."));
    }
  }
  // If the ID is unknown show error message
  else {
    ui->printMsg(F("ID: "));
    ui->printlnMsg(flashid);
    ui->printError(F("Unknown flashrom"));
  }

  ui->printlnMsg(F("Press Button..."));
  ui->flushOutput();
  ui->waitForUserInput();
  ui->clearOutput();
  ui->flushOutput();
}


//Test for SST 29LE010
void idGameshark_N64() {
  //Send flashrom ID command
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0xAAAA);
  setAddress_N64(romBase + 0x5554);
  writeWord_N64(0x5555);
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0x9090);

  setAddress_N64(romBase);
  // Read 1 byte vendor ID
  readWord_N64();
  // Read 2 bytes flashrom ID
  sprintf(flashid, "%04X", readWord_N64());
  // Reset flashrom
  resetGameshark_N64();
}

//Reset ST29LE010
void resetGameshark_N64() {
  // Send reset Command
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0xAAAA);
  setAddress_N64(romBase + 0x5554);
  writeWord_N64(0x5555);
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0xF0F0);
  delay(100);
}

// Read rom and save to the SD card
void backupGameshark_N64() {
  String outputFilePath = getNextN64GamesharkBackupOutputPathAndPrintMessage();

  // Open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  for (unsigned long currByte = romBase + 0xC00000; currByte < (romBase + 0xC00000 + 262144); currByte += 512) {
    // Blink led
    if (currByte % 16384 == 0)
      PORTB ^= (1 << 4);

    // Set the address for the next 512 bytes
    setAddress_N64(currByte);

    for (int c = 0; c < 512; c += 2) {
      // split word
      word myWord = readWord_N64();
      byte loByte = myWord & 0xFF;
      byte hiByte = myWord >> 8;

      // write to buffer
      sdBuffer[c] = hiByte;
      sdBuffer[c + 1] = loByte;
    }
    outputFile.write(sdBuffer, 512);
  }
  // Close the file:
  outputFile.close();
}

// Send chip erase to the two SST29LE010 inside the Gameshark
void eraseGameshark_N64() {
  ui->printlnMsg(F("Erasing..."));
  ui->flushOutput();

  //Sending erase command according to datasheet
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0xAAAA);
  setAddress_N64(romBase + 0x5554);
  writeWord_N64(0x5555);
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0x8080);
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0xAAAA);
  setAddress_N64(romBase + 0x5554);
  writeWord_N64(0x5555);
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0x1010);

  delay(20);
}

// Write Gameshark with 2x SST29LE010 Eeproms
void writeGameshark_N64(SafeSDFile &inputFile) {
  // Each 29LE010 has 1024 pages, each 128 bytes in size
  for (unsigned long currPage = 0; currPage < fileSize / 2; currPage += 128) {
    // Fill SD buffer with twice the amount since we flash 2 chips
    inputFile.read(sdBuffer, 256);
    // Blink led
    PORTB ^= (1 << 4);

    //Send page write command to both flashroms
    setAddress_N64(romBase + 0xAAAA);
    writeWord_N64(0xAAAA);
    setAddress_N64(romBase + 0x5554);
    writeWord_N64(0x5555);
    setAddress_N64(romBase + 0xAAAA);
    writeWord_N64(0xA0A0);

    // Write 1 page each, one flashrom gets the low byte, the other the high byte.
    for (unsigned long currByte = 0; currByte < 256; currByte += 2) {
      // Set address
      setAddress_N64(romBase + 0xC00000 + (currPage * 2) + currByte);
      // Join two bytes into one word
      word currWord = ((sdBuffer[currByte] & 0xFF) << 8) | (sdBuffer[currByte + 1] & 0xFF);
      // Send byte data
      writeWord_N64(currWord);
    }
    delay(30);
  }
}

unsigned long verifyGameshark_N64(const String &filePath) {
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
        word currWord = ( ( sdBuffer[currByte] & 0xFF ) << 8 ) | ( sdBuffer[currByte + 1] & 0xFF );
        // Read flash
        setAddress_N64(romBase + 0xC00000 + currSector + currSdBuffer + currByte);
        // Compare both
        if (readWord_N64() != currWord) {
          if ( (strcmp(flashid, "0808") == 0) && (currSector + currSdBuffer + currByte > 0x3F) && (currSector + currSdBuffer + currByte < 0x1080)) {
            // Gameshark maps this area to the bootcode of the plugged in cartridge
          }
          else {
            writeErrors++;
          }
        }
      }
    }
  }
  // Close the file:
  inputFile.close();
  return writeErrors;
}

//******************************************
// End of File
//******************************************
