//******************************************
// GB SMART MODULE
// Supports 32M cart with LH28F016SUT flash
//******************************************

#include <Arduino.h>
#include "GBSmart.h"
#include "GB.h"
#include "filebrowser.h"
#include "ui.h"
#include "globals.h"
#include "utils.h"
#include "SD.h"

#define GB_SMART_GAMES_PER_PAGE  6

/******************************************
   Function prototypes
 *****************************************/
void gbSmartGameMenu();
String getNextGbSmartFlashOutputPathAndPrintMessage();
void gbSmartGetGames();
void gbSmartReadFlash(const String &outputFilePath);
void gbSmartWriteFlash(const String &inputFilePath);
void gbSmartWriteFlash(const String &inputFilePath, uint32_t start_bank);
void gbSmartWriteFlashFromMyFile(SafeSDFile &inputFile, uint32_t addr);
uint32_t gbSmartVerifyFlash(const String &filePath);
byte gbSmartBlankCheckingFlash(uint8_t flash_start_bank);
void gbSmartResetFlash(uint8_t flash_start_bank);
void gbSmartEraseFlash(uint8_t flash_start_bank);
void gbSmartWriteFlashByte(uint32_t myAddress, uint8_t myData);
void gbSmartRemapStartBank(uint8_t rom_start_bank, uint8_t rom_size, uint8_t sram_size);
uint8_t gbSmartGetResizeParam(uint8_t rom_size, uint8_t sram_size);

typedef struct
{
  uint8_t start_bank;
  uint8_t rom_type;
  uint8_t rom_size;
  uint8_t sram_size;
  char title[16];
} GBSmartGameInfo;

uint32_t gbSmartSize = 32 * 131072;
uint16_t gbSmartBanks = 256;

uint8_t gbSmartBanksPerFlashChip = 128;
uint8_t gbSmartBanksPerFlashBlock = 4;
uint32_t gbSmartFlashBlockSize = (gbSmartBanksPerFlashBlock << 14);

uint8_t gbSmartRomSizeGB = 0x07;
uint8_t gbSmartSramSizeGB = 0x04;
uint8_t gbSmartFlashSizeGB = 0x06;

GBSmartGameInfo gbSmartGames[GB_SMART_GAMES_PER_PAGE];

byte signature[48];
uint16_t gameMenuStartBank;

extern boolean hasMenu;
extern byte numGames;

void setup_GBSmart()
{
  // take from setup_GB
  // Set RST(PH0) to Input
  DDRH &= ~(1 << 0);
  // Activate Internal Pullup Resistors
  PORTH |= (1 << 0);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;

  // Set Control Pins to Output CS(PH3) WR(PH5) RD(PH6) AUDIOIN(PH4) RESET(PH0)
  DDRH |= (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
  // Output a high signal on all pins, pins are active low therefore everything is disabled now
  PORTH |= (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;

  delay(400);

  gbSmartRemapStartBank(0x00, 0x00, 0x00);

  getCartInfo_GB();

  for (byte i = 0; i < 0x30; i++)
    signature[i] = readByte_GB(0x0104 + i);

  gameMenuStartBank = 0x02;
  hasMenu = true;
  numGames = 0;

  ui.clearOutput();
  ui.flushOutput();
}

void gbSmartMenu() {
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
      F("GB Smart"), menu, ARRAY_LENGTH(menu), item_GameMenu);

    if (answer == item_GameMenu) {
      gbSmartGameMenu();
      gbSmartGameOptions();
    }
    else if (answer == item_FlashMenu) {
      mode = CartReaderMode::GBSmartFlash;
      gbSmartFlashMenu();
    }
    else if (answer == item_Back) {
      break;
    }
  }
}

void gbSmartGameOptions() {
  while (true) {
    const __FlashStringHelper *item_ReadGame = F("Read Game");
    const __FlashStringHelper *item_ReadSRAM = F("Read SRAM");
    const __FlashStringHelper *item_WriteSRAM = F("Write SRAM");
    const __FlashStringHelper *item_SwitchGame = F("Switch Game");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_ReadGame,
      item_ReadSRAM,
      item_WriteSRAM,
      item_SwitchGame,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("GB Smart Game Menu"), menu, ARRAY_LENGTH(menu), item_ReadGame);

    if (answer == item_ReadGame) {
      ui.clearOutput();
      String outputFilePath = getNextGBRomOutputPathAndPrintMessage(romName);
      readROM_GB(outputFilePath);
      compare_checksum_GB(outputFilePath);
    }
    else if (answer == item_ReadSRAM) {
      ui.clearOutput();
      readSRAM_GB();
    }
    else if (answer == item_WriteSRAM) {
      ui.clearOutput();
      String inputFilePath = fileBrowser(F("Select sav file"));
      writeSRAM_GB(inputFilePath);
      uint32_t writeErrors = verifySRAM_GB(inputFilePath);
      if (writeErrors == 0) {
        ui.printlnMsg(F("Verified OK"));
        ui.flushOutput();
      }
      else {
        ui.printMsg(F("Error: "));
        ui.printMsg(writeErrors);
        ui.printlnMsg(F(" bytes"));
        ui.printError(F("did not verify."));
      }
    }
    else if (answer == item_SwitchGame) {
      gameMenuStartBank = 0x02;
      gbSmartGameMenu();
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

void gbSmartGameMenu() {
  uint8_t gameSubMenu = 0;
gb_smart_load_more_games:
  if (gameMenuStartBank > 0xfe)
    gameMenuStartBank = 0x02;

  gbSmartGetGames();

  if (hasMenu) {
    String menuOptionsGBSmartGames[7];
    int i = 0;
    char gameTitleTemp[17];
    gameTitleTemp[16] = '\0';
    for (; i < numGames; i++) {
      strncpy(gameTitleTemp, gbSmartGames[i].title, 16);
      menuOptionsGBSmartGames[i] = gameTitleTemp;
    }
    menuOptionsGBSmartGames[i] = F("...");

    gameSubMenu = ui.askMultipleChoiceQuestion(F("Select Game"), menuOptionsGBSmartGames, i + 1, 0);

    if (gameSubMenu >= i)
      goto gb_smart_load_more_games;
  }
  else {
    gameSubMenu = 0;
  }

  // copy romname
  strcpy(romName, gbSmartGames[gameSubMenu].title);

  // select a game
  gbSmartRemapStartBank(gbSmartGames[gameSubMenu].start_bank, gbSmartGames[gameSubMenu].rom_size, gbSmartGames[gameSubMenu].sram_size);
  getCartInfo_GB();
  showCartInfo_GB();

  mode = CartReaderMode::GBSmartGame;
}

void gbSmartFlashMenu() {
  while (true) {
    const __FlashStringHelper *item_ReadFlash = F("Read Flash");
    const __FlashStringHelper *item_WriteFlash = F("Write Flash");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_ReadFlash,
      item_WriteFlash,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("GB Smart Flash Menu"), menu, ARRAY_LENGTH(menu), item_ReadFlash);

    if (answer == item_ReadFlash) {
      // read flash
      ui.clearOutput();
      String outputFilePath = getNextGbSmartFlashOutputPathAndPrintMessage();
      gbSmartReadFlash(outputFilePath);
    }
    else if (answer == item_WriteFlash) {
      ui.clearOutput();

      ui.printlnMsg(F("Attention"));
      ui.printlnMsg(F("This will erase your"));
      ui.printlnMsg(F("GB Smart Cartridge."));
      ui.printlnMsg(F(""));
      ui.printlnMsg(F("Press Button"));
      ui.printlnMsg(F("to continue"));
      ui.flushOutput();
      ui.waitForUserInput();

      ui.clearOutput();
      String inputFilePath = fileBrowser(F("Select 4MB file"));
      gbSmartWriteFlash(inputFilePath);
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

String getNextGbSmartFlashOutputPathAndPrintMessage() {
  return getNextOutputPathWithNumberedFilenameAndPrintMessage(F("GB/GBS"), F("GBS"), F(".bin"));
}

void gbSmartGetGames()
{
  static const byte menu_title[] = {0x47, 0x42, 0x31, 0x36, 0x4d};

  // reset remap setting
  gbSmartRemapStartBank(0x00, gbSmartRomSizeGB, gbSmartSramSizeGB);

  uint16_t i;
  uint8_t myByte, myLength;

  // check if contain menu
  hasMenu = true;
  dataIn_GB();
  for (i = 0; i < 5; i++)
  {
    if (readByte_GB(0x0134 + i) != menu_title[i])
    {
      hasMenu = false;
      break;
    }
  }

  if (hasMenu)
  {
    for (i = gameMenuStartBank, numGames = 0; i < gbSmartBanks && numGames < GB_SMART_GAMES_PER_PAGE; )
    {
      myLength = 0;

      // switch bank
      dataOut();
      writeByte_GB(0x2100, i);

      dataIn_GB();
      // read signature
      for (uint8_t j = 0x00; j < 0x30; j++)
      {
        if (readByte_GB(0x4104 + j) != signature[j])
        {
          i += 0x02;
          goto gb_smart_get_game_loop_end;
        }
      }

      for (uint8_t j = 0; j < 15; j++)
      {
        myByte = readByte_GB(0x4134 + j);

        if (((char(myByte) >= 0x30 && char(myByte) <= 0x39) ||
             (char(myByte) >= 0x41 && char(myByte) <= 0x7a)))
          gbSmartGames[numGames].title[myLength++] = char(myByte);
      }

      gbSmartGames[numGames].title[myLength] = 0x00;
      gbSmartGames[numGames].start_bank = i;
      gbSmartGames[numGames].rom_type = readByte_GB(0x4147);
      gbSmartGames[numGames].rom_size = readByte_GB(0x4148);
      gbSmartGames[numGames].sram_size = readByte_GB(0x4149);

      myByte = (2 << gbSmartGames[numGames].rom_size);
      i += myByte;
      numGames++;
gb_smart_get_game_loop_end:;
    }

    gameMenuStartBank = i;
  }
  else
  {
    dataIn_GB();
    for (uint8_t j = 0; j < 15; j++)
    {
      myByte = readByte_GB(0x0134 + j);

      if (((char(myByte) >= 0x30 && char(myByte) <= 0x39) ||
           (char(myByte) >= 0x41 && char(myByte) <= 0x7a)))
        gbSmartGames[0].title[myLength++] = char(myByte);
    }

    gbSmartGames[0].title[myLength] = 0x00;
    gbSmartGames[0].start_bank = 0x00;
    gbSmartGames[0].rom_type = readByte_GB(0x0147);
    gbSmartGames[0].rom_size = readByte_GB(0x0148);
    gbSmartGames[0].sram_size = readByte_GB(0x0149);

    numGames = 1;
    gameMenuStartBank = 0xfe;
  }
}

void gbSmartReadFlash(const String &outputFilePath)
{
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  // reset flash to read array state
  for (unsigned int i = 0x00; i < gbSmartBanks; i += gbSmartBanksPerFlashChip)
    gbSmartResetFlash(i);

  // remaps mmc to full access
  gbSmartRemapStartBank(0x00, gbSmartRomSizeGB, gbSmartSramSizeGB);

  // dump fixed bank 0x00
  dataIn_GB();
  for (uint16_t addr = 0x0000; addr <= 0x3fff; addr += 512)
  {
    for (uint16_t c = 0; c < 512; c++)
      sdBuffer[c] = readByte_GB(addr + c);

    outputFile.write(sdBuffer, 512);
  }

  // read rest banks
  for (uint16_t bank = 0x01; bank < gbSmartBanks; bank++)
  {
    dataOut();
    writeByte_GB(0x2100, bank);

    dataIn_GB();
    for (uint16_t addr = 0x4000; addr <= 0x7fff; addr += 512)
    {
      for (uint16_t c = 0; c < 512; c++)
        sdBuffer[c] = readByte_GB(addr + c);

      outputFile.write(sdBuffer, 512);
    }
  }

  // back to initial state
  writeByte_GB(0x2100, 0x01);

  outputFile.close();
  ui.printlnMsg("");
  ui.printlnMsg(F("Finished reading"));
  ui.flushOutput();
}

void gbSmartWriteFlash(const String &inputFilePath)
{
  for (unsigned int bank = 0x00; bank < gbSmartBanks; bank += gbSmartBanksPerFlashChip)
  {
    ui.clearOutput();

    ui.printMsg(F("Erasing..."));
    ui.flushOutput();

    gbSmartEraseFlash(bank);
    gbSmartResetFlash(bank);

    ui.printlnMsg(F("Done"));
    ui.printMsg(F("Blankcheck..."));
    ui.flushOutput();

    if (!gbSmartBlankCheckingFlash(bank))
      ui.printErrorAndAbort(F("Could not erase flash"), false);

    ui.printlnMsg(F("Passed"));
    ui.flushOutput();

    // write full chip
    gbSmartWriteFlash(inputFilePath, bank);

    // reset chip
    gbSmartWriteFlashByte(0x0000, 0xff);
  }

  ui.printMsg(F("Verifying..."));
  ui.flushOutput();

  writeErrors = gbSmartVerifyFlash(inputFilePath);
  if (writeErrors == 0)
  {
    ui.printlnMsg(F("OK"));
    ui.flushOutput();
  }
  else
  {
    ui.printMsg(F("Error: "));
    ui.printMsg(writeErrors);
    ui.printlnMsg(F(" bytes "));
    ui.printErrorAndAbort(F("did not verify."), false);
  }
}

void gbSmartWriteFlash(const String &inputFilePath, uint32_t start_bank)
{
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

  // switch to flash base bank
  gbSmartRemapStartBank(start_bank, gbSmartFlashSizeGB, gbSmartSramSizeGB);

  inputFile.seekCur((start_bank << 14));

  ui.printMsg(F("Writing Bank 0x"));
  ui.printMsg(start_bank, HEX);
  ui.printMsg(F("..."));
  ui.flushOutput();

  // handle bank 0x00 on 0x0000
  gbSmartWriteFlashFromMyFile(inputFile, 0x0000);

  // handle rest banks on 0x4000
  for (uint8_t bank = 0x01; bank < gbSmartBanksPerFlashChip; bank++)
  {
    dataOut();
    writeByte_GB(0x2100, bank);

    gbSmartWriteFlashFromMyFile(inputFile, 0x4000);
  }

  inputFile.close();
  ui.printlnMsg("");
}

void gbSmartWriteFlashFromMyFile(SafeSDFile &inputFile, uint32_t addr)
{
  for (uint16_t i = 0; i < 16384; i += 256)
  {
    inputFile.read(sdBuffer, 256);

    // sequence load to page
    dataOut();
    gbSmartWriteFlashByte(addr, 0xe0);
    gbSmartWriteFlashByte(addr, 0xff);
    gbSmartWriteFlashByte(addr, 0x00); // BCH should be 0x00

    // fill page buffer
    for (int d = 0; d < 256; d++)
      gbSmartWriteFlashByte(d, sdBuffer[d]);

    // start flashing page
    gbSmartWriteFlashByte(addr, 0x0c);
    gbSmartWriteFlashByte(addr, 0xff);
    gbSmartWriteFlashByte(addr + i, 0x00);  // BCH should be 0x00

    // waiting for finishing
    dataIn_GB();
    while ((readByte_GB(addr + i) & 0x80) == 0x00);
  }

  // blink LED
  PORTB ^= (1 << 4);
}

uint32_t gbSmartVerifyFlash(const String &filePath)
{
  uint32_t verified = 0;

  SafeSDFile inputFile = SafeSDFile::openForReading(filePath);

  // remaps mmc to full access
  gbSmartRemapStartBank(0x00, gbSmartRomSizeGB, gbSmartSramSizeGB);

  // verify bank 0x00
  dataIn_GB();
  for (uint16_t addr = 0x0000; addr <= 0x3fff; addr += 512)
  {
    inputFile.read(sdBuffer, 512);

    for (uint16_t c = 0; c < 512; c++)
    {
      if (readByte_GB(addr + c) != sdBuffer[c])
        verified++;
    }
  }

  // verify rest banks
  for (uint16_t bank = 0x01; bank < gbSmartBanks; bank++)
  {
    dataOut();
    writeByte_GB(0x2100, bank);

    dataIn_GB();
    for (uint16_t addr = 0x4000; addr <= 0x7fff; addr += 512)
    {
      inputFile.read(sdBuffer, 512);

      for (uint16_t c = 0; c < 512; c++)
      {
        if (readByte_GB(addr + c) != sdBuffer[c])
          verified++;
      }
    }
  }

  // back to initial state
  writeByte_GB(0x2100, 0x01);

  inputFile.close();

  return verified;
}

byte gbSmartBlankCheckingFlash(uint8_t flash_start_bank)
{
  gbSmartRemapStartBank(flash_start_bank, gbSmartFlashSizeGB, gbSmartSramSizeGB);

  // check first bank
  dataIn_GB();
  for (uint16_t addr = 0x0000; addr <= 0x3fff; addr++)
  {
    if (readByte_GB(addr) != 0xff)
      return 0;
  }

  // check rest banks
  for (uint16_t bank = 0x01; bank < gbSmartBanksPerFlashChip; bank++)
  {
    dataOut();
    writeByte_GB(0x2100, bank);

    dataIn_GB();
    for (uint16_t addr = 0x4000; addr <= 0x7fff; addr++)
    {
      if (readByte_GB(addr) != 0xff)
        return 0;
    }
  }

  return 1;
}

void gbSmartResetFlash(uint8_t flash_start_bank)
{
  gbSmartRemapStartBank(flash_start_bank, gbSmartFlashSizeGB, gbSmartSramSizeGB);

  dataOut();
  gbSmartWriteFlashByte(0x0, 0xff);
}

void gbSmartEraseFlash(uint8_t flash_start_bank)
{
  gbSmartRemapStartBank(flash_start_bank, gbSmartFlashSizeGB, gbSmartSramSizeGB);

  // handling first flash block
  dataOut();
  gbSmartWriteFlashByte(0x0000, 0x20);
  gbSmartWriteFlashByte(0x0000, 0xd0);

  dataIn_GB();
  while ((readByte_GB(0x0000) & 0x80) == 0x00);

  // blink LED
  PORTB ^= (1 << 4);

  // rest of flash block
  for (uint32_t ba = gbSmartBanksPerFlashBlock; ba < gbSmartBanksPerFlashChip; ba += gbSmartBanksPerFlashBlock)
  {
    dataOut();
    writeByte_GB(0x2100, ba);

    gbSmartWriteFlashByte(0x4000, 0x20);
    gbSmartWriteFlashByte(0x4000, 0xd0);

    dataIn_GB();
    while ((readByte_GB(0x4000) & 0x80) == 0x00);

    // blink LED
    PORTB ^= (1 << 4);
  }
}

void gbSmartWriteFlashByte(uint32_t myAddress, uint8_t myData)
{
  PORTF = myAddress & 0xff;
  PORTK = (myAddress >> 8) & 0xff;
  PORTC = myData;

  // wait for 62.5 x 4 = 250ns
  __asm__("nop\n\tnop\n\tnop\n\tnop\n\t");

  // Pull FLASH_WE (PH4) low
  PORTH &= ~(1 << 4);

  // pull low for another 250ns
  __asm__("nop\n\tnop\n\tnop\n\tnop\n\t");

  // Pull FLASH_WE (PH4) high
  PORTH |= (1 << 4);

  // pull high for another 250ns
  __asm__("nop\n\tnop\n\tnop\n\tnop\n\t");
}

// rom_start_bank = 0x00 means back to original state
void gbSmartRemapStartBank(uint8_t rom_start_bank, uint8_t rom_size, uint8_t sram_size)
{
  rom_start_bank &= 0xfe;

  dataOut();

  // clear base bank setting
  writeByte_GB(0x1000, 0xa5);
  writeByte_GB(0x7000, 0x00);
  writeByte_GB(0x1000, 0x98);
  writeByte_GB(0x2000, rom_start_bank);

  if (rom_start_bank > 1)
  {
    // start set new base bank
    writeByte_GB(0x1000, 0xa5);

    dataIn_GB();
    rom_start_bank = gbSmartGetResizeParam(rom_size, sram_size);

    dataOut();
    writeByte_GB(0x7000, rom_start_bank);
    writeByte_GB(0x1000, 0x98);

    writeByte_GB(0x2100, 0x01);
  }

  dataIn_GB();
}

// Get magic number for 0x7000 register.
// Use for setting correct rom and sram size
// Code logic is take from SmartCard32M V1.3 menu code,
// see 0x2db2 to 0x2e51 (0xa0 bytes)
uint8_t gbSmartGetResizeParam(uint8_t rom_size, uint8_t sram_size)
{
  if (rom_size < 0x0f)
  {
    rom_size &= 0x07;
    rom_size ^= 0x07;
  }
  else
  {
    rom_size = 0x01;
  }

  if (sram_size > 0)
  {
    if (sram_size > 1)
    {
      sram_size--;
      sram_size ^= 0x03;
      sram_size <<= 4;
      sram_size &= 0x30;
    }
    else
    {
      sram_size = 0x20; //  2KiB treat as 8KiB
    }
  }
  else
  {
    sram_size = 0x30; // no sram
  }

  return (sram_size | rom_size);
}
