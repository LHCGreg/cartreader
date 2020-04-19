#ifndef cartreader_PagedAnswerSource_h
#define cartreader_PagedAnswerSource_h

#include <Arduino.h>

class PagedAnswerSource {
  public:
  virtual String GetNextAnswer() = 0;
  virtual ~PagedAnswerSource();
};

#endif
