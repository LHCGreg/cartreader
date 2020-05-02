//******************************************
// NES MODULE
//******************************************
// mostly copy&pasted from "Famicom Dumper" 2019-08-31 by skaman
// also based on "CoolArduino" by HardWareMan
// Pinout changes: LED and CIRAM_A10

#include <Arduino.h>
#include "NES.h"
#include "filebrowser.h"
#include "ui.h"
#include "globals.h"
#include "utils.h"
#include "SD.h"

// Contents
// [Supported Mappers]
// [Defines]
// [Variables]
// [Function Prototypes]
// [Setup]
// [Low Level Functions]
// [CRC Functions]
// [File Functions]
// [Config Functions]
// [ROM Functions]
// [RAM Functions]
// [Eeprom Functions]
// [NESmaker Flash Cart [SST 39SF40] Functions]

/******************************************
  [Supported Mappers]
 *****************************************/
// Supported Mapper Array (iNES Mapper #s)
// Format = {mapper,prglo,prghi,chrlo,chrhi,ramlo,ramhi}
static const byte PROGMEM mapsize [] = {
  0, 0, 1, 0, 1, 0, 2, // nrom                                                [sram r/w]
  1, 1, 5, 0, 5, 0, 3, // mmc1                                                [sram r/w]
  2, 3, 4, 0, 0, 0, 0, // uxrom
  3, 0, 1, 0, 3, 0, 0, // cnrom
  4, 1, 5, 0, 6, 0, 1, // mmc3/mmc6                                           [sram/prgram r/w]
  5, 3, 5, 5, 7, 0, 3, // mmc5                                                [sram r/w]
  7, 3, 4, 0, 0, 0, 0, // axrom
  9, 3, 3, 5, 5, 0, 0, // mmc2 (punch out)
  10, 3, 4, 4, 5, 1, 1, // mmc4                                               [sram r/w]
  13, 1, 1, 0, 0, 0, 0, // cprom (videomation)
  16, 3, 4, 5, 6, 0, 1, // bandai x24c02                                      [eep r/w]
  18, 3, 4, 5, 6, 0, 1, // jaleco ss8806                                      [sram r/w]
  19, 3, 4, 5, 6, 0, 1, // namco 106/163                                      [sram/prgram r/w]
  21, 4, 4, 5, 6, 0, 1, // vrc4a/vrc4c                                        [sram r/w]
  22, 3, 3, 5, 5, 0, 0, // vrc2a
  23, 3, 3, 5, 6, 0, 0, // vrc2b/vrc4e
  24, 4, 4, 5, 5, 0, 0, // vrc6a (akumajou densetsu)
  25, 3, 4, 5, 6, 0, 1, // vrc2c/vrc4b/vrc4d                                  [sram r/w]
  26, 4, 4, 5, 6, 1, 1, // vrc6b                                              [sram r/w]
  30, 4, 5, 0, 0, 0, 0, // unrom 512 (NESmaker) [UNLICENSED]
  32, 3, 4, 5, 5, 0, 0, // irem g-101
  33, 3, 4, 5, 6, 0, 0, // taito tc0190
  34, 3, 3, 0, 0, 0, 0, // bnrom [nina-1 NOT SUPPORTED]
  37, 4, 4, 6, 6, 0, 0, // (super mario bros + tetris + world cup)
  47, 4, 4, 6, 6, 0, 0, // (super spike vball + world cup)
  48, 3, 4, 6, 6, 0, 0, // taito tc0690
  65, 3, 4, 5, 6, 0, 0, // irem h-3001
  66, 2, 3, 2, 3, 0, 0, // gxrom/mhrom
  67, 3, 3, 5, 5, 0, 0, // sunsoft 3
  68, 3, 3, 5, 6, 0, 1, // sunsoft 4                                          [sram r/w]
  69, 3, 4, 5, 6, 0, 1, // sunsoft fme-7/5a/5b                                [sram r/w]
  70, 3, 3, 5, 5, 0, 0, // bandai
  71, 2, 4, 0, 0, 0, 0, // camerica/codemasters [UNLICENSED]
  72, 3, 3, 5, 5, 0, 0, // jaleco jf-17
  73, 3, 3, 0, 0, 0, 0, // vrc3 (salamander)
  75, 3, 3, 5, 5, 0, 0, // vrc1
  76, 3, 3, 5, 5, 0, 0, // namco 109 variant (megami tensei: digital devil story)
  77, 3, 3, 3, 3, 0, 0, // (napoleon senki)
  78, 3, 3, 5, 5, 0, 0, // irem 74hc161/32
  80, 3, 3, 5, 6, 0, 1, // taito x1-005                                       [prgram r/w]
  82, 3, 3, 5, 6, 0, 1, // taito x1-017                                       [prgram r/w]
  85, 3, 5, 0, 5, 0, 1, // vrc7                                               [sram r/w]
  86, 3, 3, 4, 4, 0, 0, // jaleco jf-13 (moero pro yakyuu)
  87, 0, 1, 2, 3, 0, 0,
  88, 3, 3, 5, 5, 0, 0, // namco (dxrom variant)
  89, 3, 3, 5, 5, 0, 0, // sunsoft 2 variant (tenka no goikenban: mito koumon)
  92, 4, 4, 5, 5, 0, 0, // jaleco jf-19/jf-21
  93, 3, 3, 0, 0, 0, 0, // sunsoft 2
  94, 3, 3, 0, 0, 0, 0, // hvc-un1rom (senjou no ookami)
  95, 3, 3, 3, 3, 0, 0, // namcot-3425 (dragon buster)
  96, 3, 3, 0, 0, 0, 0, // (oeka kids)
  97, 4, 4, 0, 0, 0, 0, // irem tam-s1 (kaiketsu yanchamaru)
  105, 4, 4, 0, 0, 0, 0, // (nintendo world Championships 1990) [UNTESTED]
  118, 3, 4, 5, 5, 0, 1, // txsrom/mmc3                                       [sram r/w]
  119, 3, 3, 4, 4, 0, 0, // tqrom/mmc3
  140, 3, 3, 3, 5, 0, 0, // jaleco jf-11/jf-14
  152, 2, 3, 5, 5, 0, 0,
  153, 5, 5, 0, 0, 1, 1, // (famicom jump ii)                                 [sram r/w]
  154, 3, 3, 5, 5, 0, 0, // namcot-3453 (devil man)
  155, 3, 3, 3, 5, 0, 1, // mmc1 variant                                      [sram r/w]
  159, 3, 4, 5, 6, 1, 1, // bandai x24c01                                     [eep r/w]
  180, 3, 3, 0, 0, 0, 0, // unrom variant (crazy climber)
  184, 1, 1, 2, 3, 0, 0, // sunsoft 1
  185, 0, 1, 1, 1, 0, 0, // cnrom lockout
  206, 1, 3, 2, 4, 0, 0, // dxrom
  207, 4, 4, 5, 5, 0, 0, // taito x1-005 variant (fudou myouou den)
  210, 3, 5, 5, 6, 0, 0, // namco 175/340
};

const uint8_t fieldsPerMapper = 7;

/******************************************
  [Defines]
 *****************************************/
#define ROMSEL_HI PORTF |= (1<<1)
#define ROMSEL_LOW PORTF &= ~(1<<1)
#define PHI2_HI PORTF |= (1<<0)
#define PHI2_LOW PORTF &= ~(1<<0)
#define PRG_READ PORTF |= (1<<7)
#define PRG_WRITE PORTF &= ~(1<<7)
#define CHR_READ_HI PORTF |= (1<<5)
#define CHR_READ_LOW PORTF &= ~(1<<5)
#define CHR_WRITE_HI PORTF |= (1<<2)
#define CHR_WRITE_LOW PORTF &= ~(1<<2)

// RGB LED COMMON ANODE
#define LED_RED_OFF PORTB |= (1<<6)
#define LED_RED_ON PORTB &= ~(1<<6)
#define LED_GREEN_OFF PORTB |= (1<<5)
#define LED_GREEN_ON PORTB &= ~(1<<5)
#define LED_BLUE_OFF PORTB |= (1<<4)
#define LED_BLUE_ON PORTB &= ~(1<<4)

#define MODE_READ { PORTK = 0xFF; DDRK = 0; }
#define MODE_WRITE DDRK = 0xFF

/******************************************
  [Variables]
*****************************************/
// Mapper
byte mapcount = (sizeof(mapsize) / sizeof(mapsize[0])) / 7;
boolean mapfound = false;
byte mapselect;

int PRG[] = {16, 32, 64, 128, 256, 512};
byte prglo = 0; // Lowest Entry
byte prghi = 5; // Highest Entry

int CHR[] = {0, 8, 16, 32, 64, 128, 256, 512};
byte chrlo = 0; // Lowest Entry
byte chrhi = 7; // Highest Entry

byte RAM[] = {0, 8, 16, 32};
byte ramlo = 0; // Lowest Entry
byte ramhi = 3; // Highest Entry

int banks;
int prg;
int chr;
byte ram;
boolean vrc4e = false;
byte prgchk0;
byte prgchk1;
boolean mmc6 = false;
byte prgchk2;
byte prgchk3;
unsigned int eepsize;
byte bytecheck;
byte firstbyte;

char flashID[5];
boolean flashfound = false; // NESmaker 39SF040 Flash Cart

// Cartridge Config
byte mapper;
byte prgsize;
byte chrsize;
byte ramsize;

/******************************************
   [Function prototypes]
 *****************************************/
void nesCartMenu();
void nesCartWriteMenu();
String getOutputFolderPath();
void setup_NES();
static void set_address(unsigned int address);
void resetROM();
static void write_reg_byte(unsigned int address, uint8_t data);
void outputNES(const String &outputFolderPath);
void setMapper();
void checkMapperSize();
void setPRGSize();
void setCHRSize();
void setRAMSize();
void checkStatus_NES();
void readPRG(const String &outputFolderPath);
void readCHR(const String &outputFolderPath);
void readRAM(const String &outputFolderPath);
void writeRAM();
void EepromREAD(byte address);
void EepromWRITE(byte address);
void NESmaker_ID();
void writeFLASH();

/******************************************
  Menu
*****************************************/

// NES start menu
void nesMenu() {
  mode = CartReaderMode::NES;
  ui.clearOutput();
  ui.flushOutput();
  setup_NES();
  checkStatus_NES();
  nesCartMenu();
}

void nesCartMenu() {
  while (true) {
    const __FlashStringHelper *item_SelectMapper = F("Select Mapper");
    const __FlashStringHelper *item_ReadEverything = F("Read Complete Cart");
    const __FlashStringHelper *item_ReadPrg= F("Read PRG");
    const __FlashStringHelper *item_ReadChr = F("Read CHR");
    const __FlashStringHelper *item_ReadRam = F("Read RAM");
    const __FlashStringHelper *item_WriteOptions = F("Write Options");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_SelectMapper,
      item_ReadEverything,
      item_ReadPrg,
      item_ReadChr,
      item_ReadRam,
      item_WriteOptions,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("NES CART READER"), menu, ARRAY_LENGTH(menu), item_SelectMapper);

    // wait for user choice to come back from the question box menu
    if (answer == item_SelectMapper) {
      setMapper();
      checkMapperSize();
      setPRGSize();
      setCHRSize();
      setRAMSize();
    }
    else if (answer == item_ReadEverything) {
      String outputFolderPath = getOutputFolderPath();
      readPRG(outputFolderPath);
      delay(2000);
      readCHR(outputFolderPath);
      delay(2000);
      outputNES(outputFolderPath);
      delay(2000);
      readRAM(outputFolderPath);
      delay(2000);
      resetROM();
    }
    else if (answer == item_ReadPrg) {
      String outputFolderPath = getOutputFolderPath();
      readPRG(outputFolderPath);
      resetROM();
      ui.waitForUserInput();
    }
    else if (answer == item_ReadChr) {
      String outputFolderPath = getOutputFolderPath();
      readCHR(outputFolderPath);
      resetROM();
      ui.waitForUserInput();
    }
    else if (answer == item_ReadRam) {
      String outputFolderPath = getOutputFolderPath();
      readRAM(outputFolderPath);
      resetROM();
      ui.waitForUserInput();
    }
    else if (answer == item_WriteOptions) {
      nesCartWriteMenu();
      ui.waitForUserInput();
    }
    else if (answer == item_Back) {
      break;
    }
  }
}

