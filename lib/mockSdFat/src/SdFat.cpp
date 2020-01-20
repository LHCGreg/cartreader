#include "SdFat.h"

uint32_t SdSpiCard::cardSize() {
  return 0;
}

uint8_t FatVolume::fatType() const {
  return 16;
}

int FatFile::read(void *buf, size_t nbyte) {
  return 0;
}

int FatFile::write(void *buf, size_t nbyte) {
  return 0;
}

bool FatFile::openNext(FatFile *dirFile, oflag_t oflag) {
  return false;
}

bool FatFile::getName(char *name, size_t size) {
  return false;
}

bool FatFile::isDir() const {
  return false;
}

bool FatFile::isHidden() const {
  return false;
}

uint8_t FatFile::getError() {
  return 0;
}

bool FatFile::close() {
  return false;
}

File SdFat::open(const String &path, oflag_t oflag) {
  return File();
}

bool SdFat::begin(uint8_t csPin, SPISettings spiSettings) {
  return false;
}

SdSpiCard mockCard;

SdSpiCard* SdFat::card() {
  return &mockCard;
}

FatVolume mockVolume;

FatVolume* SdFat::vol() {
  return &mockVolume;
}

bool SdFat::mkdir(const char *path, bool pFlag) {
  return false;
}

bool SdFat::chdir(const char *path, bool set_cwd) {
  return false;
}

// class SdFat {
//   public:
//   File open(const String &path, oflag_t = (oflag_t) '\000');
//   // operator bool? for !
//   bool begin(uint8_t csPin = 53, SPISettings spiSettings = SPI_FULL_SPEED);
//   SdSpiCard *card();
//   FatVolume *vol();
//   bool mkdir(const char *path, bool pFlag = true);
//   bool chdir(const char *path, bool set_cwd = false);
// };