//******************************************
// PC Engine & TurboGrafx dump code by tamanegi_taro
// April 18th 2018 Revision 1.0.1 Initial version
// August 12th 2019 Revision 1.0.2 Added Tennokoe Bank support
//
// Special thanks
// sanni - Arduino cart reader
// skaman - ROM size detection
// NO-INTRO - CRC list for game name detection
// Chris Covell - Tennokoe bank support
//
//******************************************

#include <Arduino.h>
#include <avr/io.h>
#include "PCE.h"
#include "filebrowser.h"
#include "ui.h"
#include "globals.h"
#include "utils.h"
#include "SD.h"

/******************************************
  Defines
 *****************************************/
#define HUCARD 0
#define TURBOCHIP 1

#define DETECTION_SIZE 64
#define CHKSUM_SKIP 0
#define CHKSUM_OK 1
#define CHKSUM_ERROR 2

/******************************************
   Prototype Declarations
 *****************************************/
/* Several PCE dedicated functions */
void hucardMenu();
void turbochipMenu();
void pin_read_write_PCE(void);
void pin_init_PCE(void);
void setup_cart_PCE(void);
void reset_cart_PCE(void);
uint8_t read_byte_PCE(uint32_t address);
void write_byte_PCE(uint32_t address, uint8_t data);
uint32_t detect_rom_size_PCE(void);
void read_bank_PCE(uint32_t address_start, uint32_t address_end, uint32_t *processed_size, uint32_t total_size, SafeSDFile &outputFile);
void read_tennokoe_bank_PCE();
void write_tennokoe_bank_PCE();
void read_rom_PCE(void);

/******************************************
   Variables
 *****************************************/
uint8_t pce_internal_mode; //0 - HuCARD, 1 - TurboChip

/******************************************
  Menu
*****************************************/
// PCE start menu
void pcsMenu(void) {
  while (true) {
    const __FlashStringHelper *item_Hucard = F("HuCARD");
    const __FlashStringHelper *item_Turbochip = F("Turbochip");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_Hucard,
      item_Turbochip,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("Select device"), menu, ARRAY_LENGTH(menu), item_Hucard);

    if (answer == item_Hucard) {
      ui.clearOutput();
      ui.flushOutput();
      pce_internal_mode = HUCARD;
      setup_cart_PCE();
      mode = CartReaderMode::PCE;
      hucardMenu();
    }
    else if (answer == item_Turbochip) {
      ui.clearOutput();
      ui.flushOutput();
      pce_internal_mode = TURBOCHIP;
      setup_cart_PCE();
      mode = CartReaderMode::PCE;
      turbochipMenu();
    }
    else if (answer == item_Back) {
      break;
    }
  }
}

void hucardMenu() {
  while (true) {
    const __FlashStringHelper *item_ReadROM = F("Read Rom");
    const __FlashStringHelper *item_ReadTennokoe = F("Read Tennokoe Bank");
    const __FlashStringHelper *item_WriteTennokoe = F("Write Tennokoe Bank");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_ReadROM,
      item_ReadTennokoe,
      item_WriteTennokoe,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("PCE HuCARD menu"), menu, ARRAY_LENGTH(menu), item_ReadROM);

    if (answer == item_ReadROM) {
      ui.clearOutput();
      read_rom_PCE();
    }
    else if (answer == item_ReadTennokoe) {
      ui.clearOutput();
      read_tennokoe_bank_PCE();
    }
    else if (answer == item_WriteTennokoe) {
      ui.clearOutput();
      write_tennokoe_bank_PCE();
    }
    else if (answer == item_Back) {
      break;
    }

    ui.printlnMsg("");
    ui.printlnMsg(F("Press Button..."));
    ui.flushOutput();
    ui.waitForUserInput();
  }
}

void turbochipMenu() {
  while (true) {
    const __FlashStringHelper *item_ReadROM = F("Read Rom");
    const __FlashStringHelper *item_Back = F("Back");
    const __FlashStringHelper *menu[] = {
      item_ReadROM,
      item_Back,
    };

    const __FlashStringHelper *answer = ui.askMultipleChoiceQuestion(
      F("TG TurboChip menu"), menu, ARRAY_LENGTH(menu), item_ReadROM);

    if (answer == item_ReadROM) {
      ui.clearOutput();
      read_rom_PCE();
    }
    else if (answer == item_Back) {
      break;
    }

    ui.printlnMsg("");
    ui.printlnMsg(F("Press Button..."));
    ui.flushOutput();
    ui.waitForUserInput();
  }
}