void nesCartWriteMenu() {
  while (true) {
    const __FlashStringHelper *writeMenuItem_RAM = F("Write RAM");
    const __FlashStringHelper *writeMenuItem_Flash = F("Write FLASH");
    const __FlashStringHelper *writeMenuItem_Return = F("Return to Main Menu");
    const __FlashStringHelper *writeMenu[] = {
      writeMenuItem_RAM,
      writeMenuItem_Flash,
      writeMenuItem_Return,
    };

    const __FlashStringHelper *writeAnswer = ui.askMultipleChoiceQuestion(
      F("WRITE OPTIONS MENU"), writeMenu, ARRAY_LENGTH(writeMenu), writeMenuItem_RAM);

    if (writeAnswer == writeMenuItem_RAM) {
      writeRAM();
      resetROM();
      ui.waitForUserInput();
      break;
    }
    else if (writeAnswer == writeMenuItem_Flash) {
      if (mapper == 30) {
        writeFLASH();
      }
      resetROM();
      ui.waitForUserInput();
      break;
    }
    else if (writeAnswer == writeMenuItem_Return) {
      break; // return from this function to the main NES menu
    }
  }
}

String getOutputFolderPath() {
  int16_t folderNumber = loadFolderNumber();
  saveFolderNumber(folderNumber + 1);
  String folderPath(F("/NES/CART/"));
  folderPath.concat(folderNumber);
  return folderPath;
}

/******************************************
   [Setup]
 *****************************************/
void setup_NES() {
  // CPU R/W, IRQ, PPU /RD, PPU /A13, CIRAM /CE, PPU /WR, /ROMSEL, PHI2
  DDRF = 0b10110111;
  // CPU R/W, IRQ, PPU /RD, PPU /A13, CIRAM /CE, PPU /WR, /ROMSEL, PHI2
  PORTF = 0b11111111;

  // A0-A7 to Output
  DDRL = 0xFF;
  // A8-A14 to Output
  DDRA = 0xFF;

  // Set CIRAM A10 to Input
  DDRC &= ~(1 << 2);
  // Activate Internal Pullup Resistors
  PORTC |= (1 << 2);

  // Set D0-D7 to Input
  PORTK = 0xFF;
  DDRK = 0;

  set_address(0);
  LED_RED_OFF;
  LED_GREEN_OFF;
  LED_BLUE_OFF;
}

/******************************************
   [Low Level Functions]
 *****************************************/

static void set_address(unsigned int address) {
  unsigned char l = address & 0xFF;
  unsigned char h = address >> 8;
  PORTL = l;
  PORTA = h;

  // PPU /A13
  if ((address >> 13) & 1)
    PORTF &= ~(1 << 4);
  else
    PORTF |= 1 << 4;
}

static void set_romsel(unsigned int address) {
  if (address & 0x8000) {
    ROMSEL_LOW;
  } else {
    ROMSEL_HI;
  }
}

static unsigned char read_prg_byte(unsigned int address) {
  MODE_READ;
  PRG_READ;
  set_address(address);
  PHI2_HI;
  set_romsel(address);
  _delay_us(1);
  return PINK;
}

static unsigned char read_chr_byte(unsigned int address) {
  MODE_READ;
  PHI2_HI;
  ROMSEL_HI;
  set_address(address);
  CHR_READ_LOW;
  _delay_us(1);
  uint8_t result = PINK;
  CHR_READ_HI;
  return result;
}

static void write_prg_byte(unsigned int address, uint8_t data) {
  PHI2_LOW;
  ROMSEL_HI;
  MODE_WRITE;
  PRG_WRITE;
  PORTK = data;

  set_address(address); // PHI2 low, ROMSEL always HIGH
  //  _delay_us(1);
  PHI2_HI;
  //_delay_us(10);
  set_romsel(address); // ROMSEL is low if need, PHI2 high
  _delay_us(1); // WRITING
  //_delay_ms(1); // WRITING
  // PHI2 low, ROMSEL high
  PHI2_LOW;
  _delay_us(1);
  ROMSEL_HI;
  // Back to read mode
  //  _delay_us(1);
  PRG_READ;
  MODE_READ;
  set_address(0);
  // Set phi2 to high state to keep cartridge unreseted
  //  _delay_us(1);
  PHI2_HI;
  //  _delay_us(1);
}

void resetROM() {
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
}

void write_mmc1_byte(unsigned int address, uint8_t data) { // write loop for 5 bit register
  if (address >= 0xE000) {
    for (int i = 0; i < 5; i++) {
      write_reg_byte(address, data >> i); // shift 1 bit into temp register [WRITE RAM SAFE]
    }
  }
  else {
    for (int j = 0; j < 5; j++) {
      write_prg_byte(address, data >> j); // shift 1 bit into temp register
    }
  }
}

// REFERENCE FOR REGISTER WRITE TO 0xE000/0xF000
// PORTF 7 = CPU R/W = 0
// PORTF 6 = /IRQ = 1
// PORTF 5 = PPU /RD = 1
// PORTF 4 = PPU /A13 = 1
// PORTF 3 = CIRAM /CE = 1
// PORTF 2 = PPU /WR = 1
// PORTF 1 = /ROMSEL
// PORTF 0 = PHI2 (M2)

// WRITE RAM SAFE TO REGISTERS 0xE000/0xF000
static void write_reg_byte(unsigned int address, uint8_t data) { // FIX FOR MMC1 RAM CORRUPTION
  PHI2_LOW;
  ROMSEL_HI; // A15 HI = E000
  MODE_WRITE;
  PRG_WRITE; // CPU R/W LO
  PORTK = data;

  set_address(address); // PHI2 low, ROMSEL always HIGH
  // DIRECT PIN TO PREVENT RAM CORRUPTION
  // DIFFERENCE BETWEEN M2 LO AND ROMSEL HI MUST BE AROUND 33ns
  // IF TIME IS GREATER THAN 33ns THEN WRITES TO 0xE000/0xF000 WILL CORRUPT RAM AT 0x6000/0x7000
  PORTF = 0b01111101; // ROMSEL LO/M2 HI
  PORTF = 0b01111110; // ROMSEL HI/M2 LO
  _delay_us(1);
  // Back to read mode
  PRG_READ;
  MODE_READ;
  set_address(0);
  // Set phi2 to high state to keep cartridge unreseted
  PHI2_HI;
}

static void write_ram_byte(unsigned int address, uint8_t data) { // Mapper 19 (Namco 106/163) WRITE RAM SAFE ($E000-$FFFF)
  PHI2_LOW;
  ROMSEL_HI;
  MODE_WRITE;
  PRG_WRITE;
  PORTK = data;

  set_address(address); // PHI2 low, ROMSEL always HIGH
  PHI2_HI;
  ROMSEL_LOW; // SET /ROMSEL LOW OTHERWISE CORRUPTS RAM
  _delay_us(1); // WRITING
  // PHI2 low, ROMSEL high
  PHI2_LOW;
  _delay_us(1);
  ROMSEL_HI;
  // Back to read mode
  PRG_READ;
  MODE_READ;
  set_address(0);
  // Set phi2 to high state to keep cartridge unreseted
  PHI2_HI;
}

static void write_wram_byte(unsigned int address, uint8_t data) { // Mapper 5 (MMC5) RAM
  PHI2_LOW;
  ROMSEL_HI;
  set_address(address);
  PORTK = data;

  _delay_us(1);
  MODE_WRITE;
  PRG_WRITE;
  PHI2_HI;
  _delay_us(1); // WRITING
  PHI2_LOW;
  ROMSEL_HI;
  // Back to read mode
  PRG_READ;
  MODE_READ;
  set_address(0);
  // Set phi2 to high state to keep cartridge unreseted
  PHI2_HI;
}

/******************************************
   [CRC Functions]
 *****************************************/
char tempCRC[9];

inline uint32_t updateCRC32(uint8_t ch, uint32_t crc) {
  uint32_t idx = ((crc) ^ (ch)) & 0xff;
  uint32_t tab_value = pgm_read_dword(crc_32_tab + idx);
  return tab_value ^ ((crc) >> 8);
}

uint32_t crc32(SafeSDFile &file, uint32_t &charcnt) {
  uint32_t oldcrc32 = 0xFFFFFFFF;
  charcnt = 0;
  while (file.bytesAvailable() > 0) {
    file.read(sdBuffer, 512);
    for (int x = 0; x < 512; x++) {
      uint8_t c = sdBuffer[x];
      charcnt++;
      oldcrc32 = updateCRC32(c, oldcrc32);
    }
  }
  return ~oldcrc32;
}

uint32_t crc32EEP(SafeSDFile &file, uint32_t &charcnt) {
  uint32_t oldcrc32 = 0xFFFFFFFF;
  charcnt = 0;
  while (file.bytesAvailable() > 0) {
    file.read(sdBuffer, 128);
    for (int x = 0; x < 128; x++) {
      uint8_t c = sdBuffer[x];
      charcnt++;
      oldcrc32 = updateCRC32(c, oldcrc32);
    }
  }
  return ~oldcrc32;
}

void calcCRC(const String &checkFilePath, uint32_t filesize) {
  uint32_t crc;
  SafeSDFile crcFile = SafeSDFile::openForReading(checkFilePath);
  if (filesize < 1024)
    crc = crc32EEP(crcFile, filesize);
  else
    crc = crc32(crcFile, filesize);
  crcFile.close();
  sprintf(tempCRC, "%08lX", crc);

  ui.printMsg(F("CRC: "));
  ui.printlnMsg(tempCRC);
  ui.flushOutput();
}

/******************************************
   [File Functions]
 *****************************************/
void outputNES(const String &outputFolderPath) {
  ui.clearOutput();

  LED_RED_ON;
  LED_GREEN_ON;
  LED_BLUE_ON;
  String prgFilePath = pathJoin(outputFolderPath, F("PRG.bin"));
  SafeSDFile prgFile = SafeSDFile::openForReading(prgFilePath);

  String cartFilePath = pathJoin(outputFolderPath, F("CART.nes"));
  SafeSDFile cartFile = SafeSDFile::openForCreating(cartFilePath);

  size_t n;
  while ((n = prgFile.read(sdBuffer, sizeof(sdBuffer))) > 0) {
    cartFile.write(sdBuffer, n);
  }
  prgFile.close();

  String chrFilePath = pathJoin(outputFolderPath, F("CHR.bin"));
  if (fileExists(chrFilePath)) {
    SafeSDFile chrFile = SafeSDFile::openForReading(chrFilePath);
    while ((n = chrFile.read(sdBuffer, sizeof(sdBuffer))) > 0) {
      cartFile.write(sdBuffer, n);
    }
    chrFile.close();
  }
  cartFile.close();

  ui.printlnMsg(F("NES FILE OUTPUT!"));
  ui.printlnMsg(F(""));
  ui.flushOutput();

  calcCRC(cartFilePath, (prg + chr) * 1024);
  LED_RED_OFF;
  LED_GREEN_OFF;
  LED_BLUE_OFF;
}

/******************************************
   [Config Functions]
 *****************************************/
void setMapper() {
  // Read stored mapper
  byte storedMapper = loadNESMapperNumber();

  byte newmapper = storedMapper;
  if (newmapper > 220) {
    newmapper = 0;
  }

  String prompt;
  if (ui.supportsLargeMessages()) {
    prompt = String(F("SUPPORTED MAPPERS:"));
    for (byte i = 0; i < mapcount; i++) {
      uint16_t index = i * fieldsPerMapper;
      byte mapselect = pgm_read_byte(mapsize + index);
      prompt += F("[");
      prompt += mapselect;
      prompt += F("]");
      if (i < mapcount - 1) {
        if ((i != 0) && ((i + 1) % 10 == 0)) {
          prompt += F("\r\n");
        }
        else {
          prompt += F("\t");
        }
      }
      else {
        prompt += F("\r\n");
      }
    }

    prompt += F("Enter Mapper: ");
  }
  else {
    prompt = String(F("Enter Mapper: "));
  }

  bool validMapper = false;
  while (!validMapper) {
    newmapper = (byte) ui.readNumber(3, newmapper, 220, prompt, F("Mapper not supported"));

    // Check if valid
    byte mapcount = ARRAY_LENGTH(mapsize) / fieldsPerMapper;
    for (byte currMaplist = 0; currMaplist < mapcount; currMaplist++) {
      if (pgm_read_byte(mapsize + currMaplist * fieldsPerMapper) == newmapper) {
        validMapper = true;
      }
    }

    if (!validMapper) {
      errorLvl = 1;
      ui.displayMessage(F("Mapper not supported"));
    }
  }

  saveNESMapperNumber(newmapper);
  mapper = newmapper;
}

