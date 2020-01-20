//******************************************
// NES MODULE
//******************************************
// mostly copy&pasted from "Famicom Dumper" 2019-08-31 by skaman
// also based on "CoolArduino" by HardWareMan
// Pinout changes: LED and CIRAM_A10

#include "NES.h"
#include "RGB_LED.h"
#include "SD.h"
#include "filebrowser.h"
#include "globals.h"
#include "options.h"
#include "utils.h"
#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>

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
// [NESmaker Flash Cart Functions]

/******************************************
  [Supported Mappers]
 *****************************************/
struct Mapper {
  byte inesNumber;
  byte prglo;
  byte prghi;
  byte chrlo;
  byte chrhi;
  byte ramlo;
  byte ramhi;
};

const uint8_t mmc3mmc6Mapper = 4;
const uint8_t unrom512Mapper = 30;

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
const uint8_t mapcount = (sizeof(mapsize) / sizeof(mapsize[0])) / fieldsPerMapper;

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

#define MODE_READ { PORTK = 0xFF; DDRK = 0; }
#define MODE_WRITE DDRK = 0xFF

/******************************************
  [Variables]
*****************************************/

const uint16_t PROGMEM PRG[] = {16, 32, 64, 128, 256, 512};
const uint16_t PROGMEM CHR[] = {0, 8, 16, 32, 64, 128, 256, 512};
const byte PROGMEM RAM[] = {0, 8, 16, 32};

uint32_t getRamSizeBytes(uint8_t mapper, uint8_t ramIndex) {
  uint8_t ram = pgm_read_byte(&RAM[ramIndex]);
  if (mapper == 0) {
    uint8_t sizeKb = ram / 4;
    return sizeKb * static_cast<uint32_t>(1024);
  }
  else if (mapper == 16) {
    return ram * static_cast<uint32_t>(32);
  }
  else if (mapper == 159) {
    return ram * static_cast<uint32_t>(16);
  }
  else if (mapper == 19) {
    // But mapper 19 has ramlo 0, ramhi 1, this shouldn't be possible...
    if (ramIndex == 2) {
      return 128;
    }
    else {
      uint8_t sizeKb = ram;
      return sizeKb * static_cast<uint32_t>(1024);
    }
  }
  else if (mapper == 80) {
    return ram * static_cast<uint32_t>(16);
  }
  else if (mapper == 82) {
    uint8_t sizeKb = ramIndex * 5;
    return sizeKb * static_cast<uint32_t>(1024);
  }
  else {
    uint8_t sizeKb = ram;
    return sizeKb * static_cast<uint32_t>(1024);
  }
}

String NESmaker_ID();
bool checkMMC6();

struct CartridgeConfig {
  uint8_t mapper;
  uint8_t prgSizeIndex;
  uint8_t chrSizeIndex;
  uint8_t ramSizeIndex;
  bool mmc6;
  String flashID; // NESmaker 39SF040 Flash Cart - empty string if not relevant.

  void updateMapperDependentFields() {
    if (mapper != mmc3mmc6Mapper) {
      mmc6 = false;
    }
    else {
      mmc6 = checkMMC6();
    }

    if (mapper == unrom512Mapper) {
      flashID = NESmaker_ID();
    }
  }
  
  bool flashFound() const {
    return flashID.length() > 0;
  }

  // Only least significant 12 bits matter
  uint16_t getINES2PrgSize() const {
    // Can specify in 16 KB units up to 1110 1111 1111 = 3839 in decimal = 61424 KB
    // or as 2^E * M, where E is 0-63 and M is 1, 3, 5, or 7

    // 2^prgSizeIndex * 16 = kB
    // so 2^prgSize gives what we need
    return int_pow(2, prgSizeIndex);
  }

  // Only least significant 12 bits matter
  uint16_t getINES2ChrSize() const {
    // Can specify in 8 KB units up to 1110 1111 1111 = 3389 in decimal = 27112 KB
    // or as 2^E * M, where E is 0-63 and M is 1, 3, 5, or 7

    // if chrSizeIndex is 0, size is 0
    // otherwise, 2^chrSizeIndex * 4 = kB
    // so 2^(chrSizeIndex - 1) gives what we need
    // 1 -> 1
    // 2 -> 2
    // 3 -> 4
    if (chrSizeIndex == 0) {
      return 0;
    }
    else {
      return int_pow(2, chrSizeIndex - 1);
    }
  }

  uint8_t getINES2NvRamShiftCount() const {
    // 64 << nvramShiftCount = 64 * 2^nvramShiftCount = 2^(nvramShiftCount+6) = NVRAM bytes
    uint32_t ramSizeBytes = getRamSizeBytes(mapper, ramSizeIndex);
    
    if (ramSizeBytes == 0) {
      return 0;
    }

    uint8_t shiftCount = 1;
    uint32_t bytesWithShiftCount = 128;
    while (bytesWithShiftCount < ramSizeBytes) {
      shiftCount++;
      bytesWithShiftCount = bytesWithShiftCount * 2;
    }
    return shiftCount;
  }
};

enum class MirroringType : uint8_t {
  HorizontalOrMapperControlled = 0,
  Vertical = 1,
};

enum class TimingMode : uint8_t {
  NTSC = 0,
  PAL = 1,
  MultiRegion = 2,
  Dendy = 3,
};

enum class VsPPUType : uint8_t {
  RP2C03B = 0,
  RP2C03G = 1,
  RP2C04_0001 = 2,
  RP2C04_0002 = 3,
  RP2C04_0003 = 4,
  RP2C04_0004 = 5,
  RC2C03B = 6,
  RC2C03C = 7,
  RC2C05_01 = 8,
  RC2C05_02 = 9,
  RC2C05_03 = 10,
  RC2C05_04 = 11,
  RC2C05_05 = 12,
};

enum class VsHardwareType : uint8_t {
  Unisystem = 0,
  Unisystem_RBI = 1,
  Unisystem_TKO = 2,
  Unisystem_SuperSevious = 3,
  Unisystem_IceClimberJapan = 4,
  DualSystem = 5,
  DualSystem_BungelingBay = 6,
};

struct AdditionalInesHeaderFieldValues {
  // If Mirroring is listed as Vertical in bootgod's DB, use MirroringType::Vertical,
  // otherwise MirroringType::HorizontalOrMapperControlled. Not sure what the correct
  // setting is for games with four screen mirroring like Rad Racer II and Gauntlet.
  // HorizontalOrMapperControlled, I think.
  MirroringType mirroring;

  // "Battery present" in bootgod's DB
  bool hasNonVolatileMemory;

  // "Bit 3 is set only if 4 KiB of RAM are present at PPU $2000-2FFF, exclusive to that region,
  // and cannot be banked, replaced, or rearranged. Currently, only the four-screen variants of
  // mappers 4 and 206 as well as 77, 262 and the Nintendo Vs. System meet that definition."
  bool hasHardWiredFourScreenMode;

  uint8_t consoleTypeID;

  // There doesn't seem to be any game database that includes submapper, including bootgod's.
  // https://wiki.nesdev.com/w/index.php/NES_2.0_submappers probably has enough information
  // for you to figure it out.
  uint8_t submapperID;

  uint8_t volatileWorkRAMShiftCount; // aka PRG RAM. 64 << workRAMShiftCount = 64 * 2^workRAMShiftCount = work RAM bytes
  uint8_t vRAMShiftCount; // aka CHR RAM. 64 << vRAMShiftCount = 64 * 2^vRAMShiftCount = CHR RAM bytes
  uint8_t chrNVRAMShiftCount; // Very rare. 64 << chrNVRAMShiftCount = 64 * 2^chrNVRAMShiftCount = CHR NVRAM bytes
  TimingMode timingMode;
  VsPPUType vsSystemPPUTypeID;
  VsHardwareType vsSystemHardwareTypeID;
  uint8_t numMiscROMs; // Very rare
  uint8_t defaultExpansionDevice;
};

/******************************************
  [Function Prototypes]
*****************************************/

CartridgeConfig checkStatus_NES();
Mapper getMapperFromUser();
void nesCartMenu(CartridgeConfig &config);
void nesCartWriteMenu(const CartridgeConfig &config);
void setup_NES();
AdditionalInesHeaderFieldValues getAdditionalInesHeaderFieldValues(const CartridgeConfig &config);
byte getPRGSizeIndex(const Mapper &mapper);
byte getCHRSizeIndex(const Mapper &mapper);
byte getRAMSize(const Mapper &mapper);
String getOutputFolderPath();
void readPRG(const String &outputFolderPath, const CartridgeConfig &config);
void readCHR(const String &outputFolderPath, const CartridgeConfig &config);
void readRAM(const String &outputFolderPath, const CartridgeConfig &config);
void outputNES(const String &outputfolderPath, const CartridgeConfig &config, const AdditionalInesHeaderFieldValues *additionalHeaderFieldValuesPtr);
void resetROM();
void writeRAM(const CartridgeConfig &config);
void writeFLASH(const CartridgeConfig &config);
static void set_address(uint16_t address);
static void write_reg_byte(uint16_t address, uint8_t data);
void createINES2Header(const CartridgeConfig &config, const AdditionalInesHeaderFieldValues &additionalFields, byte *buffer);
bool getMapper(byte mapperNumber, Mapper *outMapper);
void EepromREAD(byte address, const CartridgeConfig &config);
void EepromWRITE(byte address, const CartridgeConfig &config);

// NES start menu
void nesMenu() {
  mode = CartReaderMode::NES;
  ui->clearOutput();
  ui->flushOutput();
  setup_NES();
  CartridgeConfig config = checkStatus_NES();
  nesCartMenu(config);
}

void nesCartMenu(CartridgeConfig &config) {
  while (true) {
    const __FlashStringHelper *item_SelectMapper = F("Select Mapper");
    const __FlashStringHelper *item_ReadEverything = F("Read Complete Cart");
    const __FlashStringHelper *item_ReadEverythingWithHeader = F("Read Cart w/header");
    const __FlashStringHelper *item_ReadPrg= F("Read PRG");
    const __FlashStringHelper *item_ReadChr = F("Read CHR");
    const __FlashStringHelper *item_ReadRam = F("Read RAM");
    const __FlashStringHelper *item_WriteOptions = F("Write Options");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_SelectMapper,
      item_ReadEverything,
      item_ReadEverythingWithHeader,
      item_ReadPrg,
      item_ReadChr,
      item_ReadRam,
      item_WriteOptions,
      item_Back,
    };

    const __FlashStringHelper *answer = ui->askMultipleChoiceQuestion(
      F("NES CART READER"), menu, ARRAY_LENGTH(menu), item_SelectMapper);

    // wait for user choice to come back from the question box menu
    if(answer == item_SelectMapper) {
      // Select Mapper
      Mapper mapper = getMapperFromUser();
      config.mapper = mapper.inesNumber;
      config.prgSizeIndex = getPRGSizeIndex(mapper);
      config.chrSizeIndex = getCHRSizeIndex(mapper);
      config.ramSizeIndex = getRAMSize(mapper);
      config.updateMapperDependentFields();
    }
    else if(answer == item_ReadEverything) {
      String outputFolderPath = getOutputFolderPath();
      readPRG(outputFolderPath, config);
      delay(2000);
      readCHR(outputFolderPath, config);
      delay(2000);
      outputNES(outputFolderPath, config, nullptr);
      delay(2000);
      readRAM(outputFolderPath, config);
      delay(2000);
      resetROM();
    }
    else if(answer == item_ReadEverythingWithHeader) {
      // Read Complete Cart and output additional file with ines header
      AdditionalInesHeaderFieldValues headerValues = getAdditionalInesHeaderFieldValues(config);
      String outputFolderPath = getOutputFolderPath();
      readPRG(outputFolderPath, config);
      delay(2000);
      readCHR(outputFolderPath, config);
      delay(2000);
      outputNES(outputFolderPath, config, &headerValues);
      delay(2000);
      readRAM(outputFolderPath, config);
      delay(2000);
      resetROM();
    }
    else if(answer == item_ReadPrg) {
      String outputFolderPath = getOutputFolderPath();
      readPRG(outputFolderPath, config);
      resetROM();
      ui->waitForUserInput();
    }
    else if(answer == item_ReadChr) {
      String outputFolderPath = getOutputFolderPath();
      readCHR(outputFolderPath, config);
      resetROM();
      ui->waitForUserInput();
    }
    else if(answer == item_ReadRam) {
      String outputFolderPath = getOutputFolderPath();
      readRAM(outputFolderPath, config);
      resetROM();
      ui->waitForUserInput();
    }
    else if(answer == item_WriteOptions) {
      nesCartWriteMenu(config);
      ui->waitForUserInput();
    }
    else if(answer == item_Back) {
      break;
    }
  }
}

