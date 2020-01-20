#ifndef cartreader_SDFolderPagedAnswerSource_h
#define cartreader_SDFolderPagedAnswerSource_h

#include "PagedAnswerSource.h"
#include "SdFat.h"

class SDFolderPagedAnswerSource : public PagedAnswerSource {
  public:
  SDFolderPagedAnswerSource(SdFat &sd, const String &folderPath);

  String GetNextAnswer();

  virtual ~SDFolderPagedAnswerSource();

  private:
  const String m_folderPath;
  File m_dir;

  static const uint8_t MAX_FILENAME_LENGTH = 32;
  static const uint8_t FILENAME_BUFFER_LENGTH = MAX_FILENAME_LENGTH + 2;

  // Disallow copying. We keep an open file and close it in the destructor.
  SDFolderPagedAnswerSource(const SDFolderPagedAnswerSource &) = delete;
  SDFolderPagedAnswerSource &operator=(const SDFolderPagedAnswerSource &) = delete;
};

#endif