void pin_read_write_PCE(void)
{
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A19
  DDRL = (DDRL & 0xF0) | 0x0F;

  //Set Control Pin to Output CS(PL4)
  DDRL |= (1 << 4);

  //Set CS(PL4) to HIGH
  PORTL |= (1 << 4);

  // Set Control Pins to Output RST(PH0) RD(PH3) WR(PH5)
  DDRH |= (1 << 0) | (1 << 3) | (1 << 5);
  // Switch all of above to HIGH
  PORTH |= (1 << 0) | (1 << 3) | (1 << 5);

  // Set IRQ(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  PORTH |= (1 << 4);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;

  // Enable Internal Pullups
  PORTC = 0xFF;

  reset_cart_PCE();
}

void pin_init_PCE(void)
{

  //Set Address Pins to input and pull up
  DDRF = 0x00;
  PORTF = 0xFF;
  DDRK = 0x00;
  PORTK = 0xFF;
  DDRL = 0x00;
  PORTL = 0xFF;
  DDRH &= ~((1 << 0) | (1 << 3) | (1 << 5) | (1 << 6));
  PORTH = (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);

  // Set IRQ(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  PORTH |= (1 << 4);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Enable Internal Pullups
  PORTC = 0xFF;

}

void setup_cart_PCE(void)
{
  // Set cicrstPin(PG1) to Output
  DDRG |= (1 << 1);
  // Output a high to disable CIC
  PORTG |= (1 << 1);

  pin_init_PCE();

}

void reset_cart_PCE(void)
{
  //Set RESET as Low
  PORTH &= ~(1 << 0);
  delay(200);
  //Set RESET as High
  PORTH |= (1 << 0);
  delay(200);

}

void set_address_PCE(uint32_t address)
{
  //Set address
  PORTF = address & 0xFF;
  PORTK = (address >> 8) & 0xFF;
  PORTL = (PORTL & 0xF0) | ((address >> 16) & 0x0F);
}

uint8_t read_byte_PCE(uint32_t address)
{
  uint8_t ret;

  set_address_PCE(address);

  // Arduino running at 16Mhz -> one nop = 62.5ns -> 1000ns total
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Set CS(PL4) and RD(PH3) as LOW
  PORTL &= ~(1 << 4);
  PORTH &= ~(1 << 3);

  // Arduino running at 16Mhz -> one nop = 62.5ns -> 1000ns total
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  //read byte
  ret =  PINC;

  //Swap bit order for PC Engine HuCARD
  if (pce_internal_mode == HUCARD)
  {
    ret = ((ret & 0x01) << 7) | ((ret & 0x02) << 5) | ((ret & 0x04) << 3) | ((ret & 0x08) << 1) | ((ret & 0x10) >> 1) | ((ret & 0x20) >> 3) | ((ret & 0x40) >> 5) | ((ret & 0x80) >> 7);
  }

  // Set CS(PL4) and RD(PH3) as HIGH
  PORTL |= (1 << 4);
  PORTH |= (1 << 3);

  //return read data
  return ret;

}

void write_byte_PCE(uint32_t address, uint8_t data)
{
  set_address_PCE(address);

  // Arduino running at 16Mhz -> one nop = 62.5ns -> 1000ns total
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  //Swap bit order for PC Engine HuCARD
  if (pce_internal_mode == HUCARD)
  {
    data = ((data & 0x01) << 7) | ((data & 0x02) << 5) | ((data & 0x04) << 3) | ((data & 0x08) << 1) | ((data & 0x10) >> 1) | ((data & 0x20) >> 3) | ((data & 0x40) >> 5) | ((data & 0x80) >> 7);
  }

  //write byte
  PORTC = data;

  // Set Data Pins (D0-D7) to Output
  DDRC = 0xFF;

  // Set CS(PL4) and WR(PH5) as LOW
  PORTL &= ~(1 << 4);
  PORTH &= ~(1 << 5);

  // Arduino running at 16Mhz -> one nop = 62.5ns -> 1000ns total
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Set CS(PL4) and WR(PH5) as HIGH
  PORTL |= (1 << 4);
  PORTH |= (1 << 5);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;

  // Enable Internal Pullups
  PORTC = 0xFF;
}