AdditionalInesHeaderFieldValues getAdditionalInesHeaderFieldValues(const CartridgeConfig &config) {
  AdditionalInesHeaderFieldValues headerValues;
  
  // This setting is only relevant if the mapper does not allow the mirroring type to be switched.
  // Could theoretically skip this question based on the mapper if we included a flag for the
  // supported mappers saying whether mirroring can be switched. If it can, set the flag to 0
  // (horizontal or mapper-controlled)

  const __FlashStringHelper *mirroringItem_HorizontalOrMapper = F("Hori. or mapper cont");
  const __FlashStringHelper *mirroringItem_Vertical = F("Vertical");
  const __FlashStringHelper *mirroringMenu[] = {
    mirroringItem_HorizontalOrMapper,
    mirroringItem_Vertical,
  };

  const __FlashStringHelper* mirroringAnswer = ui->askMultipleChoiceQuestion(
    F("Mirroring type"), mirroringMenu, ARRAY_LENGTH(mirroringMenu), mirroringItem_HorizontalOrMapper);
  
  if(mirroringAnswer == mirroringItem_HorizontalOrMapper) {
    headerValues.mirroring = MirroringType::HorizontalOrMapperControlled;
  }
  else {
    headerValues.mirroring = MirroringType::Vertical;
  }

  const __FlashStringHelper *batteryItem_Yes = F("Yes");
  const __FlashStringHelper *batteryItem_No = F("No");
  const __FlashStringHelper *batteryMenu[] = {
    batteryItem_Yes,
    batteryItem_No,
  };

  const __FlashStringHelper *defaultBatteryMenuItem = batteryItem_No;
  if (config.ramSizeIndex > 0) {
    defaultBatteryMenuItem = batteryItem_Yes;
  }

  const __FlashStringHelper *batteryAnswer = ui->askMultipleChoiceQuestion(
    F("Battery or NV mem?"), batteryMenu, ARRAY_LENGTH(batteryMenu), defaultBatteryMenuItem);

  headerValues.hasNonVolatileMemory = batteryAnswer == batteryItem_Yes;

  const __FlashStringHelper *hardwiredFourScreenMirroringItem_Yes = F("Yes");
  const __FlashStringHelper *hardwiredFourScreenMirroringItem_No = F("No");
  const __FlashStringHelper *hardwiredFourScreenMirroringMenu[] = {
    hardwiredFourScreenMirroringItem_Yes,
    hardwiredFourScreenMirroringItem_No,
  };

  const __FlashStringHelper *hardwiredFourScreenMirroringAnswer = ui->askMultipleChoiceQuestion(
    F("Hardwired 4screen?"), hardwiredFourScreenMirroringMenu, ARRAY_LENGTH(hardwiredFourScreenMirroringMenu),
    hardwiredFourScreenMirroringItem_No);

  headerValues.hasHardWiredFourScreenMode = hardwiredFourScreenMirroringAnswer == hardwiredFourScreenMirroringItem_Yes;

  const __FlashStringHelper *consoleTypeItem_Regular = F("NES/FC/Dendy");
  const __FlashStringHelper *consoleTypeItem_Vs = F("Nintendo Vs. System");
  const __FlashStringHelper *consoleTypeItem_Playchoice = F("Playchoice 10");
  const __FlashStringHelper *consoleTypeItem_DecimalFamiclone = F("Dec Mode Famiclone");
  const __FlashStringHelper *consoleTypeItem_VT = F("VTxx famiclone");
  const __FlashStringHelper *consoleTypeMenu[] = {
    consoleTypeItem_Regular,
    consoleTypeItem_Vs,
    consoleTypeItem_Playchoice,
    consoleTypeItem_DecimalFamiclone,
    consoleTypeItem_VT,
  };

  const __FlashStringHelper *consoleTypeAnswer = ui->askMultipleChoiceQuestion(
    F("Console type"), consoleTypeMenu, ARRAY_LENGTH(consoleTypeMenu), consoleTypeItem_Regular);

  if(consoleTypeAnswer == consoleTypeItem_Regular) {
    headerValues.consoleTypeID = 0;
  }
  else if(consoleTypeAnswer == consoleTypeItem_Vs) {
    headerValues.consoleTypeID = 1;

    const __FlashStringHelper *ppuItem_RP2C03B = F("RP2C03B");
    const __FlashStringHelper *ppuItem_RP2C03G = F("RP2C03G");
    const __FlashStringHelper *ppuItem_RP2C04_0001 = F("RP2C04-0001");
    const __FlashStringHelper *ppuItem_RP2C04_0002 = F("RP2C04-0002");
    const __FlashStringHelper *ppuItem_RP2C04_0003 = F("RP2C04-0003");
    const __FlashStringHelper *ppuItem_RP2C04_0004 = F("RP2C04-0004");
    const __FlashStringHelper *ppuItem_RC2C03B = F("RC2C03B");
    const __FlashStringHelper *ppuItem_RC2C03C = F("RC2C03C");
    const __FlashStringHelper *ppuItem_RC2C05_01 = F("RC2C05-01");
    const __FlashStringHelper *ppuItem_RC2C05_02 = F("RC2C05-02");
    const __FlashStringHelper *ppuItem_RC2C05_03 = F("RC2C05-03");
    const __FlashStringHelper *ppuItem_RC2C05_04 = F("RC2C05-04");
    const __FlashStringHelper *ppuItem_RC2C05_05 = F("RC2C05-05");
    const String ppuMenu[] = {
      ppuItem_RP2C03B,
      ppuItem_RP2C03G,
      ppuItem_RP2C04_0001,
      ppuItem_RP2C04_0002,
      ppuItem_RP2C04_0003,
      ppuItem_RP2C04_0004,
      ppuItem_RC2C03B,
      ppuItem_RC2C03C,
      ppuItem_RC2C05_01,
      ppuItem_RC2C05_02,
      ppuItem_RC2C05_03,
      ppuItem_RC2C05_04,
      ppuItem_RC2C05_05
    };

    const uint8_t ppuAnswer = ui->askMultipleChoiceQuestion(
      F("Vs. PPU Type"), ppuMenu, ARRAY_LENGTH(ppuMenu), 0);

    headerValues.vsSystemPPUTypeID = static_cast<VsPPUType>(ppuAnswer);

    const __FlashStringHelper *vsHardwareItem_Unisystem = F("Unisystem");
    const __FlashStringHelper *vsHardwareItem_Unisystem_RBI = F("Unisystem (RBI)");
    const __FlashStringHelper *vsHardwareItem_Unisystem_TKO = F("Unisystem (TKO)");
    const __FlashStringHelper *vsHardwareItem_Unisystem_SuperXevious = F("Unisystem (Xevious)");
    const __FlashStringHelper *vsHardwareItem_Unisystem_IceClimberJapan = F("Unisystem (Ice Cl.)");
    const __FlashStringHelper *vsHardwareItem_DualSystem = F("Dual System");
    const __FlashStringHelper *vsHardwareItem_DualSystem_BungelingBay = F("Dual (Bungeling)");
    const String vsHardwareMenu[] = {
      vsHardwareItem_Unisystem,
      vsHardwareItem_Unisystem_RBI,
      vsHardwareItem_Unisystem_TKO,
      vsHardwareItem_Unisystem_SuperXevious,
      vsHardwareItem_Unisystem_IceClimberJapan,
      vsHardwareItem_DualSystem,
      vsHardwareItem_DualSystem_BungelingBay,
    };

    const uint8_t vsHardwareAnswer = ui->askMultipleChoiceQuestion(
      F("Vs. Hardware Type"), vsHardwareMenu, ARRAY_LENGTH(vsHardwareMenu), 0);

    headerValues.vsSystemHardwareTypeID = static_cast<VsHardwareType>(vsHardwareAnswer);
  }
  else if(consoleTypeAnswer == consoleTypeItem_Playchoice) {
    headerValues.consoleTypeID = 2;
  }
  else if(consoleTypeAnswer == consoleTypeItem_DecimalFamiclone) {
    headerValues.consoleTypeID = 3;
  }
  else if(consoleTypeAnswer == consoleTypeItem_VT) {
    const __FlashStringHelper *vtConsoleTypeItem_01Mono = F("VT01 mono palette");
    const __FlashStringHelper *vtConsoleTypeItem_01RedCyan = F("VT01 Red/Cy palette");
    const __FlashStringHelper *vtConsoleTypeItem_02 = F("VT02");
    const __FlashStringHelper *vtConsoleTypeItem_03 = F("VT03");
    const __FlashStringHelper *vtConsoleTypeItem_09 = F("VT09");
    const __FlashStringHelper *vtConsoleTypeItem_32 = F("VT32");
    const __FlashStringHelper *vtConsoleTypeItem_369 = F("VT369");
    const __FlashStringHelper *vtConsoleTypeMenu[] = {
      vtConsoleTypeItem_01Mono,
      vtConsoleTypeItem_01RedCyan,
      vtConsoleTypeItem_02,
      vtConsoleTypeItem_03,
      vtConsoleTypeItem_09,
      vtConsoleTypeItem_32,
      vtConsoleTypeItem_369,
    };

    const __FlashStringHelper *vtConsoleTypeAnswer = ui->askMultipleChoiceQuestion(
      F("VT console type"), vtConsoleTypeMenu, ARRAY_LENGTH(vtConsoleTypeMenu), vtConsoleTypeItem_01Mono);
    
    if(vtConsoleTypeAnswer == vtConsoleTypeItem_01Mono) {
      headerValues.consoleTypeID = 4;
    }
    else if(vtConsoleTypeAnswer == vtConsoleTypeItem_01RedCyan) {
      headerValues.consoleTypeID = 5;
    }
    else if (vtConsoleTypeAnswer == vtConsoleTypeItem_02) {
      headerValues.consoleTypeID = 6;
    }
    else if (vtConsoleTypeAnswer == vtConsoleTypeItem_03) {
      headerValues.consoleTypeID = 7;
    }
    else if (vtConsoleTypeAnswer == vtConsoleTypeItem_09) {
      headerValues.consoleTypeID = 8;
    }
    else if (vtConsoleTypeAnswer == vtConsoleTypeItem_32) {
      headerValues.consoleTypeID = 9;
    }
    else if (vtConsoleTypeAnswer == vtConsoleTypeItem_369) {
      headerValues.consoleTypeID = 10;
    }
  }

  const uint8_t submapperNumDigits = 2;
  const uint8_t submapperDefaultValue = 0;
  const uint8_t submapperMaxValue = 15;
  headerValues.submapperID = ui->readNumber(
    submapperNumDigits, submapperDefaultValue, submapperMaxValue, F("Submapper"), F("Must be 0-15"));

  // RAM shift count
  // 64 << shiftCount = 64 * 2^shiftCount = 2^(shiftCount+6) = RAM bytes
  // 128, 256, 512, 1k, 2k, 4k, 8k*, 16k, 32k, 64k, 128k
  const __FlashStringHelper *ramItem_128b = F("128 B");
  const __FlashStringHelper *ramItem_256b = F("256 B");
  const __FlashStringHelper *ramItem_512b = F("512 B");
  const __FlashStringHelper *ramItem_1k = F("1 KB");
  const __FlashStringHelper *ramItem_2k = F("2 KB");
  const __FlashStringHelper *ramItem_4k = F("4 KB");
  const __FlashStringHelper *ramItem_8k = F("8 KB");
  const __FlashStringHelper *ramItem_16k = F("16 KB");
  const __FlashStringHelper *ramItem_32k = F("32 KB");
  const __FlashStringHelper *ramItem_64k = F("64 KB");
  const __FlashStringHelper *ramItem_128k = F("128 KB");
  const String ramMenu[] = {
    ramItem_128b,
    ramItem_256b,
    ramItem_512b,
    ramItem_1k,
    ramItem_2k,
    ramItem_4k,
    ramItem_8k,
    ramItem_16k,
    ramItem_32k,
    ramItem_64k,
    ramItem_128k,
  };

  uint8_t ramIndex8k = 0;
  for (ramIndex8k = 0; ramIndex8k < ARRAY_LENGTH(ramMenu); ramIndex8k++) {
    if (ramMenu[ramIndex8k] == ramItem_8k) {
      break;
    }
  }

  const __FlashStringHelper *hasVolatileWorkRamItem_Yes = F("Yes");
  const __FlashStringHelper *hasVolatileWorkRamItem_No = F("No");
  const __FlashStringHelper *hasVolatileWorkRamMenu[] = {
    hasVolatileWorkRamItem_Yes,
    hasVolatileWorkRamItem_No,
  };

  const __FlashStringHelper *hasVolatileWorkRamAnswer = ui->askMultipleChoiceQuestion(
    F("Vol WorkRAM/PRG-RAM"), hasVolatileWorkRamMenu, ARRAY_LENGTH(hasVolatileWorkRamMenu), hasVolatileWorkRamItem_No);
  
  if (hasVolatileWorkRamAnswer == hasVolatileWorkRamItem_No) {
    headerValues.volatileWorkRAMShiftCount = 0;
  }
  else {
    uint8_t workRamAnswerIndex = ui->askMultipleChoiceQuestion(
      F("Vol WorkRAM/PRG-RAM"), ramMenu, ARRAY_LENGTH(ramMenu), ramIndex8k);

    headerValues.volatileWorkRAMShiftCount = workRamAnswerIndex + 1;
  }

  const __FlashStringHelper *hasVRamItem_Yes = F("Yes");
  const __FlashStringHelper *hasVRamItem_No = F("No");
  const __FlashStringHelper *hasVRamMenu[] = {
    hasVRamItem_Yes,
    hasVRamItem_No,
  };

  const __FlashStringHelper *hasVRamAnswer = ui->askMultipleChoiceQuestion(
    F("vRAM/CHR-RAM?"), hasVRamMenu, ARRAY_LENGTH(hasVRamMenu), hasVRamItem_No);

  if (hasVRamAnswer == hasVRamItem_No) {
    headerValues.vRAMShiftCount = 0;
  }
  else {
    uint8_t vRamAnswerIndex = ui->askMultipleChoiceQuestion(
      F("vRAM/CHR-RAM"), ramMenu, ARRAY_LENGTH(ramMenu), ramIndex8k);

    headerValues.vRAMShiftCount = vRamAnswerIndex + 1;
  }

  const __FlashStringHelper *hasChrNvramItem_Yes = F("Yes");
  const __FlashStringHelper *hasChrNvramItem_No = F("No");
  const __FlashStringHelper *hasChrNvramMenu[] = {
    hasChrNvramItem_Yes,
    hasChrNvramItem_No,
  };

  const __FlashStringHelper *hasChrNvramAnswer = ui->askMultipleChoiceQuestion(
    F("CHR-NVRAM (rare)?"), hasChrNvramMenu, ARRAY_LENGTH(hasChrNvramMenu), hasChrNvramItem_No);
  
  if (hasChrNvramAnswer == hasChrNvramItem_No) {
    headerValues.chrNVRAMShiftCount = 0;
  }
  else {
    uint8_t chrNvramAnswerIndex = ui->askMultipleChoiceQuestion(
      F("CHR-NVRAM"), ramMenu, ARRAY_LENGTH(ramMenu), ramIndex8k);

    headerValues.chrNVRAMShiftCount = chrNvramAnswerIndex + 1;
  }

  const __FlashStringHelper *timingItem_NTSC = F("NTSC");
  const __FlashStringHelper *timingItem_PAL = F("PAL");
  const __FlashStringHelper *timingItem_Multi = F("Multi-region");
  const __FlashStringHelper *timingItem_Dendy = F("Dendy");
  const __FlashStringHelper *timingMenu[] = {
    timingItem_NTSC,
    timingItem_PAL,
    timingItem_Multi,
    timingItem_Dendy,
  };

  const __FlashStringHelper *timingAnswer = ui->askMultipleChoiceQuestion(
    F("Timing"), timingMenu, ARRAY_LENGTH(timingMenu), timingItem_NTSC);
  
  if (timingAnswer == timingItem_NTSC) {
    headerValues.timingMode = TimingMode::NTSC;
  }
  else if (timingAnswer == timingItem_PAL) {
    headerValues.timingMode = TimingMode::PAL;
  }
  else if (timingAnswer == timingItem_Multi) {
    headerValues.timingMode = TimingMode::MultiRegion;
  }
  else if (timingAnswer == timingItem_Dendy) {
    headerValues.timingMode = TimingMode::Dendy;
  }

  // This cart reader code does not support extra ROMs
  headerValues.numMiscROMs = 0;

  const uint8_t defaultDeviceNumDigits = 2;
  const uint8_t defaultDeviceDefault = 0;
  const uint8_t defaultDeviceMaxValue = 63;
  headerValues.defaultExpansionDevice = ui->readNumber(
    defaultDeviceNumDigits, defaultDeviceDefault, defaultDeviceMaxValue, F("Default Device"), F("Must be 0-63"));

  return headerValues;
}

