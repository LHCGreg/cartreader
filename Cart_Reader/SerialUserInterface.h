#ifndef cartreader_SerialUserInterface_h
#define cartreader_SerialUserInterface_h

#include "UserInterface.h"
#include "options.h"
#include <Arduino.h>

#ifndef enable_OLED

class SerialUserInterface : public UserInterface {
  public:
  void initialize() override final;
  uint32_t readNumber(uint8_t numDigits, uint32_t defaultValue, uint32_t maxValue, const String &prompt, const __FlashStringHelper *outOfRangeMessage) override final;
  void displayMessage(const __FlashStringHelper *message) override final;
  void waitForUserInput() override final;
  bool supportsLargeMessages() override final;
  void displayAbout(const String &aboutMessage) override final;
  bool supportsSDInfoDisplay() override final;
  void displaySDInfo(uint32_t capacityGB, uint8_t FATType) override final;
  void updateN64ButtonTest(const String &currentButton, int8_t stickX, int8_t stickY) override final;
  bool supportsN64RangeTest() override final;
  void updateN64RangeTest(int8_t stickX, int8_t stickY, int8_t mode) override final;
  bool supportsN64SkippingTest() override final;
  void updateN64SkippingTest(int8_t prevStickX, int8_t stickX) override final;
  void printN64BenchmarkPrompt(uint8_t testNumber) override final;
  void printN64BenchmarkResults(const String &anastick, int8_t upX, int8_t upY, int8_t upRightX, int8_t upRightY,
                                int8_t rightX, int8_t rightY, int8_t downRightX, int8_t downRightY,
                                int8_t downX, int8_t downY, int8_t downLeftX, int8_t downLeftY,
                                int8_t leftX, int8_t leftY, int8_t upLeftX, int8_t upLeftY) override final;
  uint8_t askMultipleChoiceQuestion(const String &question, const String *answers, uint8_t numAnswers, uint8_t defaultChoice) override final;
  String askQuestionWithPagedAnswers(const String &question, PagedAnswerSource &answerSource) override final;
  using UserInterface::askMultipleChoiceQuestion; // Make the overload in the base class available to callers that have a SerialUserInterface and are not accessing via a UserInterface pointer

  void clearOutput() override final;
  void flushOutput() override final;
  void printMsg(const __FlashStringHelper *message) override final;
  void printMsg(const char *message) override final;
  void printMsg(const String &message) override final;
  void printMsg(long unsigned int message) override final;
  void printMsg(byte message, int outputFormat) override final;
  void printlnMsg(const __FlashStringHelper *message) override final;
  void printlnMsg(const char *message) override final;
  void printlnMsg(const String &message) override final;
  void printlnMsg(long unsigned int message) override final;
  void printlnMsg(byte message, int outputFormat) override final;
  [[noreturn]] void forceReset() override final;

  private:
  static const unsigned long N64_UPDATE_FREQUENCY_MILLIS = 500;
  unsigned long m_lastN64UpdateMillis = 0;
  uint8_t m_lastN64BenchmarkPromptDisplayed = 99;

  byte readByte();
  String readCommand();
};

#endif

#endif