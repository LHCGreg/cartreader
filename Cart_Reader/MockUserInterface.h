#ifndef cartreader_MockUserInterface_h
#define cartreader_MockUserInterface_h

#include "UserInterface.h"

#ifdef UNIT_TEST

#include <Arduino.h>

class MockUserInterface : public UserInterface {
  public:
  void initialize() override;
  uint32_t readNumber(uint8_t numDigits, uint32_t defaultValue, uint32_t maxValue, const String &prompt, const __FlashStringHelper *outOfRangeMessage) override;
  void displayMessage(const __FlashStringHelper *message) override;
  void waitForUserInput() override;
  bool supportsLargeMessages() override;
  void displayAbout(const String &aboutMessage) override;
  bool supportsSDInfoDisplay() override;
  void displaySDInfo(uint32_t capacityGB, uint8_t FATType) override;

  uint8_t askMultipleChoiceQuestion(const String &question, const String *answers, uint8_t numAnswers, uint8_t defaultChoice) override;
  uint8_t askMultipleChoiceQuestion(const String &question, const String *answers, uint8_t numAnswers, uint8_t defaultChoice, bool wrapSelectionMovement);
  String askQuestionWithPagedAnswers(const String &question, PagedAnswerSource &answerSource) override;

  void clearOutput() override;
  void flushOutput() override;
  void printMsg(const __FlashStringHelper *message) override;
  void printMsg(const char *message) override;
  void printMsg(const String &message) override;
  void printMsg(long unsigned int message) override;
  void printMsg(byte message, int outputFormat) override;
  void printlnMsg(const __FlashStringHelper *message) override;
  void printlnMsg(const char *message) override;
  void printlnMsg(const String &message) override;
  void printlnMsg(long unsigned int message) override;
  void printlnMsg(byte message, int outputFormat) override;
  [[noreturn]] void forceReset() override;
};

#endif // #ifdef UNIT_TEST

#endif
