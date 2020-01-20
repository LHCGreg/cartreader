#include "SD.h"
#include "globals.h"
#include "options.h"

// SD Card (Pin 50 = MISO, Pin 51 = MOSI, Pin 52 = SCK, Pin 53 = SS)
const uint8_t chipSelectPin = 53;

void initializeSD() {
  if (!sd.begin(chipSelectPin, sdSpeed)) {
    ui->clearOutput();
    ui->printErrorAndAbort(F("SD Error"));
  }

  // Print SD Info
  if (ui->supportsSDInfoDisplay()) {
    uint32_t capacityGB = sd.card()->cardSize() * 512E-9;
    uint8_t FATType = sd.vol()->fatType();
    ui->displaySDInfo(capacityGB, FATType);
  }
}

File openForReading(SdFat &sd, const String &path) {
  return open(sd, path, FILE_READ);
}

File openForReading(SdFat &sd, const char *path) {
  return open(sd, path, FILE_READ);
}

File openForCreating(SdFat &sd, const String &path) {
  return open(sd, path, O_RDWR | O_CREAT | O_EXCL);
}

File openForCreating(SdFat &sd, const char *path) {
  return open(sd, path, O_RDWR | O_CREAT | O_EXCL);
}

void mkdir(SdFat &sd, const char *path, bool createParentDirectories) {
  bool success = sd.mkdir(path, createParentDirectories);
  if (!success) {
    String errorMessage = F("ERROR CREATING ");
    errorMessage.concat(path);
    errorMessage.concat(F("\nSD ERROR"));
    ui->printErrorAndAbort(errorMessage);
  }
}

void mkdir(SdFat &sd, const String &path, bool createParentDirectories) {
  mkdir(sd, path.c_str(), createParentDirectories);
}

void chdir(SdFat &sd, const char *path) {
  bool success = sd.chdir(path);
  if (!success) {
    String errorMessage = F("CHDIR ");
    errorMessage.concat(path);
    errorMessage.concat(F(" FAILED\nSD ERROR"));
    ui->printErrorAndAbort(errorMessage);
  }
}

void chdir(SdFat &sd, const String &path) {
  chdir(sd, path.c_str());
}
