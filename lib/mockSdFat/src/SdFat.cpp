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

bool FatFile::rename(const char *newPath) {
  return false;
}

bool FatFile::remove() {
  return false;
}

bool FatFile::getName(char *name, size_t size) {
  return false;
}

bool FatFile::seekCur(int32_t offset) {
  return false;
}

bool FatFile::seekSet(uint32_t pos) {
  return false;
}

bool FatFile::isFile() const {
  return true;
}

bool FatFile::isDir() const {
  return false;
}

bool FatFile::isHidden() const {
  return false;
}

bool FatFile::isOpen() const {
  return false;
}

uint32_t FatFile::fileSize() const {
  return 123;
}

uint32_t FatFile::available() {
  return 123;
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

SdSpiCard *SdFat::card() {
  return &mockCard;
}

FatVolume mockVolume;

FatVolume *SdFat::vol() {
  return &mockVolume;
}

FatFile mockFile;

FatFile *SdFat::vwd() {
  return &mockFile;
}

bool SdFat::mkdir(const char *path, bool pFlag) {
  return false;
}

bool SdFat::chdir(const char *path, bool set_cwd) {
  return false;
}

bool SdFat::exists(const char *path) {
  return false;
}
