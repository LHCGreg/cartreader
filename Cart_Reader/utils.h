#ifndef cartreader_utils_h
#define cartreader_utils_h

#include <Arduino.h>
#include <stdint.h>
#include <EEPROM.h>

[[noreturn]] void resetArduino();

// optimization-safe nop delay
#define NOP __asm__ __volatile__("nop\n\t")

// Common I/O Functions

// Switch data pins to write
void dataOut();

// Switch data pins to read
void dataIn();

void pulseClock_N64(unsigned int times);

// CRC32 lookup table // 256 entries
extern const uint32_t crc_32_tab[] PROGMEM; /* CRC polynomial 0xedb88320 */

template <class T>
int EEPROM_writeAnything(int ee, const T &value) {
  const byte *p = (const byte *) (const void *) &value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

template <class T>
int EEPROM_readAnything(int ee, T &value) {
  byte *p = (byte *) (void *) &value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

#endif
