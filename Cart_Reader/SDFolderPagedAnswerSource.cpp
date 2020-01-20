#include "SDFolderPagedAnswerSource.h"
#include "SD.h"

SDFolderPagedAnswerSource::SDFolderPagedAnswerSource(SdFat &sd, const String &folderPath)
  : m_folderPath(folderPath) {

  m_dir = openForReading(sd, m_folderPath);
}

String SDFolderPagedAnswerSource::GetNextAnswer() {
  char fileNameBuffer[FILENAME_BUFFER_LENGTH];
  FatFile file;
  while (openNext(m_dir, file, m_folderPath)) {
    if (file.isHidden()) {
      closeFile(file, F("[file in dir]"));
    }
    else {
      getFileName(file, fileNameBuffer, FILENAME_BUFFER_LENGTH, m_folderPath);
      String fileName(fileNameBuffer);
      bool isDir = file.isDir();
      closeFile(file, F("[file in dir]"));

      // only return the file if its name length is not greater than the max. Otherwise skip it.
      if (fileName.length() <= MAX_FILENAME_LENGTH) {
        if (isDir) {
          fileName = String(F("/")) + fileName;
        }
        return fileName;
      }
    }
  }

  // If we reach the end, return empty string
  return F("");
}

SDFolderPagedAnswerSource::~SDFolderPagedAnswerSource() {
  closeFile(m_dir, m_folderPath);
}
