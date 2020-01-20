#ifndef cartreader_FLASH_h
#define cartreader_FLASH_h

extern unsigned long flashSize;
extern unsigned long blank;
extern boolean hiROM;

void flashMenu();
void setup_Flash8();
void id_Flash8();
void flashromMenu8();

#endif