void checkMapperSize() {
  for (int i = 0; i < mapcount; i++) {
    int index = i * 7;
    byte mapcheck = pgm_read_byte(mapsize + index);
    if (mapcheck == mapper) {
      prglo = pgm_read_byte(mapsize + index + 1);
      prghi = pgm_read_byte(mapsize + index + 2);
      chrlo = pgm_read_byte(mapsize + index + 3);
      chrhi = pgm_read_byte(mapsize + index + 4);
      ramlo = pgm_read_byte(mapsize + index + 5);
      ramhi = pgm_read_byte(mapsize + index + 6);
      break;
    }
  }
}

void setPRGSize() {
  byte newprgsize;
  // If only one possible PRG size for this mapper, use it, no need to ask for user input.
  if (prglo == prghi) {
    newprgsize = prglo;
  }
  else {
    // Have user choose a size from [prglo, ..., prghi] from the predetermined sizes in the PRG array
    uint8_t numChoices = prghi - prglo + 1;
    String prgSizeChoices[numChoices];
    for (uint8_t prgIndex = prglo; prgIndex <= prghi; prgIndex++) {
      prgSizeChoices[prgIndex - prglo] = String(PRG[prgIndex]) + F(" kB");
    }

    uint8_t choiceIndex = ui.askMultipleChoiceQuestion(F("PRG Size:"), prgSizeChoices, numChoices, 0);
    newprgsize = prglo + choiceIndex;
  }

  saveNESPRG(newprgsize);
  prgsize = newprgsize;
}

void setCHRSize() {
  byte newchrsize;
  // If only one possible CHR size for this mapper, use it, no need to ask for user input.
  if (chrlo == chrhi) {
    newchrsize = chrlo;
  }
  else {
    // Have user choose a size from [chrlo, ..., chrhi] from the predetermined sizes in the CHR array
    uint8_t numChoices = chrhi - chrlo + 1;
    String chrSizeChoices[numChoices];
    for (uint8_t chrIndex = chrlo; chrIndex <= chrhi; chrIndex++) {
      chrSizeChoices[chrIndex - chrlo] = String(CHR[chrIndex]) + F(" kB");
    }

    uint8_t choiceIndex = ui.askMultipleChoiceQuestion(F("CHR Size:"), chrSizeChoices, numChoices, 0);
    newchrsize = chrlo + choiceIndex;
  }

  saveNESCHR(newchrsize);
  chrsize = newchrsize;
}

void setRAMSize() {
  byte newramsize;
  // If only one possible RAM size for this mapper, use it, no need to ask for user input.
  if (ramlo == ramhi) {
    newramsize = ramlo;
  }
  else {
    const __FlashStringHelper *prompt;
    if (mapper == 16 || mapper == 159) {
      prompt = F("EEPROM Size:");
    }
    else {
      prompt = F("RAM Size:");
    }

    uint8_t numChoices = ramhi - ramlo + 1;
    String ramSizeChoices[numChoices];
    for (uint8_t ramIndex = ramlo; ramIndex <= ramhi; ramIndex++) {
      if (mapper == 0) {
        String ramSizeString = String(RAM[ramIndex] / 4);
        ramSizeString.concat(F(" kB"));
        ramSizeChoices[ramIndex - ramlo] = ramSizeString;
      }
      else if (mapper == 16) {
        String ramSizeString = String(static_cast<uint16_t>(RAM[ramIndex]) * 32);
        ramSizeString.concat(F(" B"));
        ramSizeChoices[ramIndex - ramlo] = ramSizeString;
      }
      else if (mapper == 19) {
        if (ramIndex == 2) {
          ramSizeChoices[ramIndex - ramlo] = F("128 B");
        }
        else {
          String ramSizeString = String(RAM[ramIndex]);
          ramSizeString.concat(F(" kB"));
          ramSizeChoices[ramIndex - ramlo] = ramSizeString;
        }
      }
      else if (mapper == 159 || mapper == 80) {
        String ramSizeString = String(static_cast<uint16_t>(RAM[ramIndex] * 16));
        ramSizeString.concat(F(" B"));
        ramSizeChoices[ramIndex - ramlo] = ramSizeString;
      }
      else if (mapper == 82) {
        String ramSizeString = String(ramIndex * 5);
        ramSizeString.concat(F(" kB"));
        ramSizeChoices[ramIndex - ramlo] = ramSizeString;
      }
      else {
        String ramSizeString = String(RAM[ramIndex]);
        ramSizeString.concat(F(" kB"));
        ramSizeChoices[ramIndex - ramlo] = ramSizeString;
      }
    }

    uint8_t choiceIndex = ui.askMultipleChoiceQuestion(prompt, ramSizeChoices, numChoices, 0);
    newramsize = ramlo + choiceIndex;
  }

  saveNESRAM(newramsize);
  ramsize = newramsize;
}

// MMC6 Detection
// Mapper 4 includes both MMC3 AND MMC6
// RAM is mapped differently between MMC3 and MMC6
void checkMMC6() { // Detect MMC6 Carts - read PRG 0x3E00A ("STARTROPICS")
  write_prg_byte(0x8000, 6); // PRG Bank 0 ($8000-$9FFF)
  write_prg_byte(0x8001, 0x1F); // 0x3E000
  prgchk0 = read_prg_byte(0x800A);
  prgchk1 = read_prg_byte(0x800B);
  prgchk2 = read_prg_byte(0x800C);
  prgchk3 = read_prg_byte(0x800D);
  if ((prgchk0 == 0x53) && (prgchk1 == 0x54) && (prgchk2 == 0x41) && (prgchk3 == 0x52))
    mmc6 = true; // MMC6 Cart
}

void checkStatus_NES() {
  mapper = loadNESMapperNumber();
  prgsize = loadNESPRG();
  chrsize = loadNESCHR();
  ramsize = loadNESRAM();
  prg = (int_pow(2, prgsize)) * 16;
  if (chrsize == 0)
    chr = 0; // 0K
  else
    chr = (int_pow(2, chrsize)) * 4;
  if (ramsize == 0)
    ram = 0; // 0K
  else if (mapper == 82)
    ram = 5; // 5K
  else
    ram = (int_pow(2, ramsize)) * 4;

  // Mapper Variants
  // Identify variant for use across multiple functions
  if (mapper == 4) { // Check for MMC6/MMC3
    checkMMC6();
    if (mmc6)
      ram = 1; // 1K
  }
  else if (mapper == 30) // Check for Flashable/Non-Flashable
    NESmaker_ID(); // Flash ID

  ui.clearOutput();
  ui.printlnMsg(F("NES CART READER"));
  ui.printlnMsg(F("CURRENT SETTINGS"));
  ui.printlnMsg(F(""));
  ui.printMsg(F("MAPPER:   "));
  ui.printlnMsg(mapper);
  ui.printMsg(F("PRG SIZE: "));
  ui.printMsg(prg);
  ui.printlnMsg(F("K"));
  ui.printMsg(F("CHR SIZE: "));
  ui.printMsg(chr);
  ui.printlnMsg(F("K"));
  ui.printMsg(F("RAM SIZE: "));
  if (mapper == 0) {
    ui.printMsg(ram / 4);
    ui.printlnMsg(F("K"));
  }
  else if ((mapper == 16) || (mapper == 80) || (mapper == 159)) {
    if (mapper == 16)
      ui.printMsg(ram * 32);
    else
      ui.printMsg(ram * 16);
    ui.printlnMsg(F("B"));
  }
  else if (mapper == 19) {
    if (ramsize == 2)
      ui.printlnMsg(F("128B"));
    else {
      ui.printMsg(ram);
      ui.printlnMsg(F("K"));
    }
  }
  else {
    ui.printMsg(ram);
    ui.printlnMsg(F("K"));
  }
  ui.flushOutput();
  ui.waitForUserInput();
}

/******************************************
   [ROM Functions]
 *****************************************/
void dumpPRG(word base, word address, SafeSDFile &sdFile) {
  for (int x = 0; x < 512; x++) {
    sdBuffer[x] = read_prg_byte(base + address + x);
  }
  sdFile.write(sdBuffer, 512);
}

void dumpCHR(word address, SafeSDFile &sdFile) {
  for (int x = 0; x < 512; x++) {
    sdBuffer[x] = read_chr_byte(address + x);
  }
  sdFile.write(sdBuffer, 512);
}

void dumpMMC5RAM(word base, word address, SafeSDFile &sdFile) { // MMC5 SRAM DUMP - PULSE M2 LO/HI
  for (int x = 0; x < 512; x++) {
    PHI2_LOW;
    sdBuffer[x] = read_prg_byte(base + address + x);
  }
  sdFile.write(sdBuffer, 512);
}

void writeMMC5RAM(word base, word address, SafeSDFile &sdFile) { // MMC5 SRAM WRITE
  sdFile.read(sdBuffer, 512);
  for (int x = 0; x < 512; x++) {
    do {
      write_prg_byte(0x5102, 2); // PRG RAM PROTECT1
      write_prg_byte(0x5103, 1); // PRG RAM PROTECT2
      write_wram_byte(base + address + x, sdBuffer[x]);
      bytecheck = read_prg_byte(base + address + x);
    }
    while (bytecheck != sdBuffer[x]); // CHECK WRITTEN BYTE
  }
  write_prg_byte(0x5102, 0); // PRG RAM PROTECT1
  write_prg_byte(0x5103, 0); // PRG RAM PROTECT2
}