void nesCartWriteMenu(const CartridgeConfig &config) {
  while (true) {
    const __FlashStringHelper *writeMenuItem_RAM = F("Write RAM");
    const __FlashStringHelper *writeMenuItem_Flash = F("Write FLASH");
    const __FlashStringHelper *writeMenuItem_Return = F("Return to Main Menu");
    const __FlashStringHelper *writeMenu[] = {
      writeMenuItem_RAM,
      writeMenuItem_Flash,
      writeMenuItem_Return,
    };

    const __FlashStringHelper *writeAnswer = ui->askMultipleChoiceQuestion(
      F("WRITE OPTIONS MENU"), writeMenu, ARRAY_LENGTH(writeMenu), writeMenuItem_RAM);
    
    if(writeAnswer == writeMenuItem_RAM) {
      writeRAM(config);
      resetROM();
      ui->waitForUserInput();
      break;
    }
    else if(writeAnswer == writeMenuItem_Flash) {
      if (config.mapper == 30) {
        writeFLASH(config);
      }
      resetROM();
      ui->waitForUserInput();
      break;
    }
    else if(writeAnswer == writeMenuItem_Return) {
      break; // return from this function to the main NES menu
    }
  }
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
  ledRedOff();
  ledGreenOff();
  ledBlueOff();
}

/******************************************
   [Low Level Functions]
 *****************************************/

static void set_address(uint16_t address) {
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

static void set_romsel(uint16_t address) {
  if (address & 0x8000) {
    ROMSEL_LOW;
  } else {
    ROMSEL_HI;
  }
}

static unsigned char read_prg_byte(uint16_t address) {
  MODE_READ;
  PRG_READ;
  set_address(address);
  PHI2_HI;
  set_romsel(address);
  _delay_us(1);
  return PINK;
}

static unsigned char read_chr_byte(uint16_t address) {
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

static void write_prg_byte(uint16_t address, uint8_t data) {
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

void write_mmc1_byte(uint16_t address, uint8_t data) { // write loop for 5 bit register
  if (address >= 0xE000) {
    for (uint8_t i = 0; i < 5; i++) {
      write_reg_byte(address, data >> i); // shift 1 bit into temp register [WRITE RAM SAFE]
    }
  }
  else {
    for (uint8_t j = 0; j < 5; j++) {
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
static void write_reg_byte(uint16_t address, uint8_t data) { // FIX FOR MMC1 RAM CORRUPTION
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

static void write_ram_byte(uint16_t address, uint8_t data) { // Mapper 19 (Namco 106/163) WRITE RAM SAFE ($E000-$FFFF)
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

static void write_wram_byte(uint16_t address, uint8_t data) { // Mapper 5 (MMC5) RAM
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
inline uint32_t updateCRC32(uint8_t ch, uint32_t crc) {
  uint32_t idx = ((crc) ^ (ch)) & 0xff;
  uint32_t tab_value = pgm_read_dword(crc_32_tab + idx);
  return tab_value ^ ((crc) >> 8);
}

uint32_t crc32(File &file, const char *checkFile) {
  uint32_t oldcrc32 = 0xFFFFFFFF;
  size_t numBytesRead;
  while ((numBytesRead = readFile(file, sdBuffer, sizeof(sdBuffer), checkFile)) > 0) {
    for (size_t x = 0; x < numBytesRead; x++) {
      uint8_t c = sdBuffer[x];
      oldcrc32 = updateCRC32(c, oldcrc32);
    }
  }
  return ~oldcrc32;
}

void calcCRC(const char* checkFile) {
  File crcFile = openForReading(sd, checkFile);
  uint32_t crc = crc32(crcFile, checkFile);
  closeFile(crcFile, checkFile);
  
  char crcString[9];
  sprintf(crcString, "%08lX", crc);

  ui->printMsg(F("CRC: "));
  ui->printlnMsg(crcString);
  ui->flushOutput();
}

// If additionalHeaderFieldValuesPtr is non-null, output a CART.nes file that contains an iNES 2 header,
// in addition to the CART.bin file consisting of PRG + CHR data. If null, just output the CART.bin file.
// The iNES header requires some additional information to be entered that maybe the user doesn't want
// to bother entering.
void outputNES(const String &outputfolderPath, const CartridgeConfig &config, const AdditionalInesHeaderFieldValues* additionalHeaderFieldValuesPtr) {
  ui->clearOutput();

  ledRedOn();
  ledGreenOn();
  ledBlueOn();

  String prgFilePath = pathJoin(outputfolderPath, F("PRG.bin"));
  File prgFile = openForReading(sd, prgFilePath);

  String fullCartFilePath = pathJoin(outputfolderPath, F("CART.bin"));
  File fullCartFile = openForCreating(sd, fullCartFilePath);

  File fullCartFileWithHeader;
  String fullCartFileWithHeaderPath;
  if (additionalHeaderFieldValuesPtr != nullptr) {
    fullCartFileWithHeaderPath = pathJoin(outputfolderPath, F("CART.nes"));
    fullCartFileWithHeader = openForCreating(sd, fullCartFileWithHeaderPath);

    byte header[16];
    createINES2Header(config, *additionalHeaderFieldValuesPtr, header);
    writeFile(fullCartFileWithHeader, header, 16, fullCartFileWithHeaderPath);
  }

  size_t numBytesRead;
  while ((numBytesRead = readFile(prgFile, sdBuffer, sizeof(sdBuffer), prgFilePath)) > 0) {
    writeFile(fullCartFile, sdBuffer, numBytesRead, fullCartFilePath);

    if (additionalHeaderFieldValuesPtr != nullptr) {
      writeFile(fullCartFileWithHeader, sdBuffer, numBytesRead, fullCartFileWithHeaderPath);
    }
  }

  prgFile.close();

  if (config.chrSizeIndex > 0) {
    String chrFilePath = pathJoin(outputfolderPath, F("CHR.bin"));
    File chrFile = openForReading(sd, chrFilePath);

    while ((numBytesRead = readFile(chrFile, sdBuffer, sizeof(sdBuffer), chrFilePath)) > 0) {
      writeFile(fullCartFile, sdBuffer, numBytesRead, fullCartFilePath);

      if (additionalHeaderFieldValuesPtr != nullptr) {
        writeFile(fullCartFileWithHeader, sdBuffer, numBytesRead, fullCartFileWithHeaderPath);
      }
    }

    chrFile.close();
  }

  fullCartFile.close();

  if (additionalHeaderFieldValuesPtr != nullptr) {
    fullCartFileWithHeader.close();
  }

  ui->printlnMsg(F("NES FILE OUTPUT!"));
  ui->printlnMsg(F(""));
  ui->flushOutput();

  calcCRC(fullCartFilePath.c_str());
  ledRedOff();
  ledGreenOff();
  ledBlueOff();
}

// buffer must have space for 16 bytes
void createINES2Header(const CartridgeConfig &config, const AdditionalInesHeaderFieldValues &additionalFields, byte *buffer) {
  // https://wiki.nesdev.com/w/index.php/NES_2.0

  buffer[0] = 'N';
  buffer[1] = 'E';
  buffer[2] = 'S';
  buffer[3] = '\x1A'; // EOF

  // PRG-ROM size LSB
  // Can specify in 16 KB units up to 1110 1111 1111 =  3839 in decimal = 61424 kB
  // or as 2^E * M, where E is 0-63 and M is 1, 3, 5, or 7
  // 2^prgSize * 16 = kB
  // so 2^prgSize gives
  uint16_t prgSize = config.getINES2PrgSize();

  buffer[4] = lowByte(prgSize);

  uint16_t chrSize = config.getINES2ChrSize();

  buffer[5] = lowByte(chrSize);

  if (additionalFields.mirroring == MirroringType::HorizontalOrMapperControlled) {
    bitWrite(buffer[6], 0, 0);
  }
  else {
    bitWrite(buffer[6], 0, 1);
  }

  if (additionalFields.hasNonVolatileMemory) {
    bitWrite(buffer[6], 1, 1);
  }
  else {
    bitWrite(buffer[6], 1, 0);
  }

  bitWrite(buffer[6], 2, 0); // 512-byte Trainer never present in real cartridge dumps

  if (additionalFields.hasHardWiredFourScreenMode) {
    bitWrite(buffer[6], 3, 1);
  }
  else {
    bitWrite(buffer[6], 3, 0);
  }

  // bits 4-7 are least significant 4 bits of mapper number
  bitWrite(buffer[6], 4, bitRead(config.mapper, 0));
  bitWrite(buffer[6], 5, bitRead(config.mapper, 1));
  bitWrite(buffer[6], 6, bitRead(config.mapper, 2));
  bitWrite(buffer[6], 7, bitRead(config.mapper, 3));

  if (additionalFields.consoleTypeID < 3) {
    bitWrite(buffer[7], 0, bitRead(additionalFields.consoleTypeID, 0));
    bitWrite(buffer[7], 1, bitRead(additionalFields.consoleTypeID, 1));
  }
  else {
    bitWrite(buffer[7], 0, 1);
    bitWrite(buffer[7], 1, 1);
  }

  // NES 2.0 identifier
  bitWrite(buffer[7], 2, 0);
  bitWrite(buffer[7], 3, 1);

  // bits 4-7 are bits 4-7 of mapper number
  bitWrite(buffer[7], 4, bitRead(config.mapper, 4));
  bitWrite(buffer[7], 5, bitRead(config.mapper, 5));
  bitWrite(buffer[7], 6, bitRead(config.mapper, 6));
  bitWrite(buffer[7], 7, bitRead(config.mapper, 7));

  // bits 0-4 are bits 8-11 of mapper number, but this cart reading code doesn't support any mappers above 255.
  bitWrite(buffer[8], 0, 0);
  bitWrite(buffer[8], 1, 0);
  bitWrite(buffer[8], 2, 0);
  bitWrite(buffer[8], 3, 0);

  bitWrite(buffer[8], 4, bitRead(additionalFields.submapperID, 0));
  bitWrite(buffer[8], 5, bitRead(additionalFields.submapperID, 1));
  bitWrite(buffer[8], 6, bitRead(additionalFields.submapperID, 2));
  bitWrite(buffer[8], 7, bitRead(additionalFields.submapperID, 3));

  bitWrite(buffer[9], 0, bitRead(prgSize, 8));
  bitWrite(buffer[9], 1, bitRead(prgSize, 9));
  bitWrite(buffer[9], 2, bitRead(prgSize, 10));
  bitWrite(buffer[9], 3, bitRead(prgSize, 11));

  bitWrite(buffer[9], 4, bitRead(chrSize, 8));
  bitWrite(buffer[9], 5, bitRead(chrSize, 9));
  bitWrite(buffer[9], 6, bitRead(chrSize, 10));
  bitWrite(buffer[9], 7, bitRead(chrSize, 11));

  bitWrite(buffer[10], 0, bitRead(additionalFields.volatileWorkRAMShiftCount, 0));
  bitWrite(buffer[10], 1, bitRead(additionalFields.volatileWorkRAMShiftCount, 1));
  bitWrite(buffer[10], 2, bitRead(additionalFields.volatileWorkRAMShiftCount, 2));
  bitWrite(buffer[10], 3, bitRead(additionalFields.volatileWorkRAMShiftCount, 3));

  uint8_t nvramShiftCount = config.getINES2NvRamShiftCount();
  bitWrite(buffer[10], 4, bitRead(nvramShiftCount, 0));
  bitWrite(buffer[10], 5, bitRead(nvramShiftCount, 1));
  bitWrite(buffer[10], 6, bitRead(nvramShiftCount, 2));
  bitWrite(buffer[10], 7, bitRead(nvramShiftCount, 3));

  bitWrite(buffer[11], 0, bitRead(additionalFields.vRAMShiftCount, 0));
  bitWrite(buffer[11], 1, bitRead(additionalFields.vRAMShiftCount, 1));
  bitWrite(buffer[11], 2, bitRead(additionalFields.vRAMShiftCount, 2));
  bitWrite(buffer[11], 3, bitRead(additionalFields.vRAMShiftCount, 3));

  bitWrite(buffer[11], 4, bitRead(additionalFields.chrNVRAMShiftCount, 0));
  bitWrite(buffer[11], 5, bitRead(additionalFields.chrNVRAMShiftCount, 1));
  bitWrite(buffer[11], 6, bitRead(additionalFields.chrNVRAMShiftCount, 2));
  bitWrite(buffer[11], 7, bitRead(additionalFields.chrNVRAMShiftCount, 3));

  uint8_t timingMode = static_cast<uint8_t>(additionalFields.timingMode);
  bitWrite(buffer[12], 0, bitRead(timingMode, 0));
  bitWrite(buffer[12], 1, bitRead(timingMode, 1));
  bitWrite(buffer[12], 2, 0);
  bitWrite(buffer[12], 3, 0);
  bitWrite(buffer[12], 4, 0);
  bitWrite(buffer[12], 5, 0);
  bitWrite(buffer[12], 6, 0);
  bitWrite(buffer[12], 7, 0);

  // Vs. System
  if (additionalFields.consoleTypeID == 1) {
    uint8_t ppuType = static_cast<uint8_t>(additionalFields.vsSystemPPUTypeID);
    bitWrite(buffer[13], 0, bitRead(ppuType, 0));
    bitWrite(buffer[13], 1, bitRead(ppuType, 1));
    bitWrite(buffer[13], 2, bitRead(ppuType, 2));
    bitWrite(buffer[13], 3, bitRead(ppuType, 3));

    uint8_t hardwareType = static_cast<uint8_t>(additionalFields.vsSystemHardwareTypeID);
    bitWrite(buffer[13], 4, bitRead(hardwareType, 0));
    bitWrite(buffer[13], 5, bitRead(hardwareType, 1));
    bitWrite(buffer[13], 6, bitRead(hardwareType, 2));
    bitWrite(buffer[13], 7, bitRead(hardwareType, 3));
  }
  else if (additionalFields.consoleTypeID >= 3) {
    // bits 2-5 of console id
    bitWrite(buffer[13], 0, bitRead(additionalFields.consoleTypeID, 2));
    bitWrite(buffer[13], 1, bitRead(additionalFields.consoleTypeID, 3));
    bitWrite(buffer[13], 2, bitRead(additionalFields.consoleTypeID, 4));
    bitWrite(buffer[13], 3, bitRead(additionalFields.consoleTypeID, 5));
    bitWrite(buffer[13], 4, 0);
    bitWrite(buffer[13], 5, 0);
    bitWrite(buffer[13], 6, 0);
    bitWrite(buffer[13], 7, 0);
  }
  else {
    buffer[13] = 0;
  }

  bitWrite(buffer[14], 0, bitRead(additionalFields.numMiscROMs, 0));
  bitWrite(buffer[14], 1, bitRead(additionalFields.numMiscROMs, 1));
  bitWrite(buffer[14], 2, 0);
  bitWrite(buffer[14], 3, 0);
  bitWrite(buffer[14], 4, 0);
  bitWrite(buffer[14], 5, 0);
  bitWrite(buffer[14], 6, 0);
  bitWrite(buffer[14], 7, 0);

  bitWrite(buffer[15], 0, bitRead(additionalFields.defaultExpansionDevice, 0));
  bitWrite(buffer[15], 1, bitRead(additionalFields.defaultExpansionDevice, 1));
  bitWrite(buffer[15], 2, bitRead(additionalFields.defaultExpansionDevice, 2));
  bitWrite(buffer[15], 3, bitRead(additionalFields.defaultExpansionDevice, 3));
  bitWrite(buffer[15], 4, bitRead(additionalFields.defaultExpansionDevice, 4));
  bitWrite(buffer[15], 5, bitRead(additionalFields.defaultExpansionDevice, 5));
  bitWrite(buffer[15], 6, 0);
  bitWrite(buffer[15], 7, 0);
}

void writeINES2Header(File &file, const CartridgeConfig &config, const AdditionalInesHeaderFieldValues &additionalFields, const String &filePath) {
  byte buffer[16];
  createINES2Header(config, additionalFields, buffer);
  writeFile(file, buffer, 16, filePath);
}

String getOutputFolderPath() {
  int16_t folderNumber = loadFolderNumber();
  saveFolderNumber(folderNumber + 1);
  String folderPath(F("/NES/CART/"));
  folderPath.concat(folderNumber);
  mkdir(sd, folderPath, true);
  return folderPath;
}

/******************************************
   [Config Functions]
 *****************************************/
Mapper getMapperFromUser() {
  // Read stored mapper
  byte storedMapper = loadNESMapperNumber();

  byte newmapper = storedMapper;
  if (newmapper > 220) {
    newmapper = 0;
  }

  String prompt;
  if (ui->supportsLargeMessages()) {
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
  Mapper mapper;
  while (!validMapper) {
    newmapper = (byte) ui->readNumber(3, newmapper, 220, prompt, F("Mapper not supported"));

    // Check if valid
    validMapper = getMapper(newmapper, &mapper);

    if (!validMapper) {
      errorLvl = 1;
      ui->displayMessage(F("Mapper not supported"));
    }
  }

  saveNESMapperNumber(newmapper);
  return mapper;
}

bool getMapper(uint8_t mapperNumber, Mapper *outMapper) {
  for (uint8_t i = 0; i < mapcount; i++) {
    uint16_t index = i * 7;
    byte mapcheck = pgm_read_byte(mapsize + index);
    if (mapcheck == mapperNumber) {
      outMapper->prglo = pgm_read_byte(mapsize + index + 1);
      outMapper->prghi = pgm_read_byte(mapsize + index + 2);
      outMapper->chrlo = pgm_read_byte(mapsize + index + 3);
      outMapper->chrhi = pgm_read_byte(mapsize + index + 4);
      outMapper->ramlo = pgm_read_byte(mapsize + index + 5);
      outMapper->ramhi = pgm_read_byte(mapsize + index + 6);
      return true;
    }
  }

  return false;
}

byte getPRGSizeIndex(const Mapper &mapper) {
  byte prgSizeIndex;
  // If only one possible PRG size for this mapper, use it, no need to ask for user input.
  if (mapper.prglo == mapper.prghi) {
    prgSizeIndex = mapper.prglo;
  }
  else {
    // Have user choose a size from [prglo, ..., prghi] from the predetermined sizes in the PRG array
    uint8_t numChoices = mapper.prghi - mapper.prglo + 1;
    String prgSizeChoices[numChoices];
    for(uint8_t prgIndex = mapper.prglo; prgIndex <= mapper.prghi; prgIndex++) {
      uint16_t prg;
      memcpy_P(&prg, &PRG[prgIndex], sizeof(PRG[prgIndex]));
      prgSizeChoices[prgIndex - mapper.prglo] = String(prg) + F(" kB");
    }
    
    uint8_t choiceIndex = ui->askMultipleChoiceQuestion(F("PRG Size:"), prgSizeChoices, numChoices, 0);
    prgSizeIndex = mapper.prglo + choiceIndex;
  }

  saveNESPRG(prgSizeIndex);
  return prgSizeIndex;
}

byte getCHRSizeIndex(const Mapper &mapper) {
  byte chrSizeIndex;
  // If only one possible CHR size for this mapper, use it, no need to ask for user input.
  if (mapper.chrlo == mapper.chrhi) {
    chrSizeIndex = mapper.chrlo;
  }
  else {
    // Have user choose a size from [chrlo, ..., chrhi] from the predetermined sizes in the CHR array
    uint8_t numChoices = mapper.chrhi - mapper.chrlo + 1;
    String chrSizeChoices[numChoices];
    for(uint8_t chrIndex = mapper.chrlo; chrIndex <= mapper.chrhi; chrIndex++) {
      uint16_t chr;
      memcpy_P(&chr, &CHR[chrIndex], sizeof(CHR[chrIndex]));
      chrSizeChoices[chrIndex - mapper.chrlo] = String(chr) + F(" kB");
    }

    uint8_t choiceIndex = ui->askMultipleChoiceQuestion(F("CHR Size:"), chrSizeChoices, numChoices, 0);
    chrSizeIndex = mapper.chrlo + choiceIndex;
  }

  saveNESCHR(chrSizeIndex);
  return chrSizeIndex;
}

byte getRAMSize(const Mapper &mapper) {
  byte ramSizeIndex;
  // If only one possible RAM size for this mapper, use it, no need to ask for user input.
  if (mapper.ramlo == mapper.ramhi) {
    ramSizeIndex = mapper.ramlo;
  }
  else {
    const __FlashStringHelper *prompt;
    if (mapper.inesNumber == 16 || mapper.inesNumber == 159) {
      prompt = F("EEPROM Size:");
    }
    else {
      prompt = F("RAM Size:");
    }

    uint8_t numChoices = mapper.ramhi - mapper.ramlo + 1;
    String ramSizeChoices[numChoices];
    for(uint8_t ramIndex = mapper.ramlo; ramIndex <= mapper.ramhi; ramIndex++) {
      uint32_t ramSizeBytes = getRamSizeBytes(mapper.inesNumber, ramIndex);
      if (ramSizeBytes % 1024 == 0) {
        ramSizeChoices[ramIndex - mapper.ramlo] = String(ramSizeBytes / 1024) + F(" kB");
      }
      else {
        ramSizeChoices[ramIndex - mapper.ramlo] = String(ramSizeBytes) + F(" B");
      }
    }

    uint8_t choiceIndex = ui->askMultipleChoiceQuestion(prompt, ramSizeChoices, numChoices, 0);
    ramSizeIndex = mapper.ramlo + choiceIndex;
  }
  saveNESRAM(ramSizeIndex);
  return ramSizeIndex;
}

// MMC6 Detection
// Mapper 4 includes both MMC3 AND MMC6
// RAM is mapped differently between MMC3 and MMC6
bool checkMMC6() { // Detect MMC6 Carts - read PRG 0x3E00A ("STARTROPICS")
  write_prg_byte(0x8000, 6); // PRG Bank 0 ($8000-$9FFF)
  write_prg_byte(0x8001, 0x1F); // 0x3E000
  byte prgchk0 = read_prg_byte(0x800A);
  byte prgchk1 = read_prg_byte(0x800B);
  byte prgchk2 = read_prg_byte(0x800C);
  byte prgchk3 = read_prg_byte(0x800D);

  return prgchk0 == 0x53 && prgchk1 == 0x54 && prgchk2 == 0x41 && prgchk3 == 0x52;
}

CartridgeConfig checkStatus_NES() {
  CartridgeConfig config;
  config.mapper = loadNESMapperNumber();
  config.prgSizeIndex = loadNESPRG();
  config.chrSizeIndex = loadNESCHR();
  config.ramSizeIndex = loadNESRAM();
  config.updateMapperDependentFields();

  uint16_t prgKb = (int_pow(2, config.prgSizeIndex)) * 16;

  uint16_t chrKb;
  if (config.chrSizeIndex == 0)
    chrKb = 0; // 0K
  else
    chrKb = (int_pow(2, config.chrSizeIndex)) * 4;

  // ???
  uint16_t ramSize;
  if (config.ramSizeIndex == 0)
    ramSize = 0; // 0K
  else if (config.mapper == 82)
    ramSize = 5; // 5K
  else
    ramSize = (int_pow(2, config.ramSizeIndex)) * 4;

  if (config.mmc6) {
    ramSize = 1; // 1K
  }

  ui->clearOutput();
  ui->printlnMsg(F("NES CART READER"));
  ui->printlnMsg(F("CURRENT SETTINGS"));
  ui->printlnMsg(F(""));
  ui->printMsg(F("MAPPER:   "));
  ui->printlnMsg(config.mapper);
  ui->printMsg(F("PRG SIZE: "));
  ui->printMsg(prgKb);
  ui->printlnMsg(F("K"));
  ui->printMsg(F("CHR SIZE: "));
  ui->printMsg(chrKb);
  ui->printlnMsg(F("K"));
  ui->printMsg(F("RAM SIZE: "));
  if (config.mapper == 0) {
    ui->printMsg(ramSize / 4);
    ui->printlnMsg(F("K"));
  }
  else if ((config.mapper == 16) || (config.mapper == 80) || (config.mapper == 159)) {
    if (config.mapper == 16)
      ui->printMsg(ramSize * 32);
    else
      ui->printMsg(ramSize * 16);
    ui->printlnMsg(F("B"));
  }
  else if (config.mapper == 19) {
    if (config.ramSizeIndex == 2)
      ui->printlnMsg(F("128B"));
    else {
      ui->printMsg(ramSize);
      ui->printlnMsg(F("K"));
    }
  }
  else {
    ui->printMsg(ramSize);
    ui->printlnMsg(F("K"));
  }
  ui->flushOutput();
  ui->waitForUserInput();
  return config;
}

/******************************************
   [ROM Functions]
 *****************************************/
void dumpPRG(File& outFile, uint16_t base, uint16_t address, const String& outFilePath) {
  for (uint16_t x = 0; x < 512; x++) {
    sdBuffer[x] = read_prg_byte(base + address + x);
  }
  writeFile(outFile, sdBuffer, 512, outFilePath);
}

void dumpCHR(File& outFile, uint16_t address, const String& outFilePath) {
  for (uint16_t x = 0; x < 512; x++) {
    sdBuffer[x] = read_chr_byte(address + x);
  }
  writeFile(outFile, sdBuffer, 512, outFilePath);
}

void dumpMMC5RAM(File& outFile, uint16_t base, uint16_t address, const String& outFilePath) { // MMC5 SRAM DUMP - PULSE M2 LO/HI
  for (uint16_t x = 0; x < 512; x++) {
    PHI2_LOW;
    sdBuffer[x] = read_prg_byte(base + address + x);
  }
  writeFile(outFile, sdBuffer, 512, outFilePath);
}

void writeMMC5RAM(File& inFile, uint16_t base, uint16_t address, const String& inFilePath) { // MMC5 SRAM WRITE
  readFile(inFile, sdBuffer, 512, inFilePath);
  for (uint16_t x = 0; x < 512; x++) {
    byte bytecheck;
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

void readPRG(const String &outputFolderPath, const CartridgeConfig &config) {
  ui->clearOutput();
  ui->flushOutput();

  ledBlueOn();
  set_address(0);
  _delay_us(1);
  String prgFilePath = pathJoin(outputFolderPath, F("PRG.bin"));
  File prgFile = openForCreating(sd, prgFilePath);
  uint16_t base = 0x8000;
  switch (config.mapper) {
    case 0:
    case 3:
    case 13:
    case 87: // 16K/32K
    case 184: // 32K
    case 185: { // 16K/32K
      for (uint16_t address = 0; address < ((config.prgSizeIndex * 0x4000U) + 0x4000U); address += 512) { // 16K or 32K
        dumpPRG(prgFile, base, address, prgFilePath);
      }
      break;
    }

    case 1:
    case 155: { // 32K/64K/128K/256K/512K
      uint16_t banks = int_pow(2, config.prgSizeIndex) - 1;
      for (uint16_t i = 0; i < banks; i++) { // 16K Banks ($8000-$BFFF)
        write_prg_byte(0x8000, 0x80); // Clear Register
        write_mmc1_byte(0x8000, 0x0C); // Switch 16K Bank ($8000-$BFFF) + Fixed Last Bank ($C000-$FFFF)
        if (config.prgSizeIndex > 4) // 512K
          write_mmc1_byte(0xA000, 0x00); // Reset 512K Flag for Lower 256K
        if (i > 15) // Switch Upper 256K
          write_mmc1_byte(0xA000, 0x10); // Set 512K Flag
        write_mmc1_byte(0xE000, i);
        for (uint16_t address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      for (uint16_t address = 0x4000; address < 0x8000; address += 512) { // Final Bank ($C000-$FFFF)
        dumpPRG(prgFile, base, address, prgFilePath);
      }
      break;
    }

    case 2: { // 128K/256K
      for (uint8_t i = 0; i < 8; i++) { // 128K/256K
        write_prg_byte(0x8000, i);
        for (uint16_t address = 0x0; address < (((config.prgSizeIndex - 3U) * 0x4000U) + 0x4000U); address += 512) {
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 4:
    case 47:
    case 118:
    case 119: {
      uint16_t banks = ((int_pow(2, config.prgSizeIndex) * 2)) - 2;  // Set Number of Banks
      if (config.mapper == 47)
        write_prg_byte(0xA001, 0x80); // Block Register - PRG RAM Chip Enable, Writable
      for (uint16_t i = 0; i < banks; i += 2) { // 32K/64K/128K/256K/512K
        if (config.mapper == 47) {
          if (i == 0)
            write_prg_byte(0x6000, 0); // Switch to Lower Block
          else if (i == 16)
            write_prg_byte(0x6000, 1); // Switch to Upper Block
        }
        write_prg_byte(0x8000, 6); // PRG Bank 0 ($8000-$9FFF)
        write_prg_byte(0x8001, i);
        write_prg_byte(0x8000, 7); // PRG Bank 1 ($A000-$BFFF)
        write_prg_byte(0x8001, i + 1);
        for (uint16_t address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      for (uint16_t address = 0x4000; address < 0x8000; address += 512) { // Final 2 Banks ($C000-$FFFF)
        dumpPRG(prgFile, base, address, prgFilePath);
      }
      break;
    }

    case 5: { // 128K/256K/512K
      uint16_t banks = int_pow(2, config.prgSizeIndex) * 2;
      write_prg_byte(0x5100, 3); // 8K PRG Banks
      for (uint16_t i = 0; i < banks; i += 2) { // 128K/256K/512K
        write_prg_byte(0x5114, i | 0x80);
        write_prg_byte(0x5115, (i + 1) | 0x80);
        for (uint16_t address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 7: // 128K/256K
    case 34:
    case 77:
    case 96: { // 128K
      uint16_t banks = int_pow(2, config.prgSizeIndex) / 2;
      for (uint16_t i = 0; i < banks; i++) { // 32K Banks
        write_prg_byte(0x8000, i);
        for (uint16_t address = 0x0; address < 0x8000; address += 512) { // 32K Banks ($8000-$FFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 9: { // 128K
      for (uint8_t i = 0; i < 13; i++) { // 16-3 = 13 = 128K
        write_prg_byte(0xA000, i); // $8000-$9FFF
        for (uint16_t address = 0x0; address < 0x2000; address += 512) { // Switch Bank ($8000-$9FFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      for (uint16_t address = 0x2000; address < 0x8000; address += 512) { // Final 3 Banks ($A000-$FFFF)
        dumpPRG(prgFile, base, address, prgFilePath);
      }
      break;
    }

    case 10: { // 128K/256K
      for (uint16_t i = 0; i < (((static_cast<uint16_t>(config.prgSizeIndex) - 3) * 8) + 7); i++) {
        write_prg_byte(0xA000, i); // $8000-$BFFF
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // Switch Bank ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      for (uint16_t address = 0x4000; address < 0x8000; address += 512) { // Final Bank ($C000-$FFFF)
        dumpPRG(prgFile, base, address, prgFilePath);
      }
      break;
    }

    case 16:
    case 159: { // 128K/256K
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      for (uint16_t i = 0; i < banks; i++) {
        write_prg_byte(0x6008, i); // Submapper 4
        write_prg_byte(0x8008, i); // Submapper 5
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 18: { // 128K/256K
      uint16_t banks = int_pow(2, config.prgSizeIndex) * 2;
      for (uint16_t i = 0; i < banks; i += 2) {
        write_prg_byte(0x8000, i & 0xF);
        write_prg_byte(0x8001, (i >> 4) & 0xF);
        write_prg_byte(0x8002, (i + 1) & 0xF);
        write_prg_byte(0x8003, ((i + 1) >> 4) & 0xF);
        for (uint16_t address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 19: { // 128K/256K
      for (uint8_t j = 0; j < 64; j++) { // Init Register
        write_ram_byte(0xE000, 0); // PRG Bank 0 ($8000-$9FFF)
      }
      uint16_t banks = int_pow(2, config.prgSizeIndex) * 2;
      for (uint16_t i = 0; i < banks; i++) {
        write_ram_byte(0xE000, i); // PRG Bank 0 ($8000-$9FFF)
        for (uint16_t address = 0x0; address < 0x2000; address += 512) {
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 21:
    case 22:
    case 23:
    case 25:
    case 65:
    case 75: { // 128K/256K
      uint16_t banks = int_pow(2, config.prgSizeIndex) * 2;
      for (uint16_t i = 0; i < banks; i += 2) {
        write_prg_byte(0x8000, i);
        write_prg_byte(0xA000, i + 1);
        for (uint16_t address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 24:
    case 26: // 256K
    case 78: { // 128K
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      for (uint16_t i = 0; i < banks; i++) { // 128K
        write_prg_byte(0x8000, i);
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 30: { // 256K/512K
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      for (uint16_t i = 0; i < banks; i++) { // 256K/512K
        if (config.flashFound())
          write_prg_byte(0xC000, i); // Flashable
        else
          write_prg_byte(0x8000, i); // Non-Flashable
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 32: { // 128K/256K
      uint16_t banks = int_pow(2, config.prgSizeIndex) * 2;
      for (uint16_t i = 0; i < banks; i++) { // 128K/256K
        write_prg_byte(0x9000, 1); // PRG Mode 0 - Read $A000-$BFFF to avoid difference between Modes 0 and 1
        write_prg_byte(0xA000, i); // PRG Bank
        for (uint16_t address = 0x2000; address < 0x4000; address += 512) { // 8K Banks ($A000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 33:
    case 48: { // 128K/256K
      uint16_t banks = int_pow(2, config.prgSizeIndex) * 2;
      for (uint16_t i = 0; i < banks; i += 2) {
        write_prg_byte(0x8000, i); // PRG Bank 0 ($8000-$9FFF)
        write_prg_byte(0x8001, i + 1); // PRG Bank 1 ($A000-$BFFF)
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // 8K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 37: {
      uint16_t banks = ((int_pow(2, config.prgSizeIndex) * 2)) - 2;  // Set Number of Banks
      write_prg_byte(0xA001, 0x80); // Block Register - PRG RAM Chip Enable, Writable
      for (uint16_t i = 0; i < banks; i += 2) { // 256K
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
        for (uint16_t address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      for (uint16_t address = 0x4000; address < 0x8000; address += 512) { // Final 2 Banks ($C000-$FFFF)
        dumpPRG(prgFile, base, address, prgFilePath);
      }
      break;
    }

    case 66: { // 64K/128K
      uint16_t banks = int_pow(2, config.prgSizeIndex) / 2;
      for (uint16_t i = 0; i < banks; i++) { // 64K/128K
        write_prg_byte(0x8000, i << 4); // bits 4-5
        for (uint16_t address = 0x0; address < 0x8000; address += 512) { // 32K Banks ($8000-$FFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 67: { // 128K
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      for (uint16_t i = 0; i < banks; i++) { // 128K
        write_reg_byte(0xF800, i); // [WRITE RAM SAFE]
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 68:
    case 73: { // 128K
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      for (uint16_t i = 0; i < banks; i++) { // 128K
        write_reg_byte(0xF000, i); // [WRITE RAM SAFE]
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 69: { // 128K/256K
      uint16_t banks = int_pow(2, config.prgSizeIndex) * 2;
      write_prg_byte(0x8000, 8); // Command Register - PRG Bank 0
      write_prg_byte(0xA000, 0); // Parameter Register - PRG RAM Disabled, PRG ROM, Bank 0 to $6000-$7FFF
      for (uint16_t i = 0; i < banks; i++) { // 128K/256K
        write_prg_byte(0x8000, 9); // Command Register - PRG Bank 1
        write_prg_byte(0xA000, i); // Parameter Register - ($8000-$9FFF)
        for (uint16_t address = 0x0000; address < 0x2000; address += 512) { // 8K Banks ($8000-$9FFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 70:
    case 89:
    case 93: // 128K
    case 152: { // 64K/128K
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      for (uint16_t i = 0; i < banks; i++) { // 128K
        write_prg_byte(0x8000, i << 4);
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 71: { // 64K/128K/256K
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      for (uint16_t i = 0; i < banks; i++) {
        write_prg_byte(0xC000, i);
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 72: { // 128K
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      write_prg_byte(0x8000, 0); // Reset Register
      for (uint16_t i = 0; i < banks; i++) { // 128K
        write_prg_byte(0x8000, i | 0x80); // PRG Command + Bank
        write_prg_byte(0x8000, i); // PRG Bank
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 76:
    case 88:
    case 95:
    case 154: // 128K
    case 206: { // 32K/64K/128K
      uint16_t banks = int_pow(2, config.prgSizeIndex) * 2;
      for (uint16_t i = 0; i < banks; i += 2) {
        write_prg_byte(0x8000, 6); // PRG ROM Command ($8000-$9FFF)
        write_prg_byte(0x8001, i); // PRG Bank
        write_prg_byte(0x8000, 7); // PRG ROM Command ($A000-$BFFF)
        write_prg_byte(0x8001, i + 1); // PRG Bank
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // 8K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 80: // 128K
    case 207: { // 256K [CART SOMETIMES NEEDS POWERCYCLE]
      uint16_t banks = int_pow(2, config.prgSizeIndex) * 2;
      for (uint16_t i = 0; i < banks; i += 2) {
        write_prg_byte(0x7EFA, i); // PRG Bank 0 ($8000-$9FFF)
        write_prg_byte(0x7EFC, i + 1); // PRG Bank 1 ($A000-$BFFF)
        for (uint16_t address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 82: { // 128K
      uint16_t banks = int_pow(2, config.prgSizeIndex) * 2;
      for (uint16_t i = 0; i < banks; i += 2) {
        write_prg_byte(0x7EFA, i << 2); // PRG Bank 0 ($8000-$9FFF)
        write_prg_byte(0x7EFB, (i + 1) << 2); // PRG Bank 1 ($A000-$BFFF)
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // 8K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 85: { // 128K/512K
      uint16_t banks = int_pow(2, config.prgSizeIndex) * 2;
      for (uint16_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000, i); // PRG Bank 0 ($8000-$9FFF)
        for (uint16_t address = 0x0; address < 0x2000; address += 512) { // 8K Banks ($8000-$9FFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 86:
    case 140: { // 128K
      uint16_t banks = int_pow(2, config.prgSizeIndex) / 2;
      for (uint16_t i = 0; i < banks; i++) { // 128K
        write_prg_byte(0x6000, i << 4); // bits 4-5
        for (uint16_t address = 0x0; address < 0x8000; address += 512) { // 32K Banks ($8000-$FFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 92: { // 256K
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      write_prg_byte(0x8000, 0); // Reset Register
      for (uint16_t i = 0; i < banks; i++) { // 256K
        write_prg_byte(0x8000, i | 0x80); // PRG Command + Bank
        write_prg_byte(0x8000, i); // PRG Bank
        for (uint16_t address = 0x4000; address < 0x8000; address += 512) { // 16K Banks ($C000-$FFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 94: {
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      for (uint16_t i = 0; i < banks; i++) { // 128K
        write_prg_byte(0x8000, i << 2);
        for (uint16_t address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 97: { // 256K
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      for (uint16_t i = 0; i < banks; i++) { // 256K
        write_prg_byte(0x8000, i); // PRG Bank
        for (uint16_t address = 0x4000; address < 0x8000; address += 512) { // 16K Banks ($C000-$FFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 105: { // 256K
      write_mmc1_byte(0xA000, 0x00); // Clear PRG Init/IRQ (Bit 4)
      write_mmc1_byte(0xA000, 0x10); // Set PRG Init/IRQ (Bit 4) to enable bank swapping
      for (uint8_t i = 0; i < 4; i++) { // PRG CHIP 1 128K
        write_mmc1_byte(0xA000, i << 1);
        for (uint16_t address = 0x0; address < 0x8000; address += 512) { // 32K Banks ($8000-$FFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      write_mmc1_byte(0x8000, 0x0C); // Switch 16K Bank ($8000-$BFFF) + Fixed Last Bank ($C000-$FFFF)
      write_mmc1_byte(0xA000, 0x08); // Select PRG CHIP 2 (Bit 3)
      for (uint8_t j = 0; j < 8; j++) { // PRG CHIP 2 128K
        write_mmc1_byte(0xE000, j);
        for (uint16_t address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 180: { // 128K
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      for (uint16_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000, i);
        for (uint16_t address = 0x4000; address < 0x8000; address += 512) { // 16K Banks ($C000-$FFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 153: { // 512K
      uint16_t banks = int_pow(2, config.prgSizeIndex);
      for (uint16_t i = 0; i < banks; i++) { // 512K
        write_prg_byte(0x8000, i >> 4); // PRG Outer Bank (Documentation says duplicate over $8000-$8003 registers)
        write_prg_byte(0x8001, i >> 4); // PRG Outer Bank
        write_prg_byte(0x8002, i >> 4); // PRG Outer Bank
        write_prg_byte(0x8003, i >> 4); // PRG Outer Bank
        write_prg_byte(0x8008, i & 0xF); // PRG Inner Bank
        for (uint16_t address = 0x0000; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }

    case 210: { // 128K/256K
      uint16_t banks = int_pow(2, config.prgSizeIndex) * 2;
      for (uint16_t i = 0; i < banks; i += 2) {
        write_prg_byte(0xE000, i); // PRG Bank 0 ($8000-$9FFF) [WRITE NO RAM]
        write_prg_byte(0xE800, i + 1); // PRG Bank 1 ($A000-$BFFF) [WRITE NO RAM]
        for (uint16_t address = 0x0; address < 0x4000; address += 512) {
          dumpPRG(prgFile, base, address, prgFilePath);
        }
      }
      break;
    }
  }
  prgFile.close();

  ui->printlnMsg(F("PRG FILE DUMPED!"));
  ui->printlnMsg(F(""));
  ui->flushOutput();

  calcCRC(prgFilePath.c_str());

  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
  ledBlueOff();
}

void readCHR(const String &outputFolderPath, const CartridgeConfig &config) {
  ui->clearOutput();
  ui->flushOutput();

  ledGreenOn();
  set_address(0);
  _delay_us(1);
  if (config.chrSizeIndex == 0) {
    ui->printlnMsg(F("CHR SIZE 0K"));
    ui->flushOutput();
  }
  else {
    String chrFilePath = pathJoin(outputFolderPath, F("CHR.bin"));
    File chrFile = openForCreating(sd, chrFilePath);
    switch (config.mapper) {
      case 0: { // 8K
        for (uint16_t address = 0x0; address < 0x2000; address += 512) {
          dumpCHR(chrFile, address, chrFilePath);
        }
        break;
      }

      case 1:
      case 155: {
        uint16_t banks = int_pow(2, config.chrSizeIndex);
        for (uint16_t i = 0; i < banks; i += 2) { // 8K/16K/32K/64K/128K (Bank #s are based on 4K Banks)
          write_prg_byte(0x8000, 0x80); // Clear Register
          write_mmc1_byte(0xA000, i);
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 3: // 8K/16K/32K
      case 66: // 16K/32K
      case 70:
      case 152: { // 128K
        uint16_t banks = int_pow(2, config.chrSizeIndex) / 2;
        for (uint16_t i = 0; i < banks; i++) { // 8K Banks
          write_prg_byte(0x8000, i); // CHR Bank 0
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 4:
      case 47:
      case 118:
      case 119: {
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        if (config.mapper == 47)
          write_prg_byte(0xA001, 0x80); // Block Register - PRG RAM Chip Enable, Writable
        for (uint16_t i = 0; i < banks; i += 4) { // 8K/16K/32K/64K/128K/256K
          if (config.mapper == 47) {
            if (i == 0)
              write_prg_byte(0x6000, 0); // Switch to Lower Block
            else if (i == 128)
              write_prg_byte(0x6000, 1); // Switch to Upper Block
          }
          write_prg_byte(0x8000, 0); // CHR Bank 0 ($0000-$07FF)
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 1); // CHR Bank 1 ($0800-$0FFF)
          write_prg_byte(0x8001, i + 2);
          for (uint16_t address = 0x0; address < 0x1000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 5: { // 128K/256K/512K
        uint16_t banks = int_pow(2, config.chrSizeIndex) / 2;
        write_prg_byte(0x5101, 0); // 8K CHR Banks
        for (uint16_t i = 0; i < banks; i++) {
          if (i == 0)
            write_prg_byte(0x5130, 0); // Set Upper 2 bits
          else if (i == 8)
            write_prg_byte(0x5130, 1); // Set Upper 2 bits
          else if (i == 16)
            write_prg_byte(0x5130, 2); // Set Upper 2 bits
          else if (i == 24)
            write_prg_byte(0x5130, 3); // Set Upper 2 bits
          write_prg_byte(0x5127, i);
          for (uint16_t address = 0x0; address < 0x2000; address += 512) { // ($0000-$1FFF)
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 9:
      case 10: { // Mapper 9: 128K, Mapper 10: 64K/128K
        uint16_t banks;
        if (config.mapper == 9)
          banks = 32;
        else // Mapper 10
          banks = int_pow(2, config.chrSizeIndex);
        for (uint16_t i = 0; i < banks; i++) { // 64K/128K
          write_prg_byte(0xB000, i);
          write_prg_byte(0xC000, i);
          for (uint16_t address = 0x0; address < 0x1000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 16:
      case 159: { // 128K/256K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        for (uint16_t i = 0; i < banks; i++) {
          write_prg_byte(0x6000, i); // Submapper 4
          write_prg_byte(0x8000, i); // Submapper 5
          for (uint16_t address = 0x0; address < 0x400; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 18: { // 128K/256K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        for (uint16_t i = 0; i < banks; i++) {
          write_prg_byte(0xA000, i & 0xF); // CHR Bank Lower 4 bits
          write_prg_byte(0xA001, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits
          for (uint16_t address = 0x0; address < 0x400; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 19: { // 128K/256K
        for (uint8_t j = 0; j < 64; j++) { // Init Register
          write_ram_byte(0xE800, 0xC0); // CHR RAM High/Low Disable (ROM Enable)
        }
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        write_ram_byte(0xE800, 0xC0); // CHR RAM High/Low Disable (ROM Enable)
        for (uint16_t i = 0; i < banks; i += 8) {
          write_prg_byte(0x8000, i); // CHR Bank 0
          write_prg_byte(0x8800, i + 1); // CHR Bank 1
          write_prg_byte(0x9000, i + 2); // CHR Bank 2
          write_prg_byte(0x9800, i + 3); // CHR Bank 3
          write_prg_byte(0xA000, i + 4); // CHR Bank 4
          write_prg_byte(0xA800, i + 5); // CHR Bank 5
          write_prg_byte(0xB000, i + 6); // CHR Bank 6
          write_prg_byte(0xB800, i + 7); // CHR Bank 7
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 21: { // 128K/256K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        for (uint16_t i = 0; i < banks; i++) {
          write_prg_byte(0xB000, i & 0xF); // CHR Bank Lower 4 bits
          if (banks == 128)
            write_prg_byte(0xB002, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4a (Wai Wai World 2)
          else  // banks == 256
            write_prg_byte(0xB040, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4c (Ganbare Goemon Gaiden 2)
          for (uint16_t address = 0x0; address < 0x400; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 22: { // 128K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        for (uint16_t i = 0; i < banks; i++) {
          write_prg_byte(0xB000, (i << 1) & 0xF); // CHR Bank Lower 4 bits
          write_prg_byte(0xB002, (i >> 3) & 0xF);  // CHR Bank Upper 4 bits
          for (uint16_t address = 0x0; address < 0x400; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 23: { // 128K
        // Detect VRC4e Carts - read PRG 0x1FFF6 (DATE)
        // Boku Dracula-kun = 890810, Tiny Toon = 910809
        // Crisis Force = 910701, Parodius Da! = 900916
        write_prg_byte(0x8000, 15);
        byte prgchk0 = read_prg_byte(0x9FF6);
        bool vrc4e = false;
        if (prgchk0 == 0x30) { // Check for "0" in middle of date
          vrc4e = true; // VRC4e Cart
        }
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        for (uint16_t i = 0; i < banks; i++) {
          write_prg_byte(0xB000, i & 0xF); // CHR Bank Lower 4 bits
          if (vrc4e == true)
            write_prg_byte(0xB004, (i >> 4) & 0xF); // CHR Bank Upper 4 bits VRC4e
          else
            write_prg_byte(0xB001, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC2b/VRC4f
          for (uint16_t address = 0x0; address < 0x400; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 24: { // 128K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        write_prg_byte(0xB003, 0); // PPU Banking Mode 0
        for (uint16_t i = 0; i < banks; i += 8) {
          write_prg_byte(0xD000, i); // CHR Bank 0
          write_prg_byte(0xD001, i + 1); // CHR Bank 1
          write_prg_byte(0xD002, i + 2); // CHR Bank 2
          write_prg_byte(0xD003, i + 3); // CHR Bank 3
          write_prg_byte(0xE000, i + 4); // CHR Bank 4 [WRITE NO RAM]
          write_prg_byte(0xE001, i + 5); // CHR Bank 5 [WRITE NO RAM]
          write_prg_byte(0xE002, i + 6); // CHR Bank 6 [WRITE NO RAM]
          write_prg_byte(0xE003, i + 7); // CHR Bank 7 [WRITE NO RAM]
          for (uint16_t address = 0x0; address < 0x2000; address += 512) { // 1K Banks
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 25: { // 128K/256K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        for (uint16_t i = 0; i < banks; i++) {
          write_prg_byte(0xB000, i & 0xF); // CHR Bank Lower 4 bits
          if ((config.ramSizeIndex > 0) || (banks == 128)) // VRC2c (Ganbare Goemon Gaiden)/VRC4b (Bio Miracle/Gradius 2/Racer Mini)
            write_prg_byte(0xB002, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC2c/VRC4b
          else
            write_prg_byte(0xB008, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4d (Teenage Mutant Ninja Turtles)
          for (uint16_t address = 0x0; address < 0x400; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 26: { // 128K/256K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        write_prg_byte(0xB003, 0); // PPU Banking Mode 0
        for (uint16_t i = 0; i < banks; i += 8) {
          write_prg_byte(0xD000, i); // CHR Bank 0
          write_prg_byte(0xD002, i + 1); // CHR Bank 1
          write_prg_byte(0xD001, i + 2); // CHR Bank 2
          write_prg_byte(0xD003, i + 3); // CHR Bank 3
          write_reg_byte(0xE000, i + 4); // CHR Bank 4 [WRITE RAM SAFE]
          write_reg_byte(0xE002, i + 5); // CHR Bank 5 [WRITE RAM SAFE]
          write_reg_byte(0xE001, i + 6); // CHR Bank 6 [WRITE RAM SAFE]
          write_reg_byte(0xE003, i + 7); // CHR Bank 7 [WRITE RAM SAFE]
          for (uint16_t address = 0x0; address < 0x2000; address += 512) { // 1K Banks
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 32: // 128K
      case 65: { // 128K/256K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        for (uint16_t i = 0; i < banks; i += 8) {
          write_prg_byte(0xB000, i); // CHR Bank 0
          write_prg_byte(0xB001, i + 1); // CHR Bank 1
          write_prg_byte(0xB002, i + 2); // CHR Bank 2
          write_prg_byte(0xB003, i + 3); // CHR Bank 3
          write_prg_byte(0xB004, i + 4); // CHR Bank 4
          write_prg_byte(0xB005, i + 5); // CHR Bank 5
          write_prg_byte(0xB006, i + 6); // CHR Bank 6
          write_prg_byte(0xB007, i + 7); // CHR Bank 7
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 33: // 128K/256K
      case 48: { // 256K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 2;
        for (uint16_t i = 0; i < banks; i += 2) { // 2K Banks
          write_prg_byte(0x8002, i); // CHR Bank 0
          write_prg_byte(0x8003, i + 1); // CHR Bank 1
          for (uint16_t address = 0x0; address < 0x1000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 37: {
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        write_prg_byte(0xA001, 0x80); // Block Register - PRG RAM Chip Enable, Writable
        for (uint16_t i = 0; i < banks; i += 4) { // 256K
          if (i == 0)
            write_prg_byte(0x6000, 0); // Switch to Lower Block ($00000-$1FFFF)
          else if (i == 128)
            write_prg_byte(0x6000, 4); // Switch to Upper Block ($20000-$3FFFF)
          write_prg_byte(0x8000, 0); // CHR Bank 0 ($0000-$07FF)
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 1); // CHR Bank 1 ($0800-$0FFF)
          write_prg_byte(0x8001, i + 2);
          for (uint16_t address = 0x0; address < 0x1000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 67: { // 128K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 2;
        for (uint16_t i = 0; i < banks; i += 4) { // 2K Banks
          write_prg_byte(0x8800, i); // CHR Bank 0
          write_prg_byte(0x9800, i + 1); // CHR Bank 1
          write_prg_byte(0xA800, i + 2); // CHR Bank 2
          write_prg_byte(0xB800, i + 3); // CHR Bank 3
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 68: { // 128K/256K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 2;
        for (uint16_t i = 0; i < banks; i += 4) { // 2K Banks
          write_prg_byte(0x8000, i); // CHR Bank 0
          write_prg_byte(0x9000, i + 1); // CHR Bank 1
          write_prg_byte(0xA000, i + 2); // CHR Bank 2
          write_prg_byte(0xB000, i + 3); // CHR Bank 3
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 69: { // 128K/256K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        for (uint16_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000, 0); // Command Register - CHR Bank 0
          write_prg_byte(0xA000, i); // Parameter Register - ($0000-$03FF)
          for (uint16_t address = 0x0; address < 0x400; address += 512) { // 1K Banks
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 72: { // 128K
        uint16_t banks = int_pow(2, config.chrSizeIndex) / 2;
        write_prg_byte(0x8000, 0); // Reset Register
        for (uint16_t i = 0; i < banks; i++) { // 8K Banks
          write_prg_byte(0x8000, i | 0x40); // CHR Command + Bank
          write_prg_byte(0x8000, i); // CHR Bank
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 75: { // 128K
        uint16_t banks = int_pow(2, config.chrSizeIndex);
        for (uint16_t i = 0; i < banks; i++) { // 4K Banks
          write_reg_byte(0xE000, i); // CHR Bank Low Bits [WRITE RAM SAFE]
          write_prg_byte(0x9000, (i & 0x10) >> 3); // High Bit
          for (uint16_t address = 0x0; address < 0x1000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 76: { // 128K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 2;
        for (uint16_t i = 0; i < banks; i += 2) { // 2K Banks
          write_prg_byte(0x8000, 2); // CHR Command ($0000-$07FF) 2K Bank
          write_prg_byte(0x8001, i); // CHR Bank
          write_prg_byte(0x8000, 3); // CHR Command ($0800-$0FFF) 2K Bank
          write_prg_byte(0x8001, i + 1); // CHR Bank
          for (uint16_t address = 0x0000; address < 0x1000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 77: { // 32K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 2;
        for (uint16_t i = 0; i < banks; i++) { // 2K Banks
          write_prg_byte(0x8000, i << 4); // CHR Bank 0
          for (uint16_t address = 0x0; address < 0x800; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 78: { // 128K
        uint16_t banks = int_pow(2, config.chrSizeIndex) / 2;
        for (uint16_t i = 0; i < banks; i++) { // 8K Banks
          write_prg_byte(0x8000, i << 4); // CHR Bank 0
          for (uint16_t address = 0x0; address < 0x2000; address += 512) { // 8K Banks ($0000-$1FFF)
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 80: // 128K/256K
      case 82: // 128K/256K
      case 207: { // 128K [CART SOMETIMES NEEDS POWERCYCLE]
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        for (uint16_t i = 0; i < banks; i += 4) {
          write_prg_byte(0x7EF2, i);  // CHR Bank 2 [REGISTERS 0x7EF0/0x7EF1 WON'T WORK]
          write_prg_byte(0x7EF3, i + 1);  // CHR Bank 3
          write_prg_byte(0x7EF4, i + 2);  // CHR Bank 4
          write_prg_byte(0x7EF5, i + 3);  // CHR Bank 5
          for (uint16_t address = 0x1000; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 85: { // 128K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        for (uint16_t i = 0; i < banks; i += 8) {
          write_prg_byte(0xA000, i); // CHR Bank 0
          write_prg_byte(0xA008, i + 1); // CHR Bank 1
          write_prg_byte(0xB000, i + 2); // CHR Bank 2
          write_prg_byte(0xB008, i + 3); // CHR Bank 3
          write_prg_byte(0xC000, i + 4); // CHR Bank 4
          write_prg_byte(0xC008, i + 5); // CHR Bank 5
          write_prg_byte(0xD000, i + 6); // CHR Bank 6
          write_prg_byte(0xD008, i + 7); // CHR Bank 7
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 86: { // 64K
        uint16_t banks = int_pow(2, config.chrSizeIndex) / 2;
        for (uint16_t i = 0; i < banks; i++) { // 8K Banks
          if (i < 4)
            write_prg_byte(0x6000, i & 0x3);
          else
            write_prg_byte(0x6000, (i | 0x40) & 0x43);
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 87: { // 16K/32K
        uint16_t banks = int_pow(2, config.chrSizeIndex) / 2;
        for (uint16_t i = 0; i < banks; i++) { // 16K/32K
          write_prg_byte(0x6000, (((i & 0x1) << 1) | ((i & 0x2) >> 1)));
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 88: // 128K
      case 95: // 32K
      case 154: // 128K
      case 206: { // 16K/32K/64K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        for (uint16_t i = 0; i < banks; i += 2) { // 1K Banks
          if (i < 64) {
            write_prg_byte(0x8000, 0); // CHR Command ($0000-$07FF) 2K Bank
            write_prg_byte(0x8001, i & 0x3F); // CHR Bank
            for (uint16_t address = 0x0; address < 0x800; address += 512) {
              dumpCHR(chrFile, address, chrFilePath);
            }
          }
          else {
            write_prg_byte(0x8000, 2); // CHR Command ($1000-$13FF) 1K Bank
            write_prg_byte(0x8001, i); // CHR Bank
            write_prg_byte(0x8000, 3); // CHR Command ($1400-$17FF) 1K Bank
            write_prg_byte(0x8001, i + 1); // CHR Bank
            for (uint16_t address = 0x1000; address < 0x1800; address += 512) {
              dumpCHR(chrFile, address, chrFilePath);
            }
          }
        }
        break;
      }

      case 89: { // 128K
        uint16_t banks = int_pow(2, config.chrSizeIndex) / 2;
        for (uint16_t i = 0; i < banks; i++) { // 8K Banks
          if (i < 8)
            write_prg_byte(0x8000, i & 0x7);
          else
            write_prg_byte(0x8000, (i | 0x80) & 0x87);
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 92: { // 128K
        uint16_t banks = int_pow(2, config.chrSizeIndex) / 2;
        write_prg_byte(0x8000, 0); // Reset Register
        for (uint16_t i = 0; i < banks; i++) { // 8K Banks
          write_prg_byte(0x8000, i | 0x40); // CHR Command + Bank
          write_prg_byte(0x8000, i); // CHR Bank
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 140: { // 32K/128K
        uint16_t banks = int_pow(2, config.chrSizeIndex) / 2;
        for (uint16_t i = 0; i < banks; i++) { // 8K Banks
          write_prg_byte(0x6000, i);
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 184: { // 16K/32K
        uint16_t banks = int_pow(2, config.chrSizeIndex);
        for (uint16_t i = 0; i < banks; i++) { // 4K Banks
          write_prg_byte(0x6000, i); // CHR LOW (Bits 0-2) ($0000-$0FFF)
          for (uint16_t address = 0x0; address < 0x1000; address += 512) { // 4K Banks ($0000-$0FFF)
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }

      case 185: { // 8K [READ 32K TO OVERRIDE LOCKOUT]
        for (uint8_t i = 0; i < 4; i++) { // Read 32K to locate valid 8K
          write_prg_byte(0x8000, i);
          byte chrcheck = read_chr_byte(0);
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            for (uint16_t x = 0; x < 512; x++) {
              sdBuffer[x] = read_chr_byte(address + x);
            }
            if (chrcheck != 0xFF) {
              writeFile(chrFile, sdBuffer, 512, chrFilePath);
            }
          }
        }
        break;
      }

      case 210: { // 128K/256K
        uint16_t banks = int_pow(2, config.chrSizeIndex) * 4;
        write_prg_byte(0xE800, 0xC0); // CHR RAM DISABLE (Bit 6 and 7) [WRITE NO RAM]
        for (uint16_t i = 0; i < banks; i += 8) {
          write_prg_byte(0x8000, i); // CHR Bank 0
          write_prg_byte(0x8800, i + 1); // CHR Bank 1
          write_prg_byte(0x9000, i + 2); // CHR Bank 2
          write_prg_byte(0x9800, i + 3); // CHR Bank 3
          write_prg_byte(0xA000, i + 4); // CHR Bank 4
          write_prg_byte(0xA800, i + 5); // CHR Bank 5
          write_prg_byte(0xB000, i + 6); // CHR Bank 6
          write_prg_byte(0xB800, i + 7); // CHR Bank 7
          for (uint16_t address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(chrFile, address, chrFilePath);
          }
        }
        break;
      }
    }
    closeFile(chrFile, chrFilePath);

    ui->printlnMsg(F("CHR FILE DUMPED!"));
    ui->printlnMsg(F(""));
    ui->flushOutput();

    calcCRC(chrFilePath.c_str());
  }
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
  ledGreenOff();
}

/******************************************
   [RAM Functions]
 *****************************************/
void readRAM(const String &outputFolderPath, const CartridgeConfig &config) {
  ui->clearOutput();
  ui->flushOutput();

  ledBlueOn();
  ledGreenOn();
  set_address(0);
  _delay_us(1);
  if (config.ramSizeIndex == 0) {
    ui->printlnMsg(F("RAM SIZE 0K"));
    ui->flushOutput();
  }
  else {
    String ramFilePath = pathJoin(outputFolderPath, F("RAM.bin"));
    File ramFile = openForCreating(sd, ramFilePath);
    uint16_t base = 0x6000;
    switch (config.mapper) {
      case 0: { // 2K/4K
        for (uint16_t address = 0x0; address < (0x800 * config.ramSizeIndex); address += 512) { // 2K/4K
          dumpPRG(ramFile, base, address, ramFilePath); // SWITCH MUST BE IN OFF POSITION
        }
        break;
      }

      case 1:
      case 155: { // 8K/16K/32K
        uint16_t banks = int_pow(2, config.ramSizeIndex) / 2; // banks = 1,2,4
        for (uint16_t i = 0; i < banks; i++) { // 8K Banks ($6000-$7FFF)
          write_prg_byte(0x8000, 0x80); // Clear Register
          write_mmc1_byte(0x8000, 1 << 3);
          write_mmc1_byte(0xE000, 0);
          if (banks == 4) // 32K
            write_mmc1_byte(0xA000, i << 2);
          else
            write_mmc1_byte(0xA000, i << 3);
          for (uint16_t address = 0x0; address < 0x2000; address += 512) { // 8K
            dumpPRG(ramFile, base, address, ramFilePath);
          }
        }
        break;
      }

      case 4: { // 1K/8K (MMC6/MMC3)
        if (config.mmc6) { // MMC6 1K
          write_prg_byte(0x8000, 0x20); // PRG RAM ENABLE
          write_prg_byte(0xA001, 0x20); // PRG RAM PROTECT - Enable reading RAM at $7000-$71FF
          for (uint16_t address = 0x1000; address < 0x1200; address += 512) { // 512B
            dumpMMC5RAM(ramFile, base, address, ramFilePath);
          }
          write_prg_byte(0x8000, 0x20); // PRG RAM ENABLE
          write_prg_byte(0xA001, 0x80); // PRG RAM PROTECT - Enable reading RAM at $7200-$73FF
          for (uint16_t address = 0x1200; address < 0x1400; address += 512) { // 512B
            dumpMMC5RAM(ramFile, base, address, ramFilePath);
          }
          write_prg_byte(0x8000, 6); // PRG RAM DISABLE
        }
        else { // MMC3 8K
          write_prg_byte(0xA001, 0xC0); // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
          for (uint16_t address = 0; address < 0x2000; address += 512) { // 8K
            dumpPRG(ramFile, base, address, ramFilePath);
          }
        }
        break;
      }

      case 5: { // 8K/16K/32K
        write_prg_byte(0x5100, 3); // 8K PRG Banks
        uint16_t banks = int_pow(2, config.ramSizeIndex) / 2; // banks = 1,2,4
        if (banks == 2) { // 16K - Split SRAM Chips 8K/8K
          for (uint16_t i = 0; i < (banks / 2); i++) { // Chip 1
            write_prg_byte(0x5113, i);
            for (uint16_t address = 0; address < 0x2000; address += 512) { // 8K
              dumpMMC5RAM(ramFile, base, address, ramFilePath);
            }
          }
          for (uint16_t j = 4; j < (banks / 2) + 4; j++) { // Chip 2
            write_prg_byte(0x5113, j);
            for (uint16_t address = 0; address < 0x2000; address += 512) { // 8K
              dumpMMC5RAM(ramFile, base, address, ramFilePath);
            }
          }
        }
        else { // 8K/32K Single SRAM Chip
          for (uint16_t i = 0; i < banks; i++) { // banks = 1 or 4
            write_prg_byte(0x5113, i);
            for (uint16_t address = 0; address < 0x2000; address += 512) { // 8K
              dumpMMC5RAM(ramFile, base, address, ramFilePath);
            }
          }
        }
        break;
      }

      case 16: // 256-byte EEPROM 24C02
      case 159: { // 128-byte EEPROM 24C01 [Little Endian]
        uint16_t eepsize;
        if (config.mapper == 159)
          eepsize = 128;
        else
          eepsize = 256;
        for (uint16_t address = 0; address < eepsize; address++) {
          EepromREAD(address, config);
        }
        writeFile(ramFile, sdBuffer, eepsize, ramFilePath);
        //          display_Clear(); // TEST PURPOSES - DISPLAY EEPROM DATA
        break;
      }

      case 19: {
        if (config.ramSizeIndex == 2) { // PRG RAM 128B
          for (uint8_t x = 0; x < 128; x++) {
            write_ram_byte(0xF800, x); // PRG RAM ENABLE
            sdBuffer[x] = read_prg_byte(0x4800); // DATA PORT
          }
          writeFile(ramFile, sdBuffer, 128, ramFilePath);
        }
        else { // SRAM 8K
          for (uint8_t i = 0; i < 64; i++) { // Init Register
            write_ram_byte(0xE000, 0);
          }
          for (uint16_t address = 0; address < 0x2000; address += 512) { // 8K
            dumpPRG(ramFile, base, address, ramFilePath);
          }
        }
        break;
      }

      case 80: { // 1K
        write_prg_byte(0x7EF8, 0xA3); // PRG RAM ENABLE 0
        write_prg_byte(0x7EF9, 0xA3); // PRG RAM ENABLE 1
        for (uint16_t x = 0; x < 128; x++) { // PRG RAM 1K ($7F00-$7FFF) MIRRORED ONCE
          sdBuffer[x] = read_prg_byte(0x7F00 + x);
        }
        writeFile(ramFile, sdBuffer, 128, ramFilePath);
        write_prg_byte(0x7EF8, 0xFF); // PRG RAM DISABLE 0
        write_prg_byte(0x7EF9, 0xFF); // PRG RAM DISABLE 1
        break;
      }

      case 82: { // 5K
        write_prg_byte(0x7EF7, 0xCA); // PRG RAM ENABLE 0 ($6000-$67FF)
        write_prg_byte(0x7EF8, 0x69); // PRG RAM ENABLE 1 ($6800-$6FFF)
        write_prg_byte(0x7EF9, 0x84); // PRG RAM ENABLE 2 ($7000-$73FF)
        for (uint16_t address = 0x0; address < 0x1400; address += 512) { // PRG RAM 5K ($6000-$73FF)
          dumpMMC5RAM(ramFile, base, address, ramFilePath);
        }
        write_prg_byte(0x7EF7, 0xFF); // PRG RAM DISABLE 0 ($6000-$67FF)
        write_prg_byte(0x7EF8, 0xFF); // PRG RAM DISABLE 1 ($6800-$6FFF)
        write_prg_byte(0x7EF9, 0xFF); // PRG RAM DISABLE 2 ($7000-$73FF)
        break;
      }

      default: {
        if (config.mapper == 118) // 8K
          write_prg_byte(0xA001, 0xC0); // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
        else if (config.mapper == 19) {
          for (uint8_t i = 0; i < 64; i++) { // Init Register
            write_ram_byte(0xE000, 0);
          }
        }
        else if ((config.mapper == 21) || (config.mapper == 25)) // 8K
          write_prg_byte(0x8000, 0);
        else if (config.mapper == 26) // 8K
          write_prg_byte(0xB003, 0x80); // PRG RAM ENABLE
        else if (config.mapper == 68) // 8K
          write_reg_byte(0xF000, 0x10); // PRG RAM ENABLE [WRITE RAM SAFE]
        else if (config.mapper == 69) { // 8K
          write_prg_byte(0x8000, 8); // Command Register - PRG Bank 0
          write_prg_byte(0xA000, 0xC0); // Parameter Register - PRG RAM Enabled, PRG RAM, Bank 0 to $6000-$7FFF
        }
        else if (config.mapper == 85) // 8K
          write_ram_byte(0xE000, 0x80); // PRG RAM ENABLE
        else if (config.mapper == 153) // 8K
          write_prg_byte(0x800D, 0x20); // PRG RAM Chip Enable
        for (uint16_t address = 0; address < 0x2000; address += 512) { // 8K
          dumpPRG(ramFile, base, address, ramFilePath);
        }
        if (config.mapper == 85) // 8K
          write_reg_byte(0xE000, 0); // PRG RAM DISABLE [WRITE RAM SAFE]
        break;
      }
    }
    closeFile(ramFile, ramFilePath);

    ui->printlnMsg(F("RAM FILE DUMPED!"));
    ui->printlnMsg(F(""));
    ui->flushOutput();

    if (config.mapper == 16 || config.mapper == 159)
    {
      calcCRC(ramFilePath.c_str());
    }
    else {
      calcCRC(ramFilePath.c_str());
    }
  }
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
  ledBlueOff();
  ledGreenOff();
}

void writeRAM(const CartridgeConfig &config) {
  ui->clearOutput();

  if (config.ramSizeIndex == 0) {
    ui->printError(F("RAM SIZE 0K"));
  }
  else {
    String ramFilePath = fileBrowser(F("Select RAM File"));
    uint16_t base = 0x6000;

    ui->clearOutput();
    ui->printlnMsg(F("Writing File: "));
    ui->printlnMsg(ramFilePath);
    ui->flushOutput();

    //open file on sd card
    File ramFile = openForReading(sd, ramFilePath);
    switch (config.mapper) {
      case 0: { // 2K/4K
        for (uint16_t address = 0x0; address < (0x800 * config.ramSizeIndex); address += 512) { // 2K/4K
          readFile(ramFile, sdBuffer, 512, ramFilePath);
          for (uint16_t x = 0; x < 512; x++) {
            write_prg_byte(base + address + x, sdBuffer[x]); // SWITCH MUST BE IN OFF POSITION
          }
        }
        break;
      }

      case 1:
      case 155: {
        uint16_t banks = int_pow(2, config.ramSizeIndex) / 2; // banks = 1,2,4
        for (uint16_t i = 0; i < banks; i++) { // 8K Banks ($6000-$7FFF)
          write_prg_byte(0x8000, 0x80); // Clear Register
          write_mmc1_byte(0x8000, 1 << 3); // PRG ROM MODE 32K
          write_mmc1_byte(0xE000, 0); // PRG RAM ENABLED
          if (banks == 4) // 32K
            write_mmc1_byte(0xA000, i << 2);
          else
            write_mmc1_byte(0xA000, i << 3);
          for (uint16_t address = 0x0; address < 0x2000; address += 512) { // 8K
            readFile(ramFile, sdBuffer, 512, ramFilePath);
            for (uint16_t x = 0; x < 512; x++) {
              write_prg_byte(base + address + x, sdBuffer[x]);
            }
          }
        }
        break;
      }

      case 4: { // 1K/8K (MMC6/MMC3)
        if (config.mmc6) { // MMC6 1K
          write_prg_byte(0x8000, 0x20); // PRG RAM ENABLE
          write_prg_byte(0xA001, 0x30); // PRG RAM PROTECT - Enable reading/writing to RAM at $7000-$71FF
          for (uint16_t address = 0x1000; address < 0x1200; address += 512) { // 512B
            readFile(ramFile, sdBuffer, 512, ramFilePath);
            for (uint16_t x = 0; x < 512; x++) {
              write_wram_byte(base + address + x, sdBuffer[x]);
            }
          }
          write_prg_byte(0x8000, 0x20); // PRG RAM ENABLE
          write_prg_byte(0xA001, 0xC0); // PRG RAM PROTECT - Enable reading/writing to RAM at $7200-$73FF
          for (uint16_t address = 0x1200; address < 0x1400; address += 512) { // 512B
            readFile(ramFile, sdBuffer, 512, ramFilePath);
            for (uint16_t x = 0; x < 512; x++) {
              write_wram_byte(base + address + x, sdBuffer[x]);
            }
          }
          write_prg_byte(0x8000, 0x6); // PRG RAM DISABLE
        }
        else { // MMC3 8K
          write_prg_byte(0xA001, 0x80); // PRG RAM CHIP ENABLE - Chip Enable, Allow Writes
          for (uint16_t address = 0; address < 0x2000; address += 512) { // 8K
            readFile(ramFile, sdBuffer, 512, ramFilePath);
            for (uint16_t x = 0; x < 512; x++) {
              write_prg_byte(base + address + x, sdBuffer[x]);
            }
          }
          write_prg_byte(0xA001, 0xC0); // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
        }
        break;
      }

      case 5: { // 8K/16K/32K
        write_prg_byte(0x5100, 3); // 8K PRG Banks
        uint16_t banks = int_pow(2, config.ramSizeIndex) / 2; // banks = 1,2,4
        if (banks == 2) { // 16K - Split SRAM Chips 8K/8K [ETROM = 16K (ONLY 1ST 8K BATTERY BACKED)]
          for (uint16_t i = 0; i < (banks / 2); i++) { // Chip 1
            write_prg_byte(0x5113, i);
            for (uint16_t address = 0; address < 0x2000; address += 512) { // 8K
              writeMMC5RAM(ramFile, base, address, ramFilePath);
            }
          }
          for (uint16_t j = 4; j < (banks / 2) + 4; j++) { // Chip 2
            write_prg_byte(0x5113, j);
            for (uint16_t address = 0; address < 0x2000; address += 512) { // 8K
              writeMMC5RAM(ramFile, base, address, ramFilePath);
            }
          }
        }
        else { // 8K/32K Single SRAM Chip [EKROM = 8K BATTERY BACKED, EWROM = 32K BATTERY BACKED]
          for (uint16_t i = 0; i < banks; i++) { // banks = 1 or 4
            write_prg_byte(0x5113, i);
            for (uint16_t address = 0; address < 0x2000; address += 512) { // 8K
              writeMMC5RAM(ramFile, base, address, ramFilePath);
            }
          }
        }
        break;
      }

      case 16: // 256-byte EEPROM 24C02
      case 159: { // 128-byte EEPROM 24C01 [Little Endian]
        uint16_t eepsize;
        if (config.mapper == 159)
          eepsize = 128;
        else
          eepsize = 256;
        readFile(ramFile, sdBuffer, eepsize, ramFilePath);
        for (uint16_t address = 0; address < eepsize; address++) {
          EepromWRITE(address, config);
          if ((address % 128) == 0) {
            ui->clearOutput();
          }
          ui->printMsg(F("."));
          ui->flushOutput();
        }
        break;
      }

      case 19: {
        if (config.ramSizeIndex == 2) { // PRG RAM 128B
          readFile(ramFile, sdBuffer, 128, ramFilePath);
          for (uint8_t x = 0; x < 128; x++) {
            write_ram_byte(0xF800, x); // PRG RAM ENABLE
            write_prg_byte(0x4800, sdBuffer[x]); // DATA PORT
          }
        }
        else { // SRAM 8K
          for (uint8_t i = 0; i < 64; i++) { // Init Register
            write_ram_byte(0xF800, 0x40); // PRG RAM WRITE ENABLE
          }
          write_ram_byte(0xF800, 0x40); // PRG RAM WRITE ENABLE
          for (uint16_t address = 0; address < 0x2000; address += 512) { // 8K
            readFile(ramFile, sdBuffer, 512, ramFilePath);
            for (uint16_t x = 0; x < 512; x++) {
              write_prg_byte(base + address + x, sdBuffer[x]);
            }
          }
          write_ram_byte(0xF800, 0x0F); // PRG RAM WRITE PROTECT
        }
        break;
      }

      case 80: { // 1K
        write_prg_byte(0x7EF8, 0xA3); // PRG RAM ENABLE 0
        write_prg_byte(0x7EF9, 0xA3); // PRG RAM ENABLE 1
        for (uint16_t address = 0x1F00; address < 0x2000; address += 512) { // PRG RAM 1K ($7F00-$7FFF)
          readFile(ramFile, sdBuffer, 128, ramFilePath);
          for (uint16_t x = 0; x < 128; x++) {
            write_prg_byte(base + address + x, sdBuffer[x]);
          }
        }
        write_prg_byte(0x7EF8, 0xFF); // PRG RAM DISABLE 0
        write_prg_byte(0x7EF9, 0xFF); // PRG RAM DISABLE 1
        break;
      }

      case 82: { // 5K
        write_prg_byte(0x7EF7, 0xCA); // PRG RAM ENABLE 0 ($6000-$67FF)
        write_prg_byte(0x7EF8, 0x69); // PRG RAM ENABLE 1 ($6800-$6FFF)
        write_prg_byte(0x7EF9, 0x84); // PRG RAM ENABLE 2 ($7000-$73FF)
        for (uint16_t address = 0x0; address < 0x1400; address += 1024) { // PRG RAM 5K ($6000-$73FF)
          readFile(ramFile, sdBuffer, 512, ramFilePath);
          byte firstbyte = sdBuffer[0];
          for (uint16_t x = 0; x < 512; x++)
            write_prg_byte(base + address + x, sdBuffer[x]);
          readFile(ramFile, sdBuffer, 512, ramFilePath);
          for (uint16_t x = 0; x < 512; x++)
            write_prg_byte(base + address + x + 512, sdBuffer[x]);
          write_prg_byte(base + address, firstbyte); // REWRITE 1ST BYTE
        }
        write_prg_byte(0x7EF7, 0xFF); // PRG RAM DISABLE 0 ($6000-$67FF)
        write_prg_byte(0x7EF8, 0xFF); // PRG RAM DISABLE 1 ($6800-$6FFF)
        write_prg_byte(0x7EF9, 0xFF); // PRG RAM DISABLE 2 ($7000-$73FF)
        break;
      }

      default: {
        if (config.mapper == 118) // 8K
          write_prg_byte(0xA001, 0x80); // PRG RAM CHIP ENABLE - Chip Enable, Allow Writes
        else if ((config.mapper == 21) || (config.mapper == 25)) // 8K
          write_prg_byte(0x8000, 0);
        else if (config.mapper == 26) // 8K
          write_prg_byte(0xB003, 0x80); // PRG RAM ENABLE
        //            else if (mapper == 68) // 8K
        //              write_reg_byte(0xF000, 0x10); // PRG RAM ENABLE [WRITE RAM SAFE]
        else if (config.mapper == 69) { // 8K
          write_prg_byte(0x8000, 8); // Command Register - PRG Bank 0
          write_prg_byte(0xA000, 0xC0); // Parameter Register - PRG RAM Enabled, PRG RAM, Bank 0 to $6000-$7FFF
        }
        else if (config.mapper == 85) // 8K
          write_ram_byte(0xE000, 0x80); // PRG RAM ENABLE
        else if (config.mapper == 153) // 8K
          write_prg_byte(0x800D, 0x20); // PRG RAM Chip Enable
        for (uint16_t address = 0; address < 0x2000; address += 512) { // 8K
          readFile(ramFile, sdBuffer, 512, ramFilePath);
          for (uint16_t x = 0; x < 512; x++) {
            write_prg_byte(base + address + x, sdBuffer[x]);
          }
        }
        if (config.mapper == 118) // 8K
          write_prg_byte(0xA001, 0xC0); // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
        else if (config.mapper == 26) // 8K
          write_prg_byte(0xB003, 0); // PRG RAM DISABLE
        //            else if (mapper == 68) // 8K
        //              write_reg_byte(0xF000, 0x00); // PRG RAM DISABLE [WRITE RAM SAFE]
        else if (config.mapper == 69) { // 8K
          write_prg_byte(0x8000, 8); // Command Register - PRG Bank 0
          write_prg_byte(0xA000, 0); // Parameter Register - PRG RAM Disabled, PRG ROM, Bank 0 to $6000-$7FFF
        }
        else if (config.mapper == 85) // 8K
          write_reg_byte(0xE000, 0); // PRG RAM DISABLE [WRITE RAM SAFE]
        break;
      }
    }
    closeFile(ramFile, ramFilePath);
    ledGreenOn();

    ui->printlnMsg(F(""));
    ui->printlnMsg(F("RAM FILE WRITTEN!"));
    ui->flushOutput();
  }

  ui->clearOutput();

  ledRedOff();
  ledGreenOff();
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

void EepromReadData_NES(byte eepbit[8]) {
  // read serial data into buffer
  for (uint8_t i = 0; i < 8; i++) {
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
  for (uint8_t i = 0; i < 7; i++) {
    if (address & 0x1) // Bit is HIGH
      EepromSet1_NES();
    else // Bit is LOW
      EepromSet0_NES();
    address >>= 1; // rotate to the next bit
  }
}

void EepromSetAddress02(byte address) { // 24C02
  for (uint8_t i = 0; i < 8; i++) {
    if ((address >> 7) & 0x1) // Bit is HIGH
      EepromSet1_NES();
    else // Bit is LOW
      EepromSet0_NES();
    address <<= 1; // rotate to the next bit
  }
  EepromStatus_NES(); // ACK
}

void EepromWriteData01(byte eeptemp) { // 24C01 [Little Endian]
  for (uint8_t i = 0; i < 8; i++) {
    if (eeptemp & 0x1) // Bit is HIGH
      EepromSet1_NES();
    else // Bit is LOW
      EepromSet0_NES();
    eeptemp >>= 1; // rotate to the next bit
  }
  EepromStatus_NES(); // ACK
}

void EepromWriteData02(byte eeptemp) { // 24C02
  for (uint8_t i = 0; i < 8; i++) {
    if ((eeptemp >> 7) & 0x1) // Bit is HIGH
      EepromSet1_NES();
    else // Bit is LOW
      EepromSet0_NES();
    eeptemp <<= 1; // rotate to the next bit
  }
  EepromStatus_NES(); // ACK
}

void EepromREAD(byte address, const CartridgeConfig &config) {
  EepromStart_NES(); // START
  byte eepbit[8];
  byte eeptemp;
  if (config.mapper == 159) { // 24C01
    EepromSetAddress01(address); // 24C01 [Little Endian]
    EepromReadMode_NES();
    EepromReadData_NES(eepbit);
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
    EepromReadData_NES(eepbit);
    EepromFinish_NES();
    EepromStop_NES(); // STOP
    // OR 8 bits into byte
    eeptemp = eepbit[0] << 7 | eepbit[1] << 6 | eepbit[2] << 5 | eepbit[3] << 4 | eepbit[4] << 3 | eepbit[5] << 2 | eepbit[6] << 1 | eepbit[7];
  }
  sdBuffer[address] = eeptemp;
}

void EepromWRITE(byte address, const CartridgeConfig &config) {
  byte eeptemp = sdBuffer[address];
  EepromStart_NES(); // START
  if (config.mapper == 159) { // 24C01
    EepromSetAddress01(address); // 24C01 [Little Endian]
    EepromWriteMode_NES();
    EepromWriteData01(eeptemp); // 24C01 [Little Endian]
  }
  else { // 24C02
    EepromDevice_NES(); // DEVICE [1010] + ADDR [A2-A0]
    EepromWriteMode_NES();
    EepromSetAddress02(address);
    EepromWriteData02(eeptemp);
  }
  EepromStop_NES(); // STOP
}

/******************************************
   [NESmaker Flash Cart Functions] [SST 39SF40]
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
String NESmaker_ID() { // Read Flash ID
  NESmaker_ResetFlash();
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0x90); // Software ID Entry
  unsigned char ID1 = read_prg_byte(0x8000);
  unsigned char ID2 = read_prg_byte(0x8001);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xF0); // Software ID Exit
  String flashID;
  if (ID1 == 0xBF && ID2 == 0xB7) {
    flashID.concat(F("BFB7"));
  }
  return flashID;
}

void NESmaker_SectorErase(byte bank, uint16_t address) {
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

void NESmaker_ByteProgram(byte bank, uint16_t address, byte data) {
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

void writeFLASH(const CartridgeConfig &config) {
  ui->clearOutput();
  if (!config.flashFound()) {
    ledRedOn();
    ui->printlnMsg(F("FLASH NOT DETECTED"));
    ui->flushOutput();
  }
  else {
    ui->printMsg(F("Flash ID: "));
    ui->printlnMsg(config.flashID);
    ui->printlnMsg(F(""));
    ui->printlnMsg(F("NESmaker Flash Found"));
    ui->printlnMsg(F(""));
    ui->flushOutput();
    delay(100);

    String flashFilePath = fileBrowser(F("Select FLASH File"));
    uint16_t base = 0x8000;

    ledRedOn();
    ui->clearOutput();
    ui->printlnMsg(F("Writing File: "));
    ui->printlnMsg(flashFilePath);
    ui->flushOutput();

    //open file on sd card
    File flashFile = openForReading(sd, flashFilePath);
    uint16_t banks = int_pow(2, config.prgSizeIndex); // 256K/512K
    for (uint16_t i = 0; i < banks; i++) {       // 16K Banks
      for (uint16_t sector = 0; sector < 0x4000; sector += 0x1000) { // 4K Sectors ($8000/$9000/$A000/$B000)
        // Sector Erase
        NESmaker_SectorErase(i, base + sector);
        delay(18); // Typical 18ms
        for (byte j = 0; j < 2; j++) { // Confirm erase twice
          byte bytecheck;
          do {
            bytecheck = read_prg_byte(base + sector);
            delay(18);
          }
          while (bytecheck != 0xFF);
        }
        // Program Byte
        for (uint16_t addr = 0x0; addr < 0x1000; addr += 512) {
          readFile(flashFile, sdBuffer, 512, flashFilePath);
          for (uint16_t x = 0; x < 512; x++) {
            uint16_t location = base + sector + addr + x;
            NESmaker_ByteProgram(i, location, sdBuffer[x]);
            delayMicroseconds(14); // Typical 14us
            for (byte k = 0; k < 2; k++) { // Confirm write twice
              byte bytecheck;
              do {
                bytecheck = read_prg_byte(location);
                delayMicroseconds(14);
              }
              while (bytecheck != sdBuffer[x]);
            }
          }
        }
      }

      ui->printMsg(F("*"));
      if ((i + 1) % 16 == 0) {
        ui->printlnMsg(F(""));
      }
      ui->flushOutput();
    }
    closeFile(flashFile, flashFilePath);
    ledGreenOn();

    ui->printlnMsg(F(""));
    ui->printlnMsg(F("FLASH FILE WRITTEN!"));
    ui->flushOutput();
  }
  ui->clearOutput();
  ledRedOff();
  ledGreenOff();
}

//******************************************
// End of File
//******************************************
