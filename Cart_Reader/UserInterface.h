#ifndef cartreader_UserInterface_h
#define cartreader_UserInterface_h

#include "PagedAnswerSource.h"
#include "RGB_LED.h"
#include "globals.h"
#include "utils.h"
#include <Arduino.h>
#include <stdint.h>

class UserInterface {
  public:
  virtual void initialize() = 0;
  virtual uint32_t readNumber(uint8_t numDigits, uint32_t defaultValue, uint32_t maxValue, const String &prompt, const __FlashStringHelper *outOfRangeMessage) = 0;
  virtual void displayMessage(const __FlashStringHelper *message) = 0;
  virtual void waitForUserInput() = 0;
  virtual bool supportsLargeMessages() = 0;
  virtual void displayAbout(const String &aboutMessage) = 0;
  virtual bool supportsSDInfoDisplay() = 0;
  virtual void displaySDInfo(uint32_t capacityGB, uint8_t FATType) = 0;
  virtual void updateN64ButtonTest(const String &currentButton, int8_t stickX, int8_t stickY) = 0;
  virtual bool supportsN64RangeTest() = 0;
  virtual void updateN64RangeTest(int8_t stickX, int8_t stickY, int8_t mode) = 0;
  virtual bool supportsN64SkippingTest() = 0;
  virtual void updateN64SkippingTest(int8_t prevStickX, int8_t stickX) = 0;
  virtual void printN64BenchmarkPrompt(uint8_t testNumber) = 0;
  virtual void printN64BenchmarkResults(const String &anastick, int8_t upX, int8_t upY, int8_t upRightX, int8_t upRightY,
                                        int8_t rightX, int8_t rightY, int8_t downRightX, int8_t downRightY,
                                        int8_t downX, int8_t downY, int8_t downLeftX, int8_t downLeftY,
                                        int8_t leftX, int8_t leftY, int8_t upLeftX, int8_t upLeftY) = 0;

  virtual uint8_t askMultipleChoiceQuestion(const String &question, const String *answers, uint8_t numAnswers, uint8_t defaultChoice) = 0;
  const __FlashStringHelper *askMultipleChoiceQuestion(const String &question, const __FlashStringHelper **answers, uint8_t numAnswers, const __FlashStringHelper *defaultChoice);
  virtual String askQuestionWithPagedAnswers(const String &question, PagedAnswerSource &answerSource) = 0;

  void drawProgressBar(uint32_t processedSize, uint32_t totalSize);

  virtual void clearOutput() = 0;
  virtual void flushOutput() = 0;
  virtual void printMsg(const __FlashStringHelper *message) = 0;
  virtual void printMsg(const char *message) = 0;
  virtual void printMsg(const String &message) = 0;
  virtual void printMsg(long unsigned int message) = 0;
  virtual void printMsg(byte message, int outputFormat) = 0;
  virtual void printlnMsg(const __FlashStringHelper *message) = 0;
  virtual void printlnMsg(const char *message) = 0;
  virtual void printlnMsg(const String &message) = 0;
  virtual void printlnMsg(long unsigned int message) = 0;
  virtual void printlnMsg(byte message, int outputFormat) = 0;

  [[noreturn]] virtual void forceReset() = 0;

  template <class printableType>
  void printError(const printableType &errorMessage) {
    errorLvl = 1;
    rgb.setColor(255, 0, 0);
    printlnMsg(errorMessage);
    flushOutput();
  }

  template <class printableType>
  [[noreturn]] void printErrorAndAbort(const printableType &errorMessage, bool clearOutputBeforePrinting) {
    errorLvl = 1;
    rgb.setColor(255, 0, 0);
    
    if (clearOutputBeforePrinting) {
      clearOutput();
    }

    printlnMsg(errorMessage);
    flushOutput();
    forceReset();
    resetArduino(); // The compiler doesn't know that forceReset is noreturn for all derived types.
  }
};

#endif