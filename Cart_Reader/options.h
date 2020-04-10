#ifndef cartreader_options_h
#define cartreader_options_h

// Change mainMenu to snsMenu, mdMenu, n64Menu, gbxMenu, pcsMenu,
// flashMenu, nesMenu or smsMenu for single slot Cart Readers
#define startMenu mainMenu

// Comment out to change to Serial Output
// be sure to change the Arduino Serial Monitor to no line ending
#define enable_OLED

// Skip OLED start-up animation
//#define fast_start

// Enable the second button
#define enable_Button2

#define sdSpeed SPI_FULL_SPEED

// SD Card (Pin 50 = MISO, Pin 51 = MOSI, Pin 52 = SCK, Pin 53 = SS)
#define sdChipSelectPin 53

#endif