//Confirm the size of ROM - 128Kb, 256Kb, 384Kb, 512Kb, 768Kb or 1024Kb
uint32_t detect_rom_size_PCE(void)
{
  uint32_t rom_size;
  uint8_t read_byte;
  uint8_t current_byte;
  uint8_t detect_128, detect_256, detect_512, detect_768;

  //Initialize variables
  detect_128 = 0;
  detect_256 = 0;
  detect_512 = 0;
  detect_768 = 0;

  //Set pins to read PC Engine cart
  pin_read_write_PCE();

  //Confirm where mirror address start from(128KB, 256KB, 512KB, 768, or 1024KB)
  for (current_byte = 0; current_byte < DETECTION_SIZE; current_byte++) {
    if ((current_byte != detect_128) && (current_byte != detect_256) && (current_byte != detect_512) && (current_byte != detect_768))
    {
      //If none matched, it is 1024KB
      break;
    }

    //read byte for 128KB, 256KB, 512KB detection
    read_byte = read_byte_PCE(current_byte);

    //128KB detection
    if (current_byte == detect_128)
    {
      if (read_byte_PCE(current_byte + 128UL * 1024UL) == read_byte)
      {
        detect_128++;
      }
    }

    //256KB detection
    if (current_byte == detect_256)
    {
      if (read_byte_PCE(current_byte + 256UL * 1024UL) == read_byte)
      {
        detect_256++;
      }
    }

    //512KB detection
    if (current_byte == detect_512)
    {
      if (read_byte_PCE(current_byte + 512UL * 1024UL) == read_byte)
      {
        detect_512++;
      }
    }

    //768KB detection
    read_byte = read_byte_PCE(current_byte + 512UL * 1024UL);
    if (current_byte == detect_768)
    {
      if (read_byte_PCE(current_byte + 768UL * 1024UL) == read_byte)
      {
        detect_768++;
      }
    }
  }

  //debug
  //sprintf(fileName, "%d %d %d %d", detect_128, detect_256, detect_512, detect_768); //using filename global variable as string. Initialzed in below anyways.
  //ui.printlnMsg(fileName);

  //ROM size detection by result
  if (detect_128 == DETECTION_SIZE)
  {
    rom_size = 128;
  }
  else if (detect_256 == DETECTION_SIZE)
  {
    if (detect_512 == DETECTION_SIZE)
    {
      rom_size = 256;
    }
    else
    {
      //Another confirmation for 384KB because 384KB hucard has data in 0x0--0x40000 and 0x80000--0xA0000(0x40000 is mirror of 0x00000)
      rom_size = 384;
    }
  }
  else if (detect_512 == DETECTION_SIZE)
  {
    rom_size = 512;
  }
  else if (detect_768 == DETECTION_SIZE)
  {
    rom_size = 768;
  }
  else
  {
    rom_size = 1024;
  }

  //If rom size is more than or equal to 512KB, detect Street fighter II'
  if (rom_size >= 512)
  {
    //Look for "NEC HE "
    if (read_byte_PCE(0x7FFF9) == 'N' && read_byte_PCE(0x7FFFA) == 'E'  && read_byte_PCE(0x7FFFB) == 'C'
        && read_byte_PCE(0x7FFFC) == ' ' && read_byte_PCE(0x7FFFD) == 'H' && read_byte_PCE(0x7FFFE) == 'E')
    {
      rom_size = 2560;
    }
  }

  return rom_size;
}

/* Must be address_start and address_end should be 512 byte aligned */
void read_bank_PCE(uint32_t address_start, uint32_t address_end, uint32_t *processed_size, uint32_t total_size, SafeSDFile &outputFile)
{
  uint32_t currByte;
  uint16_t c;

  for (currByte = address_start; currByte < address_end; currByte += 512) {
    for (c = 0; c < 512; c++) {
      sdBuffer[c] = read_byte_PCE(currByte + c);
    }
    outputFile.write(sdBuffer, 512);
    *processed_size += 512;
    ui.drawProgressBar(*processed_size, total_size);
  }
}

