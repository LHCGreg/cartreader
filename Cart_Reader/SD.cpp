#include "SD.h"
#include "globals.h"
#include "ui.h"
#include "options.h"

SdFat sd;

void initializeSD(uint8_t chipSelectPin, const SPISettings &speed) {
  if (!sd.begin(chipSelectPin, speed)) {
    ui.printErrorAndAbort(F("SD Error"), true);
  }

  // Print SD Info
  if (ui.supportsSDInfoDisplay()) {
    uint32_t capacityGB = sd.card()->cardSize() * 512E-9;
    uint8_t FATType = sd.vol()->fatType();
    ui.displaySDInfo(capacityGB, FATType);
  }
}

SafeSDFile::SafeSDFile()
    : m_file(), m_path(String(F("[unopened file]"))), m_dontClose(true) {
  ;
}

SafeSDFile::SafeSDFile(FatFile &file, const String &path)
    : m_file(file), m_path(path), m_dontClose(false) {
  ;
}

SafeSDFile::SafeSDFile(SdFat &sd, const String &path, oflag_t mode)
    : m_path(path), m_dontClose(false) {
  m_file = sd.open(path, mode);
  if (!m_file.isOpen()) {
    if (mode & O_CREAT) {
      String errorMessage = String(F("ERROR CREATING "));
      errorMessage.concat(path);
      ui.printErrorAndAbort(errorMessage, true);
    }
    else {
      String errorMessage = String(F("ERROR OPENING "));
      errorMessage.concat(path);
      ui.printErrorAndAbort(errorMessage, true);
    }
  }
}

SafeSDFile::SafeSDFile(SafeSDFile &&other)
    : m_file(other.m_file), m_path(other.m_path), m_dontClose(other.m_dontClose) {
  other.m_dontClose = true;
}

SafeSDFile &SafeSDFile::operator=(SafeSDFile &&other) {
  if (this != &other) {
    close();
    m_file = other.m_file;
    m_path = other.m_path;
    m_dontClose = other.m_dontClose;
    other.m_dontClose = true;
  }
  return *this;
}

SafeSDFile SafeSDFile::openForReading(const String &path) {
  return SafeSDFile(sd, path, O_RDONLY);
}

SafeSDFile SafeSDFile::openForCreating(const String &path) {
  String containingDir = pathGetDir(path);
  if (containingDir.length() > 0 && containingDir != "/" && !fileExists(path)) {
    mkdir(containingDir, true);
  }
  return SafeSDFile(sd, path, O_RDWR | O_CREAT | O_EXCL);
}

SafeSDFile SafeSDFile::openForWriting(const String &path) {
  return SafeSDFile(sd, path, O_RDWR);
}

SafeSDFile SafeSDFile::getVwd() {
  FatFile *vwd = sd.vwd();
  SafeSDFile safeVWD(*vwd, String(F("[vwd]")));
  safeVWD.m_dontClose = true;
  return safeVWD;
}

size_t SafeSDFile::read(byte *buffer, size_t numBytes) {
  int numBytesRead = m_file.read(buffer, numBytes);
  if (numBytesRead == -1) {
    String errorMessage = F("ERROR READING ");
    errorMessage.concat(m_path);
    ui.printErrorAndAbort(errorMessage, true);
  }
  return numBytesRead;
}

const char progmemStringUnexpectedEOF[] PROGMEM = "Error: unexpected end of file";
const __FlashStringHelper *unexpectedEOF = reinterpret_cast<const __FlashStringHelper *>(progmemStringUnexpectedEOF);

void SafeSDFile::readOrDie(byte *buffer, size_t numBytes) {
  readOrDie(buffer, numBytes, unexpectedEOF);
}

uint8_t SafeSDFile::readByteOrDie() {
  return readByteOrDie(unexpectedEOF);
}

void SafeSDFile::write(byte *buffer, size_t numBytes) {
  int numBytesWritten = m_file.write(buffer, numBytes);
  if (numBytesWritten == -1) {
    String errorMessage = F("ERROR WRITING ");
    errorMessage.concat(m_path);
    ui.printErrorAndAbort(errorMessage, true);
  }
  // numBytesWritten is always numBytes if it succeeds.
}

void SafeSDFile::writeByte(byte b) {
  write(&b, 1);
}

