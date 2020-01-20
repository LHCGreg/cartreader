#ifndef cartreader_SD_h
#define cartreader_SD_h

#include <Arduino.h>
#include <SdFat.h>
#include "ui.h"

void initializeSD();

template <class pathType>
File open(SdFat &sd, const pathType &path, oflag_t mode) {
  File file = sd.open(path, mode);
  if (!file) {
    if (mode & O_CREAT) {
      String errorMessage = F("ERROR CREATING ");
      errorMessage.concat(path);
      errorMessage.concat(F("\nSD ERROR"));
      ui->printErrorAndAbort(errorMessage);
    }
    else {
      String errorMessage = String(F("ERROR OPENING ")) + path + F("\nSD ERROR");
      ui->printErrorAndAbort(errorMessage);
    }
  }
  return file;
}

File openForReading(SdFat &sd, const String &path);
File openForReading(SdFat &sd, const char *path);
File openForCreating(SdFat &sd, const String &path);
File openForCreating(SdFat &sd, const char *path);

template <class pathType>
size_t readFile(FatFile &file, byte *buffer, size_t numBytes, const pathType &path) {
  int numBytesRead = file.read(buffer, numBytes);
  if (numBytesRead == -1) {
    String errorMessage = F("ERROR READING ");
    errorMessage.concat(path);
    errorMessage.concat(F("\nSD ERROR"));
    ui->printErrorAndAbort(errorMessage);
  }
  return numBytesRead;
}

template <class pathType>
void writeFile(FatFile &file, byte *buffer, size_t numBytes, const pathType &path) {
  int numBytesWritten = file.write(buffer, numBytes);
  if (numBytesWritten == -1) {
    String errorMessage = F("ERROR WRITING ");
    errorMessage.concat(path);
    errorMessage.concat(F("\nSD ERROR"));
    ui->printErrorAndAbort(errorMessage);
  }
  // numBytesWritten is always numBytes if it succeeds.
}

template <class sourcePathType, class destPathType>
void copyFile(FatFile &sourceFile, FatFile &destFile, byte *buffer, size_t bufSize, const sourcePathType &sourcePath, const destPathType &destPath) {
  size_t numBytesRead;
  while ((numBytesRead = readFile(sourceFile, buffer, bufSize, sourcePath)) > 0) {
    writeFile(destFile, buffer, numBytesRead, destPath);
  }
}

template <class pathType>
void closeFile(FatFile &file, const pathType &path) {
  bool success = file.close();
  if (!success) {
    String errorMessage = F("ERROR CLOSING ");
    errorMessage.concat(path);
    errorMessage.concat(F("\nSD ERROR"));
    ui->printErrorAndAbort(errorMessage);
  }
}

void mkdir(SdFat &sd, const char *path, bool createParentDirectories);
void mkdir(SdFat &sd, const String &path, bool createParentDirectories);

void chdir(SdFat &sd, const char *path);
void chdir(SdFat &sd, const String &path);

template <class pathType>
bool openNext(FatFile &dir, FatFile &file, const pathType &dirPath) {
  bool success = file.openNext(&dir);
  if (!success && dir.getError()) {
    String errorMessage = F("ERROR READING DIR ");
    errorMessage.concat(dirPath);
    errorMessage.concat(F("\nSD ERROR"));
    ui->printErrorAndAbort(errorMessage);
  }
  return success;
}

template <class pathType>
void getFileName(FatFile &file, char * buffer, size_t bufferSize, const pathType &dirPath) {
  bool success = file.getName(buffer, bufferSize);
  if (!success) {
    String errorMessage = F("ERROR GETTING NAME IN ");
    errorMessage.concat(dirPath);
    errorMessage.concat(F("\nSD ERROR"));
    ui->printErrorAndAbort(errorMessage);
  }
}

#endif