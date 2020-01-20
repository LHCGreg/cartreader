/**********************************************************************************
                    Cartridge Reader for Arduino Mega2560

   Author:           sanni
   Date:             03.03.2020
   Version:          4.5

   SD  lib:         https://github.com/greiman/SdFat
   LCD lib:         https://github.com/adafruit/Adafruit_SSD1306
   Clockgen:        https://github.com/etherkit/Si5351Arduino
   RGB Tools lib:   https://github.com/joushx/Arduino-RGB-Tools

   Compiled with Arduino 1.8.10

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
#include "options.h"
#include "globals.h"
#include "SD.h"
#include "utils.h"
//#include "SNES.h"
//#include "MD.h"
//#include "N64.h"
//#include "GB.h"
//#include "GBA.h"
//#include "GBSmart.h"
#include "NES.h"
//#include "FLASH.h"
//#include "PCE.h"
//#include "SMS.h"
//#include "NP.h"
//#include "SV.h"
//#include "WS.h"

/******************************************
   Variables
 *****************************************/

// Info Screen
void aboutScreen() {
  String aboutMessage(F("Cartridge Reader\ngithub.com/sanni\n2019 Version "));
  aboutMessage.concat(ver);
  ui->displayAbout(aboutMessage);
}

// All included slots
void mainMenu() {
  while (true) {
    const __FlashStringHelper *item_Addons = F("Add-ons");
    const __FlashStringHelper *item_SNES = F("Super Nintendo");
    const __FlashStringHelper *item_Megadrive = F("Mega Drive");
    const __FlashStringHelper *item_N64 = F("Nintendo 64");
    const __FlashStringHelper *item_GameBoy = F("Game Boy");
    const __FlashStringHelper *item_About = F("About");
    const __FlashStringHelper *item_Reset = F("Reset");
    const __FlashStringHelper *menu[] = {
      item_Addons,
      item_SNES,
      item_Megadrive,
      item_N64,
      item_GameBoy,
      item_About,
      item_Reset,
    };

    const __FlashStringHelper *answer = ui->askMultipleChoiceQuestion(
      F("Cartridge Reader"), menu, ARRAY_LENGTH(menu), item_Addons);

    if (answer == item_Addons) {
      addonsMenu();
    }
    // else if (answer == item_SNES) {
    //   snesMenu();
    // }
    // else if (answer == item_Megadrive) {
    //   mdMenu();
    // }
    // else if (answer == item_N64) {
    //   n64Menu();
    // }
    // else if (answer == item_GameBoy) {
    //   gbxMenu();
    // }
    else if (answer == item_About) {
      aboutScreen();
    }
    else if (answer == item_Reset) {
      resetArduino();
    }
  }
}

// Everything that needs an adapter
void addonsMenu() {
  while (true) {
    const __FlashStringHelper *item_NES = F("NES/Famicom");
    const __FlashStringHelper *item_Flashrom = F("Flashrom Programmer");
    const __FlashStringHelper *item_PCE = F("PC Engine/TG16");
    const __FlashStringHelper *item_SMS = F("Sega Master System");
    const __FlashStringHelper *item_WS = F("WonderSwan");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_NES,
      item_Flashrom,
      item_PCE,
      item_SMS,
      item_WS,
      item_Back,
    };

    const __FlashStringHelper *answer = ui->askMultipleChoiceQuestion(
      F("Choose Adapter"), menu, ARRAY_LENGTH(menu), item_NES);

    if (answer == item_NES) {
      nesMenu();
    }
    // else if (answer == item_Flashrom) {
    //   flashMenu();
    // }
    // else if (answer == item_PCE) {
    //   pcsMenu();
    // }
    // else if (answer == item_SMS) {
    //   smsMenu();
    // }
    // else if (answer == item_WS) {
    //   wsMenu();
    // }
    else if (answer == item_Back) {
      break; // exit this menu
    }
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

  ui->initialize();

  // Init SD card
  initializeSD();
}

/******************************************
  Main loop
*****************************************/
void loop() {
  startMenu();
}

//******************************************
// End of File
//******************************************
