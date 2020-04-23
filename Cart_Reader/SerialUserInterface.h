#ifndef cartreader_SerialUserInterface_h
#define cartreader_SerialUserInterface_h

#include "UserInterface.h"
#include "options.h"
#include <Arduino.h>

#ifndef enable_OLED

class SerialUserInterface : public UserInterface {
  public:
  void initialize() override;
  uint32_t readNumber(uint8_t numDigits, uint32_t defaultValue, uint32_t maxValue, const String &prompt, const __FlashStringHelper *outOfRangeMessage) override;
  void displayMessage(const __FlashStringHelper *message) override;
  void waitForUserInput() override;
  bool supportsLargeMessages() override;
  void displayAbout(const String &aboutMessage);
  bool supportsSDInfoDisplay() override;
  void displaySDInfo(uint32_t capacityGB, uint8_t FATType) override;
  void updateN64ButtonTest(const String &currentButton, char stickX, char stickY) override;
  uint8_t askMultipleChoiceQuestion(const String &question, const String *answers, uint8_t numAnswers, uint8_t defaultChoice) override;
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

  private:
  static const unsigned long N64_UPDATE_FREQUENCY_MILLIS = 500;
  unsigned long m_lastN64UpdateMillis = 0;

  byte readByte();
  String readCommand();
};

#endif

#endif