#ifndef cartreader_EEPROM_h
#define cartreader_EEPROM_h

#include <stdint.h>

class EEPROMClass {
  public:
  uint8_t read(int idx);
  void write(int idx, uint8_t val);
};

extern EEPROMClass EEPROM;

#endif
