#include <Arduino.h>
#include "SDFolderPagedAnswerSource.h"
#include "SD.h"

SDFolderPagedAnswerSource::SDFolderPagedAnswerSource(const String &folderPath)
    : m_folderPath(folderPath), m_dir(SafeSDFile::openForReading(folderPath)) {
  ;
}

String SDFolderPagedAnswerSource::GetNextAnswer() {
  char fileNameBuffer[FILENAME_BUFFER_LENGTH];
  SafeSDFile dirEntry;
  while ((dirEntry = m_dir.readNextDirectoryEntry()).isOpen()) {
    // Ignore if hidden
    if (dirEntry.isHidden()) {
      continue;
    }
    else {
      dirEntry.getName(fileNameBuffer, FILENAME_BUFFER_LENGTH);
      String fileName(fileNameBuffer);

      // only return the file if its name length is not greater than the max. Otherwise skip it.
      if (fileName.length() <= MAX_FILENAME_LENGTH) {
        if (dirEntry.isDir()) {
          fileName = String("/") + fileName;
        }
        return fileName;
      }
    }
  }

  // If we reach the end, return empty string
  return String("");
}

SDFolderPagedAnswerSource::~SDFolderPagedAnswerSource() {
  ; // m_dir's destructor automatically closes the underlying file, no need to do it here
}
