#ifndef cartreader_FLASH_h
#define cartreader_FLASH_h

extern unsigned long flashSize;
extern unsigned long blank;
extern bool hiROM;

void flashMenu();
void flashromMenu8();
void setup_Flash8();
void id_Flash8();

void flashromMenu16();

void epromMenu();

#endif