//Get line from file and convert upper case to lower case
void skip_line(SafeSDFile &readfile)
{
  int i = 0;
  char str_buf;

  while (readfile.bytesAvailable() > 0)
  {
    //Read 1 byte from file
    str_buf = readfile.readByteOrDie();

    //if end of file or newline found, execute command
    if (str_buf == '\r')
    {
      readfile.readByteOrDie(); //dispose \n because \r\n
      break;
    }
    i++;
  }//End while
}

//Get line from file and convert upper case to lower case
void get_line(char* str_buf, SafeSDFile &readfile, uint8_t maxi)
{
  int i = 0;

  while (readfile.bytesAvailable() > 0)
  {
    //If line size is more than maximum array, limit it.
    if (i >= maxi)
    {
      i = maxi - 1;
    }

    //Read 1 byte from file
    str_buf[i] = readfile.readByteOrDie();

    //if end of file or newline found, execute command
    if (str_buf[i] == '\r')
    {
      str_buf[i] = '\0';
      readfile.readByteOrDie(); //dispose \n because \r\n
      break;
    }
    i++;
  }//End while
}

uint32_t calculate_crc32(int n, unsigned char c[], uint32_t r)
{
  int i, j;

  for (i = 0; i < n; i++) {
    r ^= c[i];
    for (j = 0; j < 8; j++)
      if (r & 1) r = (r >> 1) ^ 0xEDB88320UL;
      else       r >>= 1;
  }
  return r;
}

void crc_search(const String &tempOutputFilePath, uint32_t rom_size)
{
  uint32_t r, crc, processedsize;
  char gamename[100];
  char crc_file[9], crc_search[9];
  uint8_t flag;
  flag = CHKSUM_SKIP;

  //Open list file. If no list file found, just skip
  
  if (fileExists(F("/PCE_CRC_LIST.txt")))
  {
    SafeSDFile script = SafeSDFile::openForReading(F("/PCE_CRC_LIST.txt"));
    //Calculate CRC of ROM file
    SafeSDFile rom = SafeSDFile::openForReading(tempOutputFilePath);

    //Initialize flag as error
    flag = CHKSUM_ERROR;
    crc = 0xFFFFFFFFUL; //Initialize CRC
    ui.clearOutput();
    ui.printlnMsg(F("Calculating chksum..."));
    processedsize = 0;
    ui.drawProgressBar(0, rom_size * 1024UL); //Initialize progress bar

    while (rom.bytesAvailable() > 0)
    {
      r = rom.read(sdBuffer, 512);
      crc = calculate_crc32(r, sdBuffer, crc);
      processedsize += r;
      ui.drawProgressBar(processedsize, rom_size * 1024UL);
    }

    crc = crc ^ 0xFFFFFFFFUL; //Finish CRC calculation and progress bar
    ui.drawProgressBar(rom_size * 1024UL, rom_size * 1024UL);

    //Display calculated CRC
    sprintf(crc_file, "%08lX", crc);

    //Search for same CRC in list
    while (script.bytesAvailable() > 0) {
      //Read 2 lines (game name and CRC)
      get_line(gamename, script, 96);
      get_line(crc_search, script, 9);
      skip_line(script); //Skip every 3rd line

      //if checksum search successful, rename the file and end search
      if (strcmp(crc_search, crc_file) == 0)
      {
        String newFileName = gamename;
        newFileName.concat(F(".pce"));

        int16_t previousFolderNumber = loadFolderNumber() - 1;
        String pathToMoveTo = F("/PCE/ROM/");
        pathJoinInPlace(pathToMoveTo, gamename);
        pathJoinInPlace(pathToMoveTo, previousFolderNumber);
        pathJoinInPlace(pathToMoveTo, newFileName);

        rom.rename(pathToMoveTo);

        ui.printMsg(F("Chksum OK "));
        ui.printlnMsg(crc_file);
        ui.printMsg(F("Saved to "));
        ui.printMsg(pathToMoveTo);
        flag = CHKSUM_OK;
        break;
      }
    }
    rom.close();
    script.close();
  }


  if (flag == CHKSUM_SKIP)
  {
    ui.printMsg(F("Saved to "));
    ui.printMsg(tempOutputFilePath);
  }
  else if (flag == CHKSUM_ERROR)
  {
    ui.printMsg(F("Chksum Error "));
    ui.printlnMsg(crc_file);
    ui.printMsg(F("Saved to "));
    ui.printMsg(tempOutputFilePath);
  }
}