void readPRG(const String &outputFolderPath) {
  ui.clearOutput();
  ui.flushOutput();

  LED_BLUE_ON;
  set_address(0);
  _delay_us(1);
  String prgFilePath = pathJoin(outputFolderPath, F("PRG.bin"));
  SafeSDFile prgFile = SafeSDFile::openForCreating(prgFilePath);
  word base = 0x8000;
  switch (mapper) {
    case 0:
    case 3:
    case 13:
    case 87: // 16K/32K
    case 184: // 32K
    case 185: // 16K/32K
      for (word address = 0; address < ((prgsize * 0x4000U) + 0x4000U); address += 512) { // 16K or 32K
        dumpPRG(base, address, prgFile);
      }
      break;

    case 1:
    case 155: // 32K/64K/128K/256K/512K
      banks = int_pow(2, prgsize) - 1;
      for (int i = 0; i < banks; i++) { // 16K Banks ($8000-$BFFF)
        write_prg_byte(0x8000, 0x80); // Clear Register
        write_mmc1_byte(0x8000, 0x0C); // Switch 16K Bank ($8000-$BFFF) + Fixed Last Bank ($C000-$FFFF)
        if (prgsize > 4) // 512K
          write_mmc1_byte(0xA000, 0x00); // Reset 512K Flag for Lower 256K
        if (i > 15) // Switch Upper 256K
          write_mmc1_byte(0xA000, 0x10); // Set 512K Flag
        write_mmc1_byte(0xE000, i);
        for (word address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(base, address, prgFile);
        }
      }
      for (word address = 0x4000; address < 0x8000; address += 512) { // Final Bank ($C000-$FFFF)
        dumpPRG(base, address, prgFile);
      }
      break;

    case 2: // 128K/256K
      for (int i = 0; i < 8; i++) { // 128K/256K
        write_prg_byte(0x8000, i);
        for (word address = 0x0; address < (((prgsize - 3U) * 0x4000U) + 0x4000U); address += 512) {
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 4:
    case 47:
    case 118:
    case 119:
      banks = ((int_pow(2, prgsize) * 2)) - 2;  // Set Number of Banks
      if (mapper == 47)
        write_prg_byte(0xA001, 0x80); // Block Register - PRG RAM Chip Enable, Writable
      for (int i = 0; i < banks; i += 2) { // 32K/64K/128K/256K/512K
        if (mapper == 47) {
          if (i == 0)
            write_prg_byte(0x6000, 0); // Switch to Lower Block
          else if (i == 16)
            write_prg_byte(0x6000, 1); // Switch to Upper Block
        }
        write_prg_byte(0x8000, 6); // PRG Bank 0 ($8000-$9FFF)
        write_prg_byte(0x8001, i);
        write_prg_byte(0x8000, 7); // PRG Bank 1 ($A000-$BFFF)
        write_prg_byte(0x8001, i + 1);
        for (word address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(base, address, prgFile);
        }
      }
      for (word address = 0x4000; address < 0x8000; address += 512) { // Final 2 Banks ($C000-$FFFF)
        dumpPRG(base, address, prgFile);
      }
      break;

    case 5: // 128K/256K/512K
      banks = int_pow(2, prgsize) * 2;
      write_prg_byte(0x5100, 3); // 8K PRG Banks
      for (int i = 0; i < banks; i += 2) { // 128K/256K/512K
        write_prg_byte(0x5114, i | 0x80);
        write_prg_byte(0x5115, (i + 1) | 0x80);
        for (word address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 7: // 128K/256K
    case 34:
    case 77:
    case 96: // 128K
      banks = int_pow(2, prgsize) / 2;
      for (int i = 0; i < banks; i++) { // 32K Banks
        write_prg_byte(0x8000, i);
        for (word address = 0x0; address < 0x8000; address += 512) { // 32K Banks ($8000-$FFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 9: // 128K
      for (int i = 0; i < 13; i++) { // 16-3 = 13 = 128K
        write_prg_byte(0xA000, i); // $8000-$9FFF
        for (word address = 0x0; address < 0x2000; address += 512) { // Switch Bank ($8000-$9FFF)
          dumpPRG(base, address, prgFile);
        }
      }
      for (word address = 0x2000; address < 0x8000; address += 512) { // Final 3 Banks ($A000-$FFFF)
        dumpPRG(base, address, prgFile);
      }
      break;

    case 10: // 128K/256K
      for (int i = 0; i < (((prgsize - 3) * 8) + 7); i++) {
        write_prg_byte(0xA000, i); // $8000-$BFFF
        for (word address = 0x0; address < 0x4000; address += 512) { // Switch Bank ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      for (word address = 0x4000; address < 0x8000; address += 512) { // Final Bank ($C000-$FFFF)
        dumpPRG(base, address, prgFile);
      }
      break;

    case 16:
    case 159: // 128K/256K
      banks = int_pow(2, prgsize);
      for (int i = 0; i < banks; i++) {
        write_prg_byte(0x6008, i); // Submapper 4
        write_prg_byte(0x8008, i); // Submapper 5
        for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 18: // 128K/256K
      banks = int_pow(2, prgsize) * 2;
      for (int i = 0; i < banks; i += 2) {
        write_prg_byte(0x8000, i & 0xF);
        write_prg_byte(0x8001, (i >> 4) & 0xF);
        write_prg_byte(0x8002, (i + 1) & 0xF);
        write_prg_byte(0x8003, ((i + 1) >> 4) & 0xF);
        for (word address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 19: // 128K/256K
      for (int j = 0; j < 64; j++) { // Init Register
        write_ram_byte(0xE000, 0); // PRG Bank 0 ($8000-$9FFF)
      }
      banks = int_pow(2, prgsize) * 2;
      for (int i = 0; i < banks; i++) {
        write_ram_byte(0xE000, i); // PRG Bank 0 ($8000-$9FFF)
        for (word address = 0x0; address < 0x2000; address += 512) {
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 21:
    case 22:
    case 23:
    case 25:
    case 65:
    case 75: // 128K/256K
      banks = int_pow(2, prgsize) * 2;
      for (int i = 0; i < banks; i += 2) {
        write_prg_byte(0x8000, i);
        write_prg_byte(0xA000, i + 1);
        for (word address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 24:
    case 26: // 256K
    case 78: // 128K
      banks = int_pow(2, prgsize);
      for (int i = 0; i < banks; i++) { // 128K
        write_prg_byte(0x8000, i);
        for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 30: // 256K/512K
      banks = int_pow(2, prgsize);
      for (int i = 0; i < banks; i++) { // 256K/512K
        if (flashfound)
          write_prg_byte(0xC000, i); // Flashable
        else
          write_prg_byte(0x8000, i); // Non-Flashable
        for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 32: // 128K/256K
      banks = int_pow(2, prgsize) * 2;
      for (int i = 0; i < banks; i++) { // 128K/256K
        write_prg_byte(0x9000, 1); // PRG Mode 0 - Read $A000-$BFFF to avoid difference between Modes 0 and 1
        write_prg_byte(0xA000, i); // PRG Bank
        for (word address = 0x2000; address < 0x4000; address += 512) { // 8K Banks ($A000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 33:
    case 48: // 128K/256K
      banks = int_pow(2, prgsize) * 2;
      for (int i = 0; i < banks; i += 2) {
        write_prg_byte(0x8000, i); // PRG Bank 0 ($8000-$9FFF)
        write_prg_byte(0x8001, i + 1); // PRG Bank 1 ($A000-$BFFF)
        for (word address = 0x0; address < 0x4000; address += 512) { // 8K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 37:
      banks = ((int_pow(2, prgsize) * 2)) - 2;  // Set Number of Banks
      write_prg_byte(0xA001, 0x80); // Block Register - PRG RAM Chip Enable, Writable
      for (int i = 0; i < banks; i += 2) { // 256K
        if (i == 0)
          write_prg_byte(0x6000, 0); // Switch to Lower Block ($0000-$FFFF)
        else if (i == 8)
          write_prg_byte(0x6000, 3); // Switch to 2nd 64K Block ($10000-$1FFFF)
        else if (i == 16)
          write_prg_byte(0x6000, 4); // Switch to 128K Block ($20000-$3FFFF)
        write_prg_byte(0x8000, 6); // PRG Bank 0 ($8000-$9FFF)
        write_prg_byte(0x8001, i);
        write_prg_byte(0x8000, 7); // PRG Bank 1 ($A000-$BFFF)
        write_prg_byte(0x8001, i + 1);
        for (word address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(base, address, prgFile);
        }
      }
      for (word address = 0x4000; address < 0x8000; address += 512) { // Final 2 Banks ($C000-$FFFF)
        dumpPRG(base, address, prgFile);
      }
      break;

    case 66: // 64K/128K
      banks = int_pow(2, prgsize) / 2;
      for (int i = 0; i < banks; i++) { // 64K/128K
        write_prg_byte(0x8000, i << 4); // bits 4-5
        for (word address = 0x0; address < 0x8000; address += 512) { // 32K Banks ($8000-$FFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 67: // 128K
      banks = int_pow(2, prgsize);
      for (int i = 0; i < banks; i++) { // 128K
        write_reg_byte(0xF800, i); // [WRITE RAM SAFE]
        for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 68:
    case 73: // 128K
      banks = int_pow(2, prgsize);
      for (int i = 0; i < banks; i++) { // 128K
        write_reg_byte(0xF000, i); // [WRITE RAM SAFE]
        for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 69: // 128K/256K
      banks = int_pow(2, prgsize) * 2;
      write_prg_byte(0x8000, 8); // Command Register - PRG Bank 0
      write_prg_byte(0xA000, 0); // Parameter Register - PRG RAM Disabled, PRG ROM, Bank 0 to $6000-$7FFF
      for (int i = 0; i < banks; i++) { // 128K/256K
        write_prg_byte(0x8000, 9); // Command Register - PRG Bank 1
        write_prg_byte(0xA000, i); // Parameter Register - ($8000-$9FFF)
        for (word address = 0x0000; address < 0x2000; address += 512) { // 8K Banks ($8000-$9FFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 70:
    case 89:
    case 93: // 128K
    case 152: // 64K/128K
      banks = int_pow(2, prgsize);
      for (int i = 0; i < banks; i++) { // 128K
        write_prg_byte(0x8000, i << 4);
        for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 71: // 64K/128K/256K
      banks = int_pow(2, prgsize);
      for (int i = 0; i < banks; i++) {
        write_prg_byte(0xC000, i);
        for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 72: // 128K
      banks = int_pow(2, prgsize);
      write_prg_byte(0x8000, 0); // Reset Register
      for (int i = 0; i < banks; i++) { // 128K
        write_prg_byte(0x8000, i | 0x80); // PRG Command + Bank
        write_prg_byte(0x8000, i); // PRG Bank
        for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 76:
    case 88:
    case 95:
    case 154: // 128K
    case 206: // 32K/64K/128K
      banks = int_pow(2, prgsize) * 2;
      for (int i = 0; i < banks; i += 2) {
        write_prg_byte(0x8000, 6); // PRG ROM Command ($8000-$9FFF)
        write_prg_byte(0x8001, i); // PRG Bank
        write_prg_byte(0x8000, 7); // PRG ROM Command ($A000-$BFFF)
        write_prg_byte(0x8001, i + 1); // PRG Bank
        for (word address = 0x0; address < 0x4000; address += 512) { // 8K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 80: // 128K
    case 207: // 256K [CART SOMETIMES NEEDS POWERCYCLE]
      banks = int_pow(2, prgsize) * 2;
      for (int i = 0; i < banks; i += 2) {
        write_prg_byte(0x7EFA, i); // PRG Bank 0 ($8000-$9FFF)
        write_prg_byte(0x7EFC, i + 1); // PRG Bank 1 ($A000-$BFFF)
        for (word address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 82: // 128K
      banks = int_pow(2, prgsize) * 2;
      for (int i = 0; i < banks; i += 2) {
        write_prg_byte(0x7EFA, i << 2); // PRG Bank 0 ($8000-$9FFF)
        write_prg_byte(0x7EFB, (i + 1) << 2); // PRG Bank 1 ($A000-$BFFF)
        for (word address = 0x0; address < 0x4000; address += 512) { // 8K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 85: // 128K/512K
      banks = int_pow(2, prgsize) * 2;
      for (int i = 0; i < banks; i++) {
        write_prg_byte(0x8000, i); // PRG Bank 0 ($8000-$9FFF)
        for (word address = 0x0; address < 0x2000; address += 512) { // 8K Banks ($8000-$9FFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 86:
    case 140: // 128K
      banks = int_pow(2, prgsize) / 2;
      for (int i = 0; i < banks; i++) { // 128K
        write_prg_byte(0x6000, i << 4); // bits 4-5
        for (word address = 0x0; address < 0x8000; address += 512) { // 32K Banks ($8000-$FFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 92: // 256K
      banks = int_pow(2, prgsize);
      write_prg_byte(0x8000, 0); // Reset Register
      for (int i = 0; i < banks; i++) { // 256K
        write_prg_byte(0x8000, i | 0x80); // PRG Command + Bank
        write_prg_byte(0x8000, i); // PRG Bank
        for (word address = 0x4000; address < 0x8000; address += 512) { // 16K Banks ($C000-$FFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 94:
      banks = int_pow(2, prgsize);
      for (int i = 0; i < banks; i++) { // 128K
        write_prg_byte(0x8000, i << 2);
        for (word address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 97: // 256K
      banks = int_pow(2, prgsize);
      for (int i = 0; i < banks; i++) { // 256K
        write_prg_byte(0x8000, i); // PRG Bank
        for (word address = 0x4000; address < 0x8000; address += 512) { // 16K Banks ($C000-$FFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 105: // 256K
      write_mmc1_byte(0xA000, 0x00); // Clear PRG Init/IRQ (Bit 4)
      write_mmc1_byte(0xA000, 0x10); // Set PRG Init/IRQ (Bit 4) to enable bank swapping
      for (int i = 0; i < 4; i++) { // PRG CHIP 1 128K
        write_mmc1_byte(0xA000, i << 1);
        for (word address = 0x0; address < 0x8000; address += 512) { // 32K Banks ($8000-$FFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      write_mmc1_byte(0x8000, 0x0C); // Switch 16K Bank ($8000-$BFFF) + Fixed Last Bank ($C000-$FFFF)
      write_mmc1_byte(0xA000, 0x08); // Select PRG CHIP 2 (Bit 3)
      for (int j = 0; j < 8; j++) { // PRG CHIP 2 128K
        write_mmc1_byte(0xE000, j);
        for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 180: // 128K
      banks = int_pow(2, prgsize);
      for (int i = 0; i < banks; i++) {
        write_prg_byte(0x8000, i);
        for (word address = 0x4000; address < 0x8000; address += 512) { // 16K Banks ($C000-$FFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 153: // 512K
      banks = int_pow(2, prgsize);
      for (int i = 0; i < banks; i++) { // 512K
        write_prg_byte(0x8000, i >> 4); // PRG Outer Bank (Documentation says duplicate over $8000-$8003 registers)
        write_prg_byte(0x8001, i >> 4); // PRG Outer Bank
        write_prg_byte(0x8002, i >> 4); // PRG Outer Bank
        write_prg_byte(0x8003, i >> 4); // PRG Outer Bank
        write_prg_byte(0x8008, i & 0xF); // PRG Inner Bank
        for (word address = 0x0000; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(base, address, prgFile);
        }
      }
      break;

    case 210: // 128K/256K
      banks = int_pow(2, prgsize) * 2;
      for (int i = 0; i < banks; i += 2) {
        write_prg_byte(0xE000, i); // PRG Bank 0 ($8000-$9FFF) [WRITE NO RAM]
        write_prg_byte(0xE800, i + 1); // PRG Bank 1 ($A000-$BFFF) [WRITE NO RAM]
        for (word address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(base, address, prgFile);
        }
      }
      break;
  }
  prgFile.close();

  ui.printlnMsg(F("PRG FILE DUMPED!"));
  ui.printlnMsg(F(""));
  ui.flushOutput();

  calcCRC(prgFilePath, prg * 1024);

  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
  LED_BLUE_OFF;
}

void readCHR(const String &outputFolderPath) {
  ui.clearOutput();
  ui.flushOutput();

  LED_GREEN_ON;
  set_address(0);
  _delay_us(1);
  if (chrsize == 0) {
    ui.printlnMsg(F("CHR SIZE 0K"));
    ui.flushOutput();
  }
  else {
    String chrFilePath = pathJoin(outputFolderPath, F("CHR.bin"));
    SafeSDFile chrFile = SafeSDFile::openForCreating(chrFilePath);

    switch (mapper) {
      case 0: // 8K
        for (word address = 0x0; address < 0x2000; address += 512) {
          dumpCHR(address, chrFile);
        }
        break;

      case 1:
      case 155:
        banks = int_pow(2, chrsize);
        for (int i = 0; i < banks; i += 2) { // 8K/16K/32K/64K/128K (Bank #s are based on 4K Banks)
          write_prg_byte(0x8000, 0x80); // Clear Register
          write_mmc1_byte(0xA000, i);
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 3: // 8K/16K/32K
      case 66: // 16K/32K
      case 70:
      case 152: // 128K
        banks = int_pow(2, chrsize) / 2;
        for (int i = 0; i < banks; i++) { // 8K Banks
          write_prg_byte(0x8000, i); // CHR Bank 0
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 4:
      case 47:
      case 118:
      case 119:
        banks = int_pow(2, chrsize) * 4;
        if (mapper == 47)
          write_prg_byte(0xA001, 0x80); // Block Register - PRG RAM Chip Enable, Writable
        for (int i = 0; i < banks; i += 4) { // 8K/16K/32K/64K/128K/256K
          if (mapper == 47) {
            if (i == 0)
              write_prg_byte(0x6000, 0); // Switch to Lower Block
            else if (i == 128)
              write_prg_byte(0x6000, 1); // Switch to Upper Block
          }
          write_prg_byte(0x8000, 0); // CHR Bank 0 ($0000-$07FF)
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 1); // CHR Bank 1 ($0800-$0FFF)
          write_prg_byte(0x8001, i + 2);
          for (word address = 0x0; address < 0x1000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 5: // 128K/256K/512K
        banks = int_pow(2, chrsize) / 2;
        write_prg_byte(0x5101, 0); // 8K CHR Banks
        for (int i = 0; i < banks; i++) {
          if (i == 0)
            write_prg_byte(0x5130, 0); // Set Upper 2 bits
          else if (i == 8)
            write_prg_byte(0x5130, 1); // Set Upper 2 bits
          else if (i == 16)
            write_prg_byte(0x5130, 2); // Set Upper 2 bits
          else if (i == 24)
            write_prg_byte(0x5130, 3); // Set Upper 2 bits
          write_prg_byte(0x5127, i);
          for (word address = 0x0; address < 0x2000; address += 512) { // ($0000-$1FFF)
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 9:
      case 10: // Mapper 9: 128K, Mapper 10: 64K/128K
        if (mapper == 9)
          banks = 32;
        else // Mapper 10
          banks = int_pow(2, chrsize);
        for (int i = 0; i < banks; i++) { // 64K/128K
          write_prg_byte(0xB000, i);
          write_prg_byte(0xC000, i);
          for (word address = 0x0; address < 0x1000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 16:
      case 159: // 128K/256K
        banks = int_pow(2, chrsize) * 4;
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0x6000, i); // Submapper 4
          write_prg_byte(0x8000, i); // Submapper 5
          for (word address = 0x0; address < 0x400; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 18: // 128K/256K
        banks = int_pow(2, chrsize) * 4;
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0xA000, i & 0xF); // CHR Bank Lower 4 bits
          write_prg_byte(0xA001, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits
          for (word address = 0x0; address < 0x400; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 19: // 128K/256K
        for (int j = 0; j < 64; j++) { // Init Register
          write_ram_byte(0xE800, 0xC0); // CHR RAM High/Low Disable (ROM Enable)
        }
        banks = int_pow(2, chrsize) * 4;
        write_ram_byte(0xE800, 0xC0); // CHR RAM High/Low Disable (ROM Enable)
        for (int i = 0; i < banks; i += 8) {
          write_prg_byte(0x8000, i); // CHR Bank 0
          write_prg_byte(0x8800, i + 1); // CHR Bank 1
          write_prg_byte(0x9000, i + 2); // CHR Bank 2
          write_prg_byte(0x9800, i + 3); // CHR Bank 3
          write_prg_byte(0xA000, i + 4); // CHR Bank 4
          write_prg_byte(0xA800, i + 5); // CHR Bank 5
          write_prg_byte(0xB000, i + 6); // CHR Bank 6
          write_prg_byte(0xB800, i + 7); // CHR Bank 7
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 21: // 128K/256K
        banks = int_pow(2, chrsize) * 4;
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0xB000, i & 0xF); // CHR Bank Lower 4 bits
          if (banks == 128)
            write_prg_byte(0xB002, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4a (Wai Wai World 2)
          else  // banks == 256
            write_prg_byte(0xB040, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4c (Ganbare Goemon Gaiden 2)
          for (word address = 0x0; address < 0x400; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 22: // 128K
        banks = int_pow(2, chrsize) * 4;
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0xB000, (i << 1) & 0xF); // CHR Bank Lower 4 bits
          write_prg_byte(0xB002, (i >> 3) & 0xF);  // CHR Bank Upper 4 bits
          for (word address = 0x0; address < 0x400; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 23: // 128K
        // Detect VRC4e Carts - read PRG 0x1FFF6 (DATE)
        // Boku Dracula-kun = 890810, Tiny Toon = 910809
        // Crisis Force = 910701, Parodius Da! = 900916
        write_prg_byte(0x8000, 15);
        prgchk0 = read_prg_byte(0x9FF6);
        if (prgchk0 == 0x30) { // Check for "0" in middle of date
          vrc4e = true; // VRC4e Cart
        }
        banks = int_pow(2, chrsize) * 4;
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0xB000, i & 0xF); // CHR Bank Lower 4 bits
          if (vrc4e == true)
            write_prg_byte(0xB004, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4e
          else
            write_prg_byte(0xB001, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC2b/VRC4f
          for (word address = 0x0; address < 0x400; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 24: // 128K
        banks = int_pow(2, chrsize) * 4;
        write_prg_byte(0xB003, 0); // PPU Banking Mode 0
        for (int i = 0; i < banks; i += 8) {
          write_prg_byte(0xD000, i); // CHR Bank 0
          write_prg_byte(0xD001, i + 1); // CHR Bank 1
          write_prg_byte(0xD002, i + 2); // CHR Bank 2
          write_prg_byte(0xD003, i + 3); // CHR Bank 3
          write_prg_byte(0xE000, i + 4); // CHR Bank 4 [WRITE NO RAM]
          write_prg_byte(0xE001, i + 5); // CHR Bank 5 [WRITE NO RAM]
          write_prg_byte(0xE002, i + 6); // CHR Bank 6 [WRITE NO RAM]
          write_prg_byte(0xE003, i + 7); // CHR Bank 7 [WRITE NO RAM]
          for (word address = 0x0; address < 0x2000; address += 512) { // 1K Banks
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 25: // 128K/256K
        banks = int_pow(2, chrsize) * 4;
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0xB000, i & 0xF); // CHR Bank Lower 4 bits
          if ((ramsize > 0) || (banks == 128)) // VRC2c (Ganbare Goemon Gaiden)/VRC4b (Bio Miracle/Gradius 2/Racer Mini)
            write_prg_byte(0xB002, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC2c/VRC4b
          else
            write_prg_byte(0xB008, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4d (Teenage Mutant Ninja Turtles)
          for (word address = 0x0; address < 0x400; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 26: // 128K/256K
        banks = int_pow(2, chrsize) * 4;
        write_prg_byte(0xB003, 0); // PPU Banking Mode 0
        for (int i = 0; i < banks; i += 8) {
          write_prg_byte(0xD000, i); // CHR Bank 0
          write_prg_byte(0xD002, i + 1); // CHR Bank 1
          write_prg_byte(0xD001, i + 2); // CHR Bank 2
          write_prg_byte(0xD003, i + 3); // CHR Bank 3
          write_reg_byte(0xE000, i + 4); // CHR Bank 4 [WRITE RAM SAFE]
          write_reg_byte(0xE002, i + 5); // CHR Bank 5 [WRITE RAM SAFE]
          write_reg_byte(0xE001, i + 6); // CHR Bank 6 [WRITE RAM SAFE]
          write_reg_byte(0xE003, i + 7); // CHR Bank 7 [WRITE RAM SAFE]
          for (word address = 0x0; address < 0x2000; address += 512) { // 1K Banks
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 32: // 128K
      case 65: // 128K/256K
        banks = int_pow(2, chrsize) * 4;
        for (int i = 0; i < banks; i += 8) {
          write_prg_byte(0xB000, i); // CHR Bank 0
          write_prg_byte(0xB001, i + 1); // CHR Bank 1
          write_prg_byte(0xB002, i + 2); // CHR Bank 2
          write_prg_byte(0xB003, i + 3); // CHR Bank 3
          write_prg_byte(0xB004, i + 4); // CHR Bank 4
          write_prg_byte(0xB005, i + 5); // CHR Bank 5
          write_prg_byte(0xB006, i + 6); // CHR Bank 6
          write_prg_byte(0xB007, i + 7); // CHR Bank 7
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 33: // 128K/256K
      case 48: // 256K
        banks = int_pow(2, chrsize) * 2;
        for (int i = 0; i < banks; i += 2) { // 2K Banks
          write_prg_byte(0x8002, i); // CHR Bank 0
          write_prg_byte(0x8003, i + 1); // CHR Bank 1
          for (word address = 0x0; address < 0x1000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 37:
        banks = int_pow(2, chrsize) * 4;
        write_prg_byte(0xA001, 0x80); // Block Register - PRG RAM Chip Enable, Writable
        for (int i = 0; i < banks; i += 4) { // 256K
          if (i == 0)
            write_prg_byte(0x6000, 0); // Switch to Lower Block ($00000-$1FFFF)
          else if (i == 128)
            write_prg_byte(0x6000, 4); // Switch to Upper Block ($20000-$3FFFF)
          write_prg_byte(0x8000, 0); // CHR Bank 0 ($0000-$07FF)
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 1); // CHR Bank 1 ($0800-$0FFF)
          write_prg_byte(0x8001, i + 2);
          for (word address = 0x0; address < 0x1000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 67: // 128K
        banks = int_pow(2, chrsize) * 2;
        for (int i = 0; i < banks; i += 4) { // 2K Banks
          write_prg_byte(0x8800, i); // CHR Bank 0
          write_prg_byte(0x9800, i + 1); // CHR Bank 1
          write_prg_byte(0xA800, i + 2); // CHR Bank 2
          write_prg_byte(0xB800, i + 3); // CHR Bank 3
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 68: // 128K/256K
        banks = int_pow(2, chrsize) * 2;
        for (int i = 0; i < banks; i += 4) { // 2K Banks
          write_prg_byte(0x8000, i); // CHR Bank 0
          write_prg_byte(0x9000, i + 1); // CHR Bank 1
          write_prg_byte(0xA000, i + 2); // CHR Bank 2
          write_prg_byte(0xB000, i + 3); // CHR Bank 3
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 69: // 128K/256K
        banks = int_pow(2, chrsize) * 4;
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0x8000, 0); // Command Register - CHR Bank 0
          write_prg_byte(0xA000, i); // Parameter Register - ($0000-$03FF)
          for (word address = 0x0; address < 0x400; address += 512) { // 1K Banks
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 72: // 128K
        banks = int_pow(2, chrsize) / 2;
        write_prg_byte(0x8000, 0); // Reset Register
        for (int i = 0; i < banks; i++) { // 8K Banks
          write_prg_byte(0x8000, i | 0x40); // CHR Command + Bank
          write_prg_byte(0x8000, i); // CHR Bank
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 75: // 128K
        banks = int_pow(2, chrsize);
        for (int i = 0; i < banks; i++) { // 4K Banks
          write_reg_byte(0xE000, i); // CHR Bank Low Bits [WRITE RAM SAFE]
          write_prg_byte(0x9000, (i & 0x10) >> 3); // High Bit
          for (word address = 0x0; address < 0x1000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 76: // 128K
        banks = int_pow(2, chrsize) * 2;
        for (int i = 0; i < banks; i += 2) { // 2K Banks
          write_prg_byte(0x8000, 2); // CHR Command ($0000-$07FF) 2K Bank
          write_prg_byte(0x8001, i); // CHR Bank
          write_prg_byte(0x8000, 3); // CHR Command ($0800-$0FFF) 2K Bank
          write_prg_byte(0x8001, i + 1); // CHR Bank
          for (word address = 0x0000; address < 0x1000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 77: // 32K
        banks = int_pow(2, chrsize) * 2;
        for (int i = 0; i < banks; i++) { // 2K Banks
          write_prg_byte(0x8000, i << 4); // CHR Bank 0
          for (word address = 0x0; address < 0x800; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 78: // 128K
        banks = int_pow(2, chrsize) / 2;
        for (int i = 0; i < banks; i++) { // 8K Banks
          write_prg_byte(0x8000, i << 4); // CHR Bank 0
          for (word address = 0x0; address < 0x2000; address += 512) { // 8K Banks ($0000-$1FFF)
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 80: // 128K/256K
      case 82: // 128K/256K
      case 207: // 128K [CART SOMETIMES NEEDS POWERCYCLE]
        banks = int_pow(2, chrsize) * 4;
        for (int i = 0; i < banks; i += 4) {
          write_prg_byte(0x7EF2, i);  // CHR Bank 2 [REGISTERS 0x7EF0/0x7EF1 WON'T WORK]
          write_prg_byte(0x7EF3, i + 1);  // CHR Bank 3
          write_prg_byte(0x7EF4, i + 2);  // CHR Bank 4
          write_prg_byte(0x7EF5, i + 3);  // CHR Bank 5
          for (word address = 0x1000; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 85: // 128K
        banks = int_pow(2, chrsize) * 4;
        for (int i = 0; i < banks; i += 8) {
          write_prg_byte(0xA000, i); // CHR Bank 0
          write_prg_byte(0xA008, i + 1); // CHR Bank 1
          write_prg_byte(0xB000, i + 2); // CHR Bank 2
          write_prg_byte(0xB008, i + 3); // CHR Bank 3
          write_prg_byte(0xC000, i + 4); // CHR Bank 4
          write_prg_byte(0xC008, i + 5); // CHR Bank 5
          write_prg_byte(0xD000, i + 6); // CHR Bank 6
          write_prg_byte(0xD008, i + 7); // CHR Bank 7
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 86: // 64K
        banks = int_pow(2, chrsize) / 2;
        for (int i = 0; i < banks; i++) { // 8K Banks
          if (i < 4)
            write_prg_byte(0x6000, i & 0x3);
          else
            write_prg_byte(0x6000, (i | 0x40) & 0x43);
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 87: // 16K/32K
        banks = int_pow(2, chrsize) / 2;
        for (int i = 0; i < banks; i++) { // 16K/32K
          write_prg_byte(0x6000, (((i & 0x1) << 1) | ((i & 0x2) >> 1)));
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 88: // 128K
      case 95: // 32K
      case 154: // 128K
      case 206: // 16K/32K/64K
        banks = int_pow(2, chrsize) * 4;
        for (int i = 0; i < banks; i += 2) { // 1K Banks
          if (i < 64) {
            write_prg_byte(0x8000, 0); // CHR Command ($0000-$07FF) 2K Bank
            write_prg_byte(0x8001, i & 0x3F); // CHR Bank
            for (word address = 0x0; address < 0x800; address += 512) {
              dumpCHR(address, chrFile);
            }
          }
          else {
            write_prg_byte(0x8000, 2); // CHR Command ($1000-$13FF) 1K Bank
            write_prg_byte(0x8001, i); // CHR Bank
            write_prg_byte(0x8000, 3); // CHR Command ($1400-$17FF) 1K Bank
            write_prg_byte(0x8001, i + 1); // CHR Bank
            for (word address = 0x1000; address < 0x1800; address += 512) {
              dumpCHR(address, chrFile);
            }
          }
        }
        break;

      case 89: // 128K
        banks = int_pow(2, chrsize) / 2;
        for (int i = 0; i < banks; i++) { // 8K Banks
          if (i < 8)
            write_prg_byte(0x8000, i & 0x7);
          else
            write_prg_byte(0x8000, (i | 0x80) & 0x87);
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 92: // 128K
        banks = int_pow(2, chrsize) / 2;
        write_prg_byte(0x8000, 0); // Reset Register
        for (int i = 0; i < banks; i++) { // 8K Banks
          write_prg_byte(0x8000, i | 0x40); // CHR Command + Bank
          write_prg_byte(0x8000, i); // CHR Bank
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 140: // 32K/128K
        banks = int_pow(2, chrsize) / 2;
        for (int i = 0; i < banks; i++) { // 8K Banks
          write_prg_byte(0x6000, i);
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 184: // 16K/32K
        banks = int_pow(2, chrsize);
        for (int i = 0; i < banks; i++) { // 4K Banks
          write_prg_byte(0x6000, i); // CHR LOW (Bits 0-2) ($0000-$0FFF)
          for (word address = 0x0; address < 0x1000; address += 512) { // 4K Banks ($0000-$0FFF)
            dumpCHR(address, chrFile);
          }
        }
        break;

      case 185: // 8K [READ 32K TO OVERRIDE LOCKOUT]
        for (int i = 0; i < 4; i++) { // Read 32K to locate valid 8K
          write_prg_byte(0x8000, i);
          byte chrcheck = read_chr_byte(0);
          for (word address = 0x0; address < 0x2000; address += 512) {
            for (int x = 0; x < 512; x++) {
              sdBuffer[x] = read_chr_byte(address + x);
            }
            if (chrcheck != 0xFF)
              chrFile.write(sdBuffer, 512);
          }
        }
        break;

      case 210: // 128K/256K
        banks = int_pow(2, chrsize) * 4;
        write_prg_byte(0xE800, 0xC0); // CHR RAM DISABLE (Bit 6 and 7) [WRITE NO RAM]
        for (int i = 0; i < banks; i += 8) {
          write_prg_byte(0x8000, i); // CHR Bank 0
          write_prg_byte(0x8800, i + 1); // CHR Bank 1
          write_prg_byte(0x9000, i + 2); // CHR Bank 2
          write_prg_byte(0x9800, i + 3); // CHR Bank 3
          write_prg_byte(0xA000, i + 4); // CHR Bank 4
          write_prg_byte(0xA800, i + 5); // CHR Bank 5
          write_prg_byte(0xB000, i + 6); // CHR Bank 6
          write_prg_byte(0xB800, i + 7); // CHR Bank 7
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address, chrFile);
          }
        }
        break;
    }
    chrFile.close();

    ui.printlnMsg(F("CHR FILE DUMPED!"));
    ui.printlnMsg(F(""));
    ui.flushOutput();

    calcCRC(chrFilePath, chr * 1024);
  }
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
  LED_GREEN_OFF;
}

/******************************************
   [RAM Functions]
 *****************************************/
void readRAM(const String &outputFolderPath) {
  ui.clearOutput();
  ui.flushOutput();

  LED_BLUE_ON;
  LED_GREEN_ON;
  set_address(0);
  _delay_us(1);
  if (ramsize == 0) {
    ui.printlnMsg(F("RAM SIZE 0K"));
    ui.flushOutput();
  }
  else {
    String ramFilePath = pathJoin(outputFolderPath, F("RAM.bin"));
    SafeSDFile ramFile = SafeSDFile::openForCreating(ramFilePath);
    word base = 0x6000;
    switch (mapper) {
      case 0: // 2K/4K
        for (word address = 0x0; address < (0x800 * ramsize); address += 512) { // 2K/4K
          dumpPRG(base, address, ramFile); // SWITCH MUST BE IN OFF POSITION
        }
        break;

      case 1:
      case 155: // 8K/16K/32K
        banks = int_pow(2, ramsize) / 2; // banks = 1,2,4
        for (int i = 0; i < banks; i++) { // 8K Banks ($6000-$7FFF)
          write_prg_byte(0x8000, 0x80); // Clear Register
          write_mmc1_byte(0x8000, 1 << 3);
          write_mmc1_byte(0xE000, 0);
          if (banks == 4) // 32K
            write_mmc1_byte(0xA000, i << 2);
          else
            write_mmc1_byte(0xA000, i << 3);
          for (word address = 0x0; address < 0x2000; address += 512) { // 8K
            dumpPRG(base, address, ramFile);
          }
        }
        break;

      case 4: // 1K/8K (MMC6/MMC3)
        if (mmc6) { // MMC6 1K
          write_prg_byte(0x8000, 0x20); // PRG RAM ENABLE
          write_prg_byte(0xA001, 0x20); // PRG RAM PROTECT - Enable reading RAM at $7000-$71FF
          for (word address = 0x1000; address < 0x1200; address += 512) { // 512B
            dumpMMC5RAM(base, address, ramFile);
          }
          write_prg_byte(0x8000, 0x20); // PRG RAM ENABLE
          write_prg_byte(0xA001, 0x80); // PRG RAM PROTECT - Enable reading RAM at $7200-$73FF
          for (word address = 0x1200; address < 0x1400; address += 512) { // 512B
            dumpMMC5RAM(base, address, ramFile);
          }
          write_prg_byte(0x8000, 6); // PRG RAM DISABLE
        }
        else { // MMC3 8K
          write_prg_byte(0xA001, 0xC0); // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
          for (word address = 0; address < 0x2000; address += 512) { // 8K
            dumpPRG(base, address, ramFile);
          }
        }
        break;

      case 5: // 8K/16K/32K
        write_prg_byte(0x5100, 3); // 8K PRG Banks
        banks = int_pow(2, ramsize) / 2; // banks = 1,2,4
        if (banks == 2) { // 16K - Split SRAM Chips 8K/8K
          for (int i = 0; i < (banks / 2); i++) { // Chip 1
            write_prg_byte(0x5113, i);
            for (word address = 0; address < 0x2000; address += 512) { // 8K
              dumpMMC5RAM(base, address, ramFile);
            }
          }
          for (int j = 4; j < (banks / 2) + 4; j++) { // Chip 2
            write_prg_byte(0x5113, j);
            for (word address = 0; address < 0x2000; address += 512) { // 8K
              dumpMMC5RAM(base, address, ramFile);
            }
          }
        }
        else { // 8K/32K Single SRAM Chip
          for (int i = 0; i < banks; i++) { // banks = 1 or 4
            write_prg_byte(0x5113, i);
            for (word address = 0; address < 0x2000; address += 512) { // 8K
              dumpMMC5RAM(base, address, ramFile);
            }
          }
        }
        break;

      case 16: // 256-byte EEPROM 24C02
      case 159: // 128-byte EEPROM 24C01 [Little Endian]
        if (mapper == 159)
          eepsize = 128;
        else
          eepsize = 256;
        for (word address = 0; address < eepsize; address++) {
          EepromREAD(address);
        }
        ramFile.write(sdBuffer, eepsize);
        //          ui.clearOutput(); // TEST PURPOSES - DISPLAY EEPROM DATA
        break;

      case 19:
        if (ramsize == 2) { // PRG RAM 128B
          for (int x = 0; x < 128; x++) {
            write_ram_byte(0xF800, x); // PRG RAM ENABLE
            sdBuffer[x] = read_prg_byte(0x4800); // DATA PORT
          }
          ramFile.write(sdBuffer, 128);
        }
        else { // SRAM 8K
          for (int i = 0; i < 64; i++) { // Init Register
            write_ram_byte(0xE000, 0);
          }
          for (word address = 0; address < 0x2000; address += 512) { // 8K
            dumpPRG(base, address, ramFile);
          }
        }
        break;

      case 80: // 1K
        write_prg_byte(0x7EF8, 0xA3); // PRG RAM ENABLE 0
        write_prg_byte(0x7EF9, 0xA3); // PRG RAM ENABLE 1
        for (int x = 0; x < 128; x++) {  // PRG RAM 1K ($7F00-$7FFF) MIRRORED ONCE
          sdBuffer[x] = read_prg_byte(0x7F00 + x);
        }
        ramFile.write(sdBuffer, 128);
        write_prg_byte(0x7EF8, 0xFF); // PRG RAM DISABLE 0
        write_prg_byte(0x7EF9, 0xFF); // PRG RAM DISABLE 1
        break;

      case 82: // 5K
        write_prg_byte(0x7EF7, 0xCA); // PRG RAM ENABLE 0 ($6000-$67FF)
        write_prg_byte(0x7EF8, 0x69); // PRG RAM ENABLE 1 ($6800-$6FFF)
        write_prg_byte(0x7EF9, 0x84); // PRG RAM ENABLE 2 ($7000-$73FF)
        for (word address = 0x0; address < 0x1400; address += 512) { // PRG RAM 5K ($6000-$73FF)
          dumpMMC5RAM(base, address, ramFile);
        }
        write_prg_byte(0x7EF7, 0xFF); // PRG RAM DISABLE 0 ($6000-$67FF)
        write_prg_byte(0x7EF8, 0xFF); // PRG RAM DISABLE 1 ($6800-$6FFF)
        write_prg_byte(0x7EF9, 0xFF); // PRG RAM DISABLE 2 ($7000-$73FF)
        break;

      default:
        if (mapper == 118) // 8K
          write_prg_byte(0xA001, 0xC0); // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
        else if (mapper == 19) {
          for (int i = 0; i < 64; i++) { // Init Register
            write_ram_byte(0xE000, 0);
          }
        }
        else if ((mapper == 21) || (mapper == 25)) // 8K
          write_prg_byte(0x8000, 0);
        else if (mapper == 26) // 8K
          write_prg_byte(0xB003, 0x80); // PRG RAM ENABLE
        else if (mapper == 68) // 8K
          write_reg_byte(0xF000, 0x10); // PRG RAM ENABLE [WRITE RAM SAFE]
        else if (mapper == 69) { // 8K
          write_prg_byte(0x8000, 8); // Command Register - PRG Bank 0
          write_prg_byte(0xA000, 0xC0); // Parameter Register - PRG RAM Enabled, PRG RAM, Bank 0 to $6000-$7FFF
        }
        else if (mapper == 85) // 8K
          write_ram_byte(0xE000, 0x80); // PRG RAM ENABLE
        else if (mapper == 153) // 8K
          write_prg_byte(0x800D, 0x20); // PRG RAM Chip Enable
        for (word address = 0; address < 0x2000; address += 512) { // 8K
          dumpPRG(base, address, ramFile);
        }
        if (mapper == 85) // 8K
          write_reg_byte(0xE000, 0); // PRG RAM DISABLE [WRITE RAM SAFE]
        break;
    }
    ramFile.close();

    ui.printlnMsg(F("RAM FILE DUMPED!"));
    ui.printlnMsg(F(""));
    ui.flushOutput();

    if ((mapper == 16) || (mapper == 159))
      calcCRC(ramFilePath, eepsize);
    else
      calcCRC(ramFilePath, ram * 1024);
  }
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
  LED_BLUE_OFF;
  LED_GREEN_OFF;
}

void writeRAM() {
  ui.clearOutput();

  if (ramsize == 0) {
    ui.printError(F("RAM SIZE 0K"));
  }
  else {
    String ramFilePath = fileBrowser(F("Select RAM File"));
    word base = 0x6000;

    ui.clearOutput();
    ui.printlnMsg(F("Writing File: "));
    ui.printlnMsg(ramFilePath);
    ui.flushOutput();

    //open file on sd card
    SafeSDFile ramFile = SafeSDFile::openForReading(ramFilePath);
    switch (mapper) {
      case 0: // 2K/4K
        for (word address = 0x0; address < (0x800 * ramsize); address += 512) { // 2K/4K
          ramFile.read(sdBuffer, 512);
          for (int x = 0; x < 512; x++) {
            write_prg_byte(base + address + x, sdBuffer[x]); // SWITCH MUST BE IN OFF POSITION
          }
        }
        break;

      case 1:
      case 155:
        banks = int_pow(2, ramsize) / 2; // banks = 1,2,4
        for (int i = 0; i < banks; i++) { // 8K Banks ($6000-$7FFF)
          write_prg_byte(0x8000, 0x80); // Clear Register
          write_mmc1_byte(0x8000, 1 << 3); // PRG ROM MODE 32K
          write_mmc1_byte(0xE000, 0); // PRG RAM ENABLED
          if (banks == 4) // 32K
            write_mmc1_byte(0xA000, i << 2);
          else
            write_mmc1_byte(0xA000, i << 3);
          for (word address = 0x0; address < 0x2000; address += 512) { // 8K
            ramFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              write_prg_byte(base + address + x, sdBuffer[x]);
            }
          }
        }
        break;

      case 4: // 1K/8K (MMC6/MMC3)
        if (mmc6) { // MMC6 1K
          write_prg_byte(0x8000, 0x20); // PRG RAM ENABLE
          write_prg_byte(0xA001, 0x30); // PRG RAM PROTECT - Enable reading/writing to RAM at $7000-$71FF
          for (word address = 0x1000; address < 0x1200; address += 512) { // 512B
            ramFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              write_wram_byte(base + address + x, sdBuffer[x]);
            }
          }
          write_prg_byte(0x8000, 0x20); // PRG RAM ENABLE
          write_prg_byte(0xA001, 0xC0); // PRG RAM PROTECT - Enable reading/writing to RAM at $7200-$73FF
          for (word address = 0x1200; address < 0x1400; address += 512) { // 512B
            ramFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              write_wram_byte(base + address + x, sdBuffer[x]);
            }
          }
          write_prg_byte(0x8000, 0x6); // PRG RAM DISABLE
        }
        else { // MMC3 8K
          write_prg_byte(0xA001, 0x80); // PRG RAM CHIP ENABLE - Chip Enable, Allow Writes
          for (word address = 0; address < 0x2000; address += 512) { // 8K
            ramFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              write_prg_byte(base + address + x, sdBuffer[x]);
            }
          }
          write_prg_byte(0xA001, 0xC0); // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
        }
        break;

      case 5: // 8K/16K/32K
        write_prg_byte(0x5100, 3); // 8K PRG Banks
        banks = int_pow(2, ramsize) / 2; // banks = 1,2,4
        if (banks == 2) { // 16K - Split SRAM Chips 8K/8K [ETROM = 16K (ONLY 1ST 8K BATTERY BACKED)]
          for (int i = 0; i < (banks / 2); i++) { // Chip 1
            write_prg_byte(0x5113, i);
            for (word address = 0; address < 0x2000; address += 512) { // 8K
              writeMMC5RAM(base, address, ramFile);
            }
          }
          for (int j = 4; j < (banks / 2) + 4; j++) { // Chip 2
            write_prg_byte(0x5113, j);
            for (word address = 0; address < 0x2000; address += 512) { // 8K
              writeMMC5RAM(base, address, ramFile);
            }
          }
        }
        else { // 8K/32K Single SRAM Chip [EKROM = 8K BATTERY BACKED, EWROM = 32K BATTERY BACKED]
          for (int i = 0; i < banks; i++) { // banks = 1 or 4
            write_prg_byte(0x5113, i);
            for (word address = 0; address < 0x2000; address += 512) { // 8K
              writeMMC5RAM(base, address, ramFile);
            }
          }
        }
        break;

      case 16: // 256-byte EEPROM 24C02
      case 159: // 128-byte EEPROM 24C01 [Little Endian]
        if (mapper == 159)
          eepsize = 128;
        else
          eepsize = 256;
        ramFile.read(sdBuffer, eepsize);
        for (word address = 0; address < eepsize; address++) {
          EepromWRITE(address);
          if ((address % 128) == 0)
            ui.clearOutput();
          ui.printMsg(F("."));
          ui.flushOutput();
        }
        break;

      case 19:
        if (ramsize == 2) { // PRG RAM 128B
          ramFile.read(sdBuffer, 128);
          for (int x = 0; x < 128; x++) {
            write_ram_byte(0xF800, x); // PRG RAM ENABLE
            write_prg_byte(0x4800, sdBuffer[x]); // DATA PORT
          }
        }
        else { // SRAM 8K
          for (int i = 0; i < 64; i++) { // Init Register
            write_ram_byte(0xF800, 0x40); // PRG RAM WRITE ENABLE
          }
          write_ram_byte(0xF800, 0x40); // PRG RAM WRITE ENABLE
          for (word address = 0; address < 0x2000; address += 512) { // 8K
            ramFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              write_prg_byte(base + address + x, sdBuffer[x]);
            }
          }
          write_ram_byte(0xF800, 0x0F); // PRG RAM WRITE PROTECT
        }
        break;

      case 80: // 1K
        write_prg_byte(0x7EF8, 0xA3); // PRG RAM ENABLE 0
        write_prg_byte(0x7EF9, 0xA3); // PRG RAM ENABLE 1
        for (word address = 0x1F00; address < 0x2000; address += 512) { // PRG RAM 1K ($7F00-$7FFF)
          ramFile.read(sdBuffer, 128);
          for (int x = 0; x < 128; x++) {
            write_prg_byte(base + address + x, sdBuffer[x]);
          }
        }
        write_prg_byte(0x7EF8, 0xFF); // PRG RAM DISABLE 0
        write_prg_byte(0x7EF9, 0xFF); // PRG RAM DISABLE 1
        break;

      case 82: // 5K
        write_prg_byte(0x7EF7, 0xCA); // PRG RAM ENABLE 0 ($6000-$67FF)
        write_prg_byte(0x7EF8, 0x69); // PRG RAM ENABLE 1 ($6800-$6FFF)
        write_prg_byte(0x7EF9, 0x84); // PRG RAM ENABLE 2 ($7000-$73FF)
        for (word address = 0x0; address < 0x1400; address += 1024) { // PRG RAM 5K ($6000-$73FF)
          ramFile.read(sdBuffer, 512);
          firstbyte = sdBuffer[0];
          for (int x = 0; x < 512; x++)
            write_prg_byte(base + address + x, sdBuffer[x]);
          ramFile.read(sdBuffer, 512);
          for (int x = 0; x < 512; x++)
            write_prg_byte(base + address + x + 512, sdBuffer[x]);
          write_prg_byte(base + address, firstbyte); // REWRITE 1ST BYTE
        }
        write_prg_byte(0x7EF7, 0xFF); // PRG RAM DISABLE 0 ($6000-$67FF)
        write_prg_byte(0x7EF8, 0xFF); // PRG RAM DISABLE 1 ($6800-$6FFF)
        write_prg_byte(0x7EF9, 0xFF); // PRG RAM DISABLE 2 ($7000-$73FF)
        break;

      default:
        if (mapper == 118) // 8K
          write_prg_byte(0xA001, 0x80); // PRG RAM CHIP ENABLE - Chip Enable, Allow Writes
        else if ((mapper == 21) || (mapper == 25)) // 8K
          write_prg_byte(0x8000, 0);
        else if (mapper == 26) // 8K
          write_prg_byte(0xB003, 0x80); // PRG RAM ENABLE
        //            else if (mapper == 68) // 8K
        //              write_reg_byte(0xF000, 0x10); // PRG RAM ENABLE [WRITE RAM SAFE]
        else if (mapper == 69) { // 8K
          write_prg_byte(0x8000, 8); // Command Register - PRG Bank 0
          write_prg_byte(0xA000, 0xC0); // Parameter Register - PRG RAM Enabled, PRG RAM, Bank 0 to $6000-$7FFF
        }
        else if (mapper == 85) // 8K
          write_ram_byte(0xE000, 0x80); // PRG RAM ENABLE
        else if (mapper == 153) // 8K
          write_prg_byte(0x800D, 0x20); // PRG RAM Chip Enable
        for (word address = 0; address < 0x2000; address += 512) { // 8K
          ramFile.read(sdBuffer, 512);
          for (int x = 0; x < 512; x++) {
            write_prg_byte(base + address + x, sdBuffer[x]);
          }
        }
        if (mapper == 118) // 8K
          write_prg_byte(0xA001, 0xC0); // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
        else if (mapper == 26) // 8K
          write_prg_byte(0xB003, 0); // PRG RAM DISABLE
        //            else if (mapper == 68) // 8K
        //              write_reg_byte(0xF000, 0x00); // PRG RAM DISABLE [WRITE RAM SAFE]
        else if (mapper == 69) { // 8K
          write_prg_byte(0x8000, 8); // Command Register - PRG Bank 0
          write_prg_byte(0xA000, 0); // Parameter Register - PRG RAM Disabled, PRG ROM, Bank 0 to $6000-$7FFF
        }
        else if (mapper == 85) // 8K
          write_reg_byte(0xE000, 0); // PRG RAM DISABLE [WRITE RAM SAFE]
        break;
    }
    ramFile.close();
    LED_GREEN_ON;

    ui.printlnMsg(F(""));
    ui.printlnMsg(F("RAM FILE WRITTEN!"));
    ui.flushOutput();
  }

  ui.clearOutput();

  LED_RED_OFF;
  LED_GREEN_OFF;
}

/******************************************
   [Eeprom Functions]
 *****************************************/
void EepromStart_NES() {
  write_prg_byte(0x800D, 0x00); // sda low, scl low
  write_prg_byte(0x800D, 0x60); // sda, scl high
  write_prg_byte(0x800D, 0x20); // sda low, scl high
  write_prg_byte(0x800D, 0x00); // START
}

void EepromStop_NES() {
  write_prg_byte(0x800D, 0x00); // sda, scl low
  write_prg_byte(0x800D, 0x20); // sda low, scl high
  write_prg_byte(0x800D, 0x60); // sda, scl high
  write_prg_byte(0x800D, 0x40); // sda high, scl low
  write_prg_byte(0x800D, 0x00); // STOP
}

void EepromSet0_NES() {
  write_prg_byte(0x800D, 0x00); // sda low, scl low
  write_prg_byte(0x800D, 0x20); // sda low, scl high // 0
  write_prg_byte(0x800D, 0x00); // sda low, scl low
}

void EepromSet1_NES() {
  write_prg_byte(0x800D, 0x40); // sda high, scl low
  write_prg_byte(0x800D, 0x60); // sda high, scl high // 1
  write_prg_byte(0x800D, 0x40); // sda high, scl low
  write_prg_byte(0x800D, 0x00); // sda low, scl low
}

void EepromStatus_NES() { // ACK
  write_prg_byte(0x800D, 0x40); // sda high, scl low
  write_prg_byte(0x800D, 0x60); // sda high, scl high
  write_prg_byte(0x800D, 0xE0); // sda high, scl high, read high
  byte eepStatus = 1;
  do {
    eepStatus = (read_prg_byte(0x6000) & 0x10) >> 4;
    delayMicroseconds(4);
  }
  while (eepStatus == 1);
  write_prg_byte(0x800D, 0x40); // sda high, scl low
}

void EepromReadData_NES() {
  // read serial data into buffer
  for (int i = 0; i < 8; i++) {
    write_prg_byte(0x800D, 0x60); // sda high, scl high, read low
    write_prg_byte(0x800D, 0xE0); // sda high, scl high, read high
    eepbit[i] = (read_prg_byte(0x6000) & 0x10) >> 4; // Read 0x6000 with Mask 0x10 (bit 4)
    write_prg_byte(0x800D, 0x40); // sda high, scl low
  }
}

void EepromDevice_NES() { // 24C02 ONLY
  EepromSet1_NES();
  EepromSet0_NES();
  EepromSet1_NES();
  EepromSet0_NES();
  EepromSet0_NES(); // A2
  EepromSet0_NES(); // A1
  EepromSet0_NES(); // A0
}

void EepromReadMode_NES() {
  EepromSet1_NES(); // READ
  EepromStatus_NES(); // ACK
}
void EepromWriteMode_NES() {
  EepromSet0_NES(); // WRITE
  EepromStatus_NES(); // ACK
}

void EepromFinish_NES() {
  write_prg_byte(0x800D, 0x00); // sda low, scl low
  write_prg_byte(0x800D, 0x40); // sda high, scl low
  write_prg_byte(0x800D, 0x60); // sda high, scl high
  write_prg_byte(0x800D, 0x40); // sda high, scl low
  write_prg_byte(0x800D, 0x00); // sda low, scl low
}

void EepromSetAddress01(byte address) { // 24C01 [Little Endian]
  for (int i = 0; i < 7; i++) {
    if (address & 0x1) // Bit is HIGH
      EepromSet1_NES();
    else // Bit is LOW
      EepromSet0_NES();
    address >>= 1; // rotate to the next bit
  }
}

void EepromSetAddress02(byte address) { // 24C02
  for (int i = 0; i < 8; i++) {
    if ((address >> 7) & 0x1) // Bit is HIGH
      EepromSet1_NES();
    else // Bit is LOW
      EepromSet0_NES();
    address <<= 1; // rotate to the next bit
  }
  EepromStatus_NES(); // ACK
}

void EepromWriteData01() { // 24C01 [Little Endian]
  for (int i = 0; i < 8; i++) {
    if (eeptemp & 0x1) // Bit is HIGH
      EepromSet1_NES();
    else // Bit is LOW
      EepromSet0_NES();
    eeptemp >>= 1; // rotate to the next bit
  }
  EepromStatus_NES(); // ACK
}

void EepromWriteData02() { // 24C02
  for (int i = 0; i < 8; i++) {
    if ((eeptemp >> 7) & 0x1) // Bit is HIGH
      EepromSet1_NES();
    else // Bit is LOW
      EepromSet0_NES();
    eeptemp <<= 1; // rotate to the next bit
  }
  EepromStatus_NES(); // ACK
}

void EepromREAD(byte address) {
  EepromStart_NES(); // START
  if (mapper == 159) { // 24C01
    EepromSetAddress01(address); // 24C01 [Little Endian]
    EepromReadMode_NES();
    EepromReadData_NES();
    EepromFinish_NES();
    EepromStop_NES(); // STOP
    // OR 8 bits into byte
    eeptemp = eepbit[7] << 7 | eepbit[6] << 6 | eepbit[5] << 5 | eepbit[4] << 4 | eepbit[3] << 3 | eepbit[2] << 2 | eepbit[1] << 1 | eepbit[0];
  }
  else { // 24C02
    EepromDevice_NES(); // DEVICE [1010] + ADDR [A2-A0]
    EepromWriteMode_NES();
    EepromSetAddress02(address);
    EepromStart_NES(); // START
    EepromDevice_NES(); // DEVICE [1010] + ADDR [A2-A0]
    EepromReadMode_NES();
    EepromReadData_NES();
    EepromFinish_NES();
    EepromStop_NES(); // STOP
    // OR 8 bits into byte
    eeptemp = eepbit[0] << 7 | eepbit[1] << 6 | eepbit[2] << 5 | eepbit[3] << 4 | eepbit[4] << 3 | eepbit[5] << 2 | eepbit[6] << 1 | eepbit[7];
  }
  sdBuffer[address] = eeptemp;
}

void EepromWRITE(byte address) {
  eeptemp = sdBuffer[address];
  EepromStart_NES(); // START
  if (mapper == 159) { // 24C01
    EepromSetAddress01(address); // 24C01 [Little Endian]
    EepromWriteMode_NES();
    EepromWriteData01(); // 24C01 [Little Endian]
  }
  else { // 24C02
    EepromDevice_NES(); // DEVICE [1010] + ADDR [A2-A0]
    EepromWriteMode_NES();
    EepromSetAddress02(address);
    EepromWriteData02();
  }
  EepromStop_NES(); // STOP
}

/******************************************
   [NESmaker Flash Cart [SST 39SF40] Functions]
 *****************************************/
void NESmaker_ResetFlash() { // Reset Flash
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xFF); // Reset
}

// SST 39SF040 Software ID
void NESmaker_ID() { // Read Flash ID
  NESmaker_ResetFlash();
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0x90); // Software ID Entry
  unsigned char ID1 = read_prg_byte(0x8000);
  unsigned char ID2 = read_prg_byte(0x8001);
  sprintf(flashID, "%02X%02X", ID1, ID2);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xF0); // Software ID Exit
  if (strcmp(flashID, "BFB7") == 0) // SST 39SF040
    flashfound = 1;
}

void NESmaker_SectorErase(byte bank, word address) {
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0x80);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, bank); // $00-$1F
  write_prg_byte(address, 0x30); // Sector Erase ($8000/$9000/$A000/$B000)
}

void NESmaker_ByteProgram(byte bank, word address, byte data) {
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xA0);
  write_prg_byte(0xC000, bank); // $00-$1F
  write_prg_byte(address, data); // $8000-$BFFF
}

// SST 39SF040 Chip Erase [NOT IMPLEMENTED]
void NESmaker_ChipErase() { // Typical 70ms
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0x80);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0x10); // Chip Erase
}

void writeFLASH() {
  ui.clearOutput();
  if (!flashfound) {
    LED_RED_ON;
    ui.printlnMsg(F("FLASH NOT DETECTED"));
    ui.flushOutput();
  }
  else {
    ui.printMsg(F("Flash ID: "));
    ui.printlnMsg(flashID);
    ui.printlnMsg(F(""));
    ui.printlnMsg(F("NESmaker Flash Found"));
    ui.printlnMsg(F(""));
    ui.flushOutput();
    delay(100);

    String flashFilePath = fileBrowser(F("Select FLASH File"));
    word base = 0x8000;

    LED_RED_ON;
    ui.clearOutput();
    ui.printlnMsg(F("Writing File: "));
    ui.printlnMsg(flashFilePath);
    ui.flushOutput();

    //open file on sd card
    SafeSDFile flashFile = SafeSDFile::openForReading(flashFilePath);

    banks = int_pow(2, prgsize); // 256K/512K
    for (int i = 0; i < banks; i++) { // 16K Banks
      for (word sector = 0; sector < 0x4000; sector += 0x1000) { // 4K Sectors ($8000/$9000/$A000/$B000)
        // Sector Erase
        NESmaker_SectorErase(i, base + sector);
        delay(18); // Typical 18ms
        for (byte j = 0; j < 2; j++) { // Confirm erase twice
          do {
            bytecheck = read_prg_byte(base + sector);
            delay(18);
          }
          while (bytecheck != 0xFF);
        }
        // Program Byte
        for (word addr = 0x0; addr < 0x1000; addr += 512) {
          flashFile.read(sdBuffer, 512);
          for (int x = 0; x < 512; x++) {
            word location = base + sector + addr + x;
            NESmaker_ByteProgram(i, location, sdBuffer[x]);
            delayMicroseconds(14); // Typical 14us
            for (byte k = 0; k < 2; k++) { // Confirm write twice
              do {
                bytecheck = read_prg_byte(location);
                delayMicroseconds(14);
              }
              while (bytecheck != sdBuffer[x]);
            }
          }
        }
      }

      ui.printMsg(F("*"));
      if ((i + 1) % 16 == 0) {
        ui.printlnMsg(F(""));
      }
      ui.flushOutput();
    }
    flashFile.close();
    LED_GREEN_ON;

    ui.printlnMsg(F(""));
    ui.printlnMsg(F("FLASH FILE WRITTEN!"));
    ui.flushOutput();
  }
  ui.clearOutput();
  LED_RED_OFF;
  LED_GREEN_OFF;
}

//******************************************
// End of File
//******************************************