void SafeSDFile::seekCur(int32_t offset) {
  bool success = m_file.seekCur(offset);
  if (!success) {
    String errorMessage = F("ERROR SEEKING ");
    errorMessage.concat(offset);
    errorMessage.concat(F(" BYTES FROM CURRENT IN "));
    errorMessage.concat(m_path);
    ui.printErrorAndAbort(errorMessage, true);
  }
}

void SafeSDFile::seekSet(uint32_t pos) {
  bool success = m_file.seekSet(pos);
  if (!success) {
    String errorMessage = F("ERROR SEEKING TO BYTE OFFSET ");
    errorMessage.concat(pos);
    errorMessage.concat(F(" IN "));
    errorMessage.concat(m_path);
    ui.printErrorAndAbort(errorMessage, true);
  }
}

SafeSDFile SafeSDFile::readNextDirectoryEntry() {
  File file;
  bool success = file.openNext(&m_file);
  if (!success && m_file.getError()) {
    String errorMessage = F("ERROR READING DIR ");
    errorMessage.concat(m_path);
    ui.printErrorAndAbort(errorMessage, true);
  }

  if (success) {
    return SafeSDFile(file, String(F("[file in dir]")));
  }
  else {
    return SafeSDFile();
  }
}

void SafeSDFile::getName(char *outName, size_t size) {
  bool success = m_file.getName(outName, size);
  if (!success) {
    ui.printErrorAndAbort(F("ERROR GETTING NAME OF FILE"), true);
  }
}

uint32_t SafeSDFile::fileSize() {
  return m_file.fileSize();
}

uint32_t SafeSDFile::bytesAvailable() {
  return m_file.available();
}

bool SafeSDFile::isOpen() {
  return m_file.isOpen();
}

bool SafeSDFile::isHidden() {
  return m_file.isHidden();
}

bool SafeSDFile::isDir() {
  return m_file.isDir();
}

bool SafeSDFile::isFile() {
  return m_file.isFile();
}

SafeSDFile::~SafeSDFile() {
  close();
}

void SafeSDFile::rename(const String &newPath) {
  bool success = m_file.rename(newPath.c_str());
  if (!success) {
    String errorMessage = F("ERROR RENAMING ");
    errorMessage.concat(m_path);
    errorMessage.concat(F(" TO "));
    errorMessage.concat(newPath);
    ui.printErrorAndAbort(errorMessage, true);
  }
}

void SafeSDFile::remove() {
  bool success = m_file.remove();
  if (!success) {
    String errorMessage = F("ERROR DELETING ");
    errorMessage.concat(m_path);
    ui.printErrorAndAbort(errorMessage, true);
  }
}

void SafeSDFile::close() {
  if (m_dontClose) {
    return;
  }

  bool success = m_file.close();
  if (!success) {
    String errorMessage = F("ERROR CLOSING ");
    errorMessage.concat(m_path);
    ui.printErrorAndAbort(errorMessage, true);
  }

  m_dontClose = true;
}

void copyFile(SafeSDFile &sourceFile, SafeSDFile &destFile, byte *buffer, size_t bufSize) {
  size_t numBytesRead;
  while ((numBytesRead = sourceFile.read(buffer, bufSize)) > 0) {
    destFile.write(buffer, numBytesRead);
  }
}

void mkdir(const char *path, bool createParentDirectories) {
  bool success = sd.mkdir(path, createParentDirectories);
  if (!success) {
    String errorMessage = F("ERROR CREATING DIR ");
    errorMessage.concat(path);
    ui.printErrorAndAbort(errorMessage, true);
  }
}

void mkdir(const String &path, bool createParentDirectories) {
  mkdir(path.c_str(), createParentDirectories);
}

void chdirToRoot() {
  bool success = sd.chdir();
  if (!success) {
    ui.printErrorAndAbort(F("CHDIR TO ROOT FAILED\nSD ERROR"), true);
  }
}

void chdir(const char *path) {
  bool success = sd.chdir(path);
  if (!success) {
    String errorMessage = F("CHDIR ");
    errorMessage.concat(path);
    errorMessage.concat(F(" FAILED\nSD ERROR"));
    ui.printErrorAndAbort(errorMessage, true);
  }
}

void chdir(const String &path) {
  chdir(path.c_str());
}

bool fileExists(const char *path) {
  return sd.exists(path);
}

bool fileExists(const String &path) {
  return sd.exists(path.c_str());
}