void read_tennokoe_bank_PCE(void)
{
  uint32_t processed_size = 0;
  uint32_t verify_loop;
  uint8_t verify_flag = 1;

  //clear the screen
  ui.clearOutput();

  ui.printlnMsg(F("RAM size: 8KB"));

  String outputFilePath = getNextOutputPathWithNumberedFolder(F("PCE"), F("ROM"), F("BANKRAM"), F(".sav"));

  ui.printlnMsg(F("Saving RAM..."));
  ui.flushOutput();

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(outputFilePath);

  pin_read_write_PCE();

  //Initialize progress bar by setting processed size as 0
  ui.drawProgressBar(0, 8 * 1024UL);

  //Unlock Tennokoe Bank RAM
  write_byte_PCE(0x0D0000, 0x68); //Unlock RAM sequence 1 Bank 68
  write_byte_PCE(0x0F0000, 0x0); //Unlock RAM sequence 2 Bank 78
  write_byte_PCE(0x0F0000, 0x73); //Unlock RAM sequence 3 Bank 78
  write_byte_PCE(0x0F0000, 0x73); //Unlock RAM sequence 4 Bank 78
  write_byte_PCE(0x0F0000, 0x73); //Unlock RAM sequence 5 Bank 78

  //Read Tennokoe bank RAM
  read_bank_PCE(0x080000, 0x080000 + 8 * 1024UL, &processed_size, 8 * 1024UL, outputFile);

  outputFile.seekSet(0);    // Go back to file beginning
  processed_size = 0;

  //Verify Tennokoe bank RAM
  for (verify_loop = 0; verify_loop < 8 * 1024UL; verify_loop++)
  {
    if (outputFile.readByteOrDie() != read_byte_PCE(verify_loop + 0x080000))
    {
      verify_flag = 0;
      ui.drawProgressBar(8 * 1024UL, 8 * 1024UL);
      break;
    }
    ui.drawProgressBar(verify_loop, 8 * 1024UL);
  }

  //If verify flag is 0, verify failed
  if (verify_flag == 1)
  {
    ui.printlnMsg(F("Verify OK..."));
  }
  else
  {
    ui.printlnMsg(F("Verify failed..."));
  }

  //Lock Tennokoe Bank RAM
  write_byte_PCE(0x0D0000, 0x68); //Lock RAM sequence 1 Bank 68
  write_byte_PCE(0x0F0001, 0x0); //Lock RAM sequence 2 Bank 78
  write_byte_PCE(0x0C0001, 0x60); //Lock RAM sequence 3 Bank 60

  pin_init_PCE();

  //Close the file:
  outputFile.close();
}

