/**********************************************************************************
                    Cartridge Reader for Arduino Mega2560

   Author:           sanni
   Date:             19.03.2020
   Version:          4.7

   SD  lib:         https://github.com/greiman/SdFat
   LCD lib:         https://github.com/adafruit/Adafruit_SSD1306
   Clockgen:        https://github.com/etherkit/Si5351Arduino
   RGB Tools lib:   https://github.com/joushx/Arduino-RGB-Tools

   Compiled with Arduino 1.8.12

   Thanks to:
   MichlK - ROM-Reader for Super Nintendo
   Jeff Saltzman - 4-Way Button
   Wayne and Layne - Video-Game-Shield menu
   skaman - SNES enhancements, SA1/BSX sram support, GB flash fix, MD improvements, Famicom dumper
   nocash - Nintendo Power and GBA Eeprom commands and lots of other info
   crazynation - N64 bus timing
   hkz/themanbehindthecurtain - N64 flashram commands
   jago85 - help with N64 stuff
   Andrew Brown/Peter Den Hartog - N64 controller protocol
   bryc - mempak
   Shaun Taylor - N64 controller CRC functions
   Angus Gratton - CRC32
   Tamanegi_taro - SA1 fix, PCE and Satellaview support
   Snes9x - SuperFX sram fix
   zzattack - multigame pcb fix
   Pickle - SDD1 fix
   insidegadgets - GBCartRead
   RobinTheHood - GameboyAdvanceRomDumper
   YamaArashi - GBA flashrom bank switch command
   infinest - GB Memory Binary Maker
   moldov - SF Memory Binary Maker
   vogelfreiheit - N64 flashram fix
   rama - code speedup & improvements
   Gens-gs - Megadrive checksum
   Modman - N64 checksum comparison fix
   splash5 - EMS GB Smart cart support, Wonderswan module

**********************************************************************************/

/******************************************
   Libraries
 *****************************************/
#include <Arduino.h>
#include "globals.h"
#include "options.h"
#include "menu.h"
#include "OLED_menu.h"
#include "utils.h"
#include "RGB_LED.h"
#include "SNES.h"
#include "MD.h"
#include "N64.h"
#include "GB.h"
#include "GBA.h"
#include "GBSmart.h"
#include "NES.h"
#include "FLASH.h"
#include "PCE.h"
#include "SMS.h"
#include "NP.h"
#include "SV.h"
#include "WS.h"

/******************************************
  Menu
*****************************************/
// Main menu
static const char modeItem1[] PROGMEM = "Add-ons";
static const char modeItem2[] PROGMEM = "Super Nintendo";
static const char modeItem3[] PROGMEM = "Mega Drive";
static const char modeItem4[] PROGMEM = "Nintendo 64";
static const char modeItem5[] PROGMEM = "Game Boy";
static const char modeItem6[] PROGMEM = "About";
static const char modeItem7[] PROGMEM = "Reset";
static const char* const modeOptions[] PROGMEM = {modeItem1, modeItem2, modeItem3, modeItem4, modeItem5, modeItem6, modeItem7};

// Add-ons submenu
static const char addonsItem1[] PROGMEM = "NES/Famicom";
static const char addonsItem2[] PROGMEM = "Flashrom Programmer";
static const char addonsItem3[] PROGMEM = "PC Engine/TG16";
static const char addonsItem4[] PROGMEM = "Sega Master System";
static const char addonsItem6[] PROGMEM = "WonderSwan";
static const char addonsItem5[] PROGMEM = "Reset";
static const char* const addonsOptions[] PROGMEM = {addonsItem1, addonsItem2, addonsItem3, addonsItem4, addonsItem6, addonsItem5};

// All included slots
void mainMenu() {
  // create menu with title and 6 options to choose from
  unsigned char modeMenu;
  // Copy menuOptions out of progmem
  convertPgm(modeOptions, 7);
  modeMenu = question_box(F("Cartridge Reader"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (modeMenu)
  {
    case 0:
      addonsMenu();
      break;

    case 1:
      snsMenu();
      break;

    case 2:
      mdMenu();
      break;

    case 3:
      n64Menu();
      break;

    case 4:
      gbxMenu();
      break;

    case 5:
      aboutScreen();
      break;

    case 6:
      resetArduino();
      break;
  }
}

// Everything that needs an adapter
void addonsMenu() {
  // create menu with title and 5 options to choose from
  unsigned char addonsMenu;
  // Copy menuOptions out of progmem
  convertPgm(addonsOptions, 6);
  addonsMenu = question_box(F("Choose Adapter"), menuOptions, 6, 0);

  // wait for user choice to come back from the question box menu
  switch (addonsMenu)
  {
    case 0:
      nesMenu();
      break;

    case 1:
      flashMenu();
      break;

    case 2:
      pcsMenu();
      break;

    case 3:
      smsMenu();
      break;

    case 4:
      setup_WS();
      break;

    default:
      resetArduino();
      break;
  }
}

/******************************************
   Setup
 *****************************************/
void setup() {
  // Set Button Pins(PD7, PG2) to Input
  DDRD &= ~(1 << 7);
  DDRG &= ~(1 << 2);
  // Activate Internal Pullup Resistors
  //PORTD |= (1 << 7);
  PORTG |= (1 << 2);

  // Read current folder number out of eeprom
  foldern = loadFolderNumber();

#ifdef enable_OLED
  // GLCD
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Clear the screen buffer.
  display_Clear();

#ifndef fast_start
  delay(100);

  // Draw line
  display.drawLine(0, 32, 127, 32, WHITE);
  display_Update();
  delay(100);

  // Initialize LED
  rgb.setColor(0, 0, 0);

  // Clear the screen.
  display_Clear();
  display_Update();
  delay(25);

  // Draw the Logo
  display.drawBitmap(28, 0, icon, 72, 64, 1);
  for (int s = 1; s < 64; s += 2) {
    // Draw Scanlines
    display.drawLine(0, s, 127, s, BLACK);
  }
  display_Update();
  delay(50);

  // Clear the screen.
  display_Clear();
  display_Update();
  delay(25);

  // Draw the Logo
  display.drawBitmap(28, 0, icon, 72, 64, 1);
  for (int s = 1; s < 64; s += 2) {
    // Draw Scanlines
    display.drawLine(0, s, 127, s, BLACK);
  }
  display.setCursor(100, 55);
  display.println(ver);
  display_Update();
  delay(200);
#endif

#else
  // Serial Begin
  Serial.begin(9600);
  Serial.println(F("Cartridge Reader"));
  Serial.println(F("2019 sanni"));
  Serial.println("");
  // LED Error
  rgb.setColor(0, 0, 255);
#endif

  // Init SD card
  if (!sd.begin(sdChipSelectPin, sdSpeed)) {
    display_Clear();
    print_Error(F("SD Error"), true);
  }

#ifndef enable_OLED
  // Print SD Info
  Serial.print(F("SD Card: "));
  Serial.print(sd.card()->cardSize() * 512E-9);
  Serial.print(F("GB FAT"));
  Serial.println(int(sd.vol()->fatType()));
#endif

  startMenu();
}

/******************************************
  Main loop
*****************************************/
void loop() {
  if (mode == mode_N64_Controller) {
    n64ControllerMenu();
  }
  else if (mode == mode_N64_Cart) {
    n64CartMenu();
  }
  else if (mode == mode_SNES) {
    snesMenu();
  }
  else if (mode == mode_FLASH8) {
    flashromMenu8();
  }
  else if (mode == mode_FLASH16) {
    flashromMenu16();
  }
  else if (mode == mode_EPROM) {
    epromMenu();
  }
  else if (mode == mode_SFM) {
    sfmMenu();
  }
  else if (mode == mode_GB) {
    gbMenu();
  }
  else if (mode == mode_GBA) {
    gbaMenu();
  }
  else if (mode == mode_SFM_Flash) {
    sfmFlashMenu();
  }
  else if (mode == mode_SFM_Game) {
    sfmGameOptions();
  }
  else if (mode == mode_GBM) {
    gbmMenu();
  }
  else if (mode == mode_MD_Cart) {
    mdCartMenu();
  }
  else if (mode == mode_PCE) {
    pceMenu();
  }
  else if (mode == mode_SV) {
    svMenu();
  }
  else if (mode == mode_NES) {
    nesMenu();
  }
  else if (mode == mode_SMS) {
    smsMenu();
  }
  else if (mode == mode_SEGA_CD) {
    segaCDMenu();
  }
  else if (mode == mode_GB_GBSmart) {
    gbSmartMenu();
  }
  else if (mode == mode_GB_GBSmart_Flash) {
    gbSmartFlashMenu();
  }
  else if (mode == mode_GB_GBSmart_Game) {
    gbSmartGameOptions();
  }
  else if (mode == mode_WS) {
    wsMenu();
  }
  else {
    display_Clear();
    println_Msg(F("Menu Error"));
    println_Msg("");
    println_Msg("");
    print_Msg(F("Mode = "));
    print_Msg(mode);
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
    resetArduino();
  }
}

//******************************************
// End of File
//******************************************
