#ifndef cartreader_mock_SdFat_h
#define cartreader_mock_SdFat_h

#include <Arduino.h>
#include <stdint.h>

typedef uint8_t oflag_t;

#define O_CREAT 0x10
#define O_RDONLY 0X00
#define FILE_READ O_RDONLY
#define O_RDWR 0X02
#define O_EXCL 0x40
#define SPI_FULL_SPEED SPISettings()

class SPISettings {
};

class SdSpiCard {
  public:
  uint32_t cardSize();
};

class FatVolume {
  public:
  uint8_t fatType() const;
};

class FatFile {
  public:
  int read(void *buf, size_t nbyte);
  int write(void *buf, size_t nbyte);
  bool openNext(FatFile *dirFile, oflag_t oflag = O_RDONLY);
  bool getName(char *name, size_t size);
  bool isDir() const;
  bool isHidden() const;
  uint8_t getError();
  bool close();
};

class File : public FatFile {
  public:
  operator bool() {
    return true;
  }
};

class SdFat {
  public:
  File open(const String &path, oflag_t oflag = O_RDONLY);
  // operator bool? for !
  bool begin(uint8_t csPin = 53, SPISettings spiSettings = SPI_FULL_SPEED);
  SdSpiCard *card();
  FatVolume *vol();
  bool mkdir(const char *path, bool pFlag = true);
  bool chdir(const char *path, bool set_cwd = false);
};

#endif
