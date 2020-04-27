#ifndef cartreader_utils_h
#define cartreader_utils_h

#include <Arduino.h>
#include <stdint.h>

[[noreturn]] void resetArduino();

// optimization-safe nop delay
#define NOP __asm__ __volatile__("nop\n\t")

// AVR Eeprom

void saveFolderNumber(int16_t folderNumber);
int16_t loadFolderNumber();
void saveNESMapperNumber(uint8_t mapperNumber);
uint8_t loadNESMapperNumber();
void saveNESPRG(uint8_t prg);
uint8_t loadNESPRG();
void saveNESCHR(uint8_t chr);
uint8_t loadNESCHR();
void saveNESRAM(uint8_t ram);
uint8_t loadNESRAM();

// Common I/O Functions

// Switch data pins to write
void dataOut();

// Switch data pins to read
void dataIn();

// get length of fixed size array
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

// Parses the null-terminated string str as an unsigned number.
// Sets errorCode to 0 and returns the parsed number on success.
// Sets errorCode to a non-zero value on failure.
uint32_t stringToNumber(const char *const str, int8_t &errorCode);

void ensureEndsInSlash(String &path);

template <class concatable>
String pathJoin(const String &path, const concatable &otherPath) {
  String result = path;
  if (result.length() != 0) {
    ensureEndsInSlash(result);
  }
  result.concat(otherPath);
  return result;
}

template <class concatable>
void pathJoinInPlace(String &path, const concatable &otherPath) {
  if (path.length() != 0) {
    ensureEndsInSlash(path);
  }
  path.concat(otherPath);
}

String pathGetDir(const String &path);

void printSavingMessage(const String &outputFilePath);
String getNextOutputPathWithNumberedFolder(const String &console, const String &fileType, const String &romName, const String &extension);
String getNextOutputPathWithNumberedFolderAndPrintMessage(const String &console, const String &fileType, const String &romName, const String &extension);
String getNextOutputPathWithNumberedFilename(const String &fileType, const String &fileNamePrefix, const String &extension);
String getNextOutputPathWithNumberedFilenameAndPrintMessage(const String &fileType, const String &fileNamePrefix, const String &extension);

// Power for int
uint16_t int_pow(uint16_t base, uint8_t exp);

void pulseClock_N64(unsigned int times);

// CRC32 lookup table // 256 entries
extern const uint32_t crc_32_tab[] PROGMEM; /* CRC polynomial 0xedb88320 */

#endif