void write_tennokoe_bank_PCE(void)
{
  uint32_t readwrite_loop, verify_loop;
  uint32_t verify_flag = 1;

  //Display file Browser and wait user to select a file. Size must be 8KB.
  String inputFilePath = fileBrowser(F("Select RAM file"));
  ui.clearOutput();

  //open file on sd card
  SafeSDFile inputFile = SafeSDFile::openForReading(inputFilePath);

  fileSize = inputFile.fileSize();
  if (fileSize != 8 * 1024UL) {
    ui.printlnMsg(F("File must be 1MB"));
    ui.flushOutput();
    inputFile.close();
    ui.waitForUserInput();
    return;
  }

  pin_read_write_PCE();

  //Unlock Tennokoe Bank RAM
  write_byte_PCE(0x0D0000, 0x68); //Unlock RAM sequence 1 Bank 68
  write_byte_PCE(0x0F0000, 0x0); //Unlock RAM sequence 2 Bank 78
  write_byte_PCE(0x0F0000, 0x73); //Unlock RAM sequence 3 Bank 78
  write_byte_PCE(0x0F0000, 0x73); //Unlock RAM sequence 4 Bank 78
  write_byte_PCE(0x0F0000, 0x73); //Unlock RAM sequence 5 Bank 78

  //Write file to Tennokoe BANK RAM
  for (readwrite_loop = 0; readwrite_loop < 8 * 1024UL; readwrite_loop++)
  {
    write_byte_PCE(0x080000 + readwrite_loop, inputFile.readByteOrDie());
    ui.drawProgressBar(readwrite_loop, 8 * 1024UL);
  }

  inputFile.seekSet(0);    // Go back to file beginning

  for (verify_loop = 0; verify_loop < 8 * 1024UL; verify_loop++)
  {
    if (inputFile.readByteOrDie() != read_byte_PCE(verify_loop + 0x080000))
    {
      ui.drawProgressBar(2 * 1024UL, 8 * 1024UL);
      verify_flag = 0;
      break;
    }
    ui.drawProgressBar(verify_loop, 8 * 1024UL);
  }

  //If verify flag is 0, verify failed
  if (verify_flag == 1)
  {
    ui.printlnMsg(F("Verify OK..."));
  }
  else
  {
    ui.printlnMsg(F("Verify failed..."));
  }

  //Lock Tennokoe Bank RAM
  write_byte_PCE(0x0D0000, 0x68); //Lock RAM sequence 1 Bank 68
  write_byte_PCE(0x0F0001, 0x0); //Lock RAM sequence 2 Bank 78
  write_byte_PCE(0x0C0001, 0x60); //Lock RAM sequence 3 Bank 60

  pin_init_PCE();

  // Close the file:
  inputFile.close();
  ui.printlnMsg(F("Finished"));
  ui.flushOutput();
  ui.waitForUserInput();
}

void read_rom_PCE(void)
{
  uint32_t rom_size;
  uint32_t processed_size = 0;

  //clear the screen
  ui.clearOutput();
  rom_size = detect_rom_size_PCE();

  ui.printMsg(F("Detected size: "));
  ui.printMsg(rom_size);
  ui.printlnMsg(F("KB"));

  String tempOutputFilePath = getNextOutputPathWithNumberedFilename(F("PCE/ROM"), F("PCEROM"), F(".pce"));

  ui.printlnMsg(F("Saving ROM..."));
  ui.flushOutput();

  //open file on sd card
  SafeSDFile outputFile = SafeSDFile::openForCreating(tempOutputFilePath);

  pin_read_write_PCE();

  //Initialize progress bar by setting processed size as 0
  ui.drawProgressBar(0, rom_size * 1024UL);

  if (rom_size == 384)
  {
    //Read two sections. 0x000000--0x040000 and 0x080000--0x0A0000 for 384KB
    read_bank_PCE(0, 0x40000, &processed_size, rom_size * 1024UL, outputFile);
    read_bank_PCE(0x80000, 0xA0000, &processed_size, rom_size * 1024UL, outputFile);
  }
  else if (rom_size == 2560)
  {
    //Dump Street fighter II' Champion Edition
    read_bank_PCE(0, 0x80000, &processed_size, rom_size * 1024UL, outputFile);  //Read first bank
    write_byte_PCE(0x1FF0, 0xFF); //Display second bank
    read_bank_PCE(0x80000, 0x100000, &processed_size, rom_size * 1024UL, outputFile); //Read second bank
    write_byte_PCE(0x1FF1, 0xFF); //Display third bank
    read_bank_PCE(0x80000, 0x100000, &processed_size, rom_size * 1024UL, outputFile); //Read third bank
    write_byte_PCE(0x1FF2, 0xFF); //Display forth bank
    read_bank_PCE(0x80000, 0x100000, &processed_size, rom_size * 1024UL, outputFile); //Read forth bank
    write_byte_PCE(0x1FF3, 0xFF); //Display fifth bank
    read_bank_PCE(0x80000, 0x100000, &processed_size, rom_size * 1024UL, outputFile); //Read fifth bank
  }
  else
  {
    //Read start form 0x000000 and keep reading until end of ROM
    read_bank_PCE(0, rom_size * 1024UL, &processed_size, rom_size * 1024UL, outputFile);
  }

  pin_init_PCE();

  //Close the file:
  outputFile.close();

  //CRC search and rename ROM
  crc_search(tempOutputFilePath, rom_size);

}

//******************************************
// End of File
//******************************************