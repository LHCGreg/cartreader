#ifndef cartreader_SD_h
#define cartreader_SD_h

#include <Arduino.h>
#include <SdFat.h>
#include "ui.h"

void initializeSD(uint8_t chipSelectPin, const SPISettings &speed);

// Does error checking on all operations. If there's an error, displays the error and halts the program.
class SafeSDFile {
  public:
  SafeSDFile();
  static SafeSDFile openForReading(const String &path);
  static SafeSDFile openForCreating(const String &path);
  static SafeSDFile openForWriting(const String &path);
  static SafeSDFile getVwd();

  SafeSDFile(SafeSDFile &&other); // move constructor, allowing this class to be returned from functions
  SafeSDFile &operator=(SafeSDFile &&other); // move assignment
  size_t read(byte *buffer, size_t numBytes);

  void readOrDie(byte *buffer, size_t numBytes);

  template <class printable>
  void readOrDie(byte *buffer, size_t numBytes, const printable &errorMessage) {
    size_t numBytesRead = read(buffer, numBytes);
    if (numBytesRead != numBytes) {
      ui.printErrorAndAbort(errorMessage, true);
    }
  }

  uint8_t readByteOrDie();

  template <class printable>
  uint8_t readByteOrDie(const printable &errorMessage) {
    uint8_t byteRead;
    readOrDie(&byteRead, 1);
    return byteRead;
  }

  void write(byte *buffer, size_t numBytes);
  void writeByte(byte b);
  void seekCur(int32_t offset);
  void seekSet(uint32_t pos);

  // Check returned file isOpen(). If false, no more entries in the directory.
  // Only call this function on directories.
  SafeSDFile readNextDirectoryEntry();

  void getName(char *outName, size_t size);
  uint32_t fileSize();
  uint32_t bytesAvailable();
  bool isOpen();
  bool isHidden();
  bool isDir();
  bool isFile();
  void rename(const String &newPath);
  void remove();
  void close();
  ~SafeSDFile();

  operator bool() {
    return isOpen();
  }

  private:
  FatFile m_file;
  String m_path; // hold onto the path for use in error messages
  bool m_dontClose;

  SafeSDFile(FatFile &file, const String &path);
  SafeSDFile(SdFat &sd, const String &path, oflag_t mode);

  // Disallow copying. This class manages the file handle, closing it in the destructor.
  // If copies of an object were made, one object could close the handle held by another object.
  SafeSDFile(const SafeSDFile &) = delete;
  SafeSDFile& operator=(const SafeSDFile &) = delete;
};

void copyFile(SafeSDFile &sourceFile, SafeSDFile &destFile, byte *buffer, size_t bufSize);

void mkdir(const char *path, bool createParentDirectories);
void mkdir(const String &path, bool createParentDirectories);

void chdirToRoot();
void chdir(const char *path);
void chdir(const String &path);

bool fileExists(const char *path);
bool fileExists(const String &path);

#endif
