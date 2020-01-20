#include "MockUserInterface.h"

#ifdef UNIT_TEST

void MockUserInterface::initialize() {
  ;
}

uint32_t MockUserInterface::readNumber(uint8_t numDigits, uint32_t defaultValue, uint32_t maxValue, const String &prompt, const __FlashStringHelper *outOfRangeMessage) {
  return 0;
}

void MockUserInterface::displayMessage(const __FlashStringHelper *message) {
  ;
}

void MockUserInterface::waitForUserInput() {
  ;
}

bool MockUserInterface::supportsLargeMessages() {
  return true;
}

void MockUserInterface::displayAbout(const String &aboutMessage) {
  ;
}

bool MockUserInterface::supportsSDInfoDisplay() {
  return true;
}

void MockUserInterface::displaySDInfo(uint32_t capacityGB, uint8_t FATType) {
  ;
}

uint8_t MockUserInterface::askMultipleChoiceQuestion(const String &question, const String *answers, uint8_t numAnswers, uint8_t defaultChoice) {
  return 0;
}

String MockUserInterface::askQuestionWithPagedAnswers(const String &question, PagedAnswerSource &answerSource) {
  return "";
}

void MockUserInterface::clearOutput() {
  ;
}

void MockUserInterface::flushOutput() {
  ;
}

void MockUserInterface::printMsg(const __FlashStringHelper *message) {
  ;
}

void MockUserInterface::printMsg(const char *message) {
  ;
}

void MockUserInterface::printMsg(const String &message) {
  ;
}

void MockUserInterface::printMsg(long unsigned int message) {
  ;
}

void MockUserInterface::printMsg(byte message, int outputFormat) {
  ;
}

void MockUserInterface::printlnMsg(const __FlashStringHelper *message) {
  ;
}

void MockUserInterface::printlnMsg(const char *message) {

}

void MockUserInterface::printlnMsg(const String &message) {
  ;
}

void MockUserInterface::printlnMsg(long unsigned int message) {
  ;
}

void MockUserInterface::printlnMsg(byte message, int outputFormat) {
  ;
}

void MockUserInterface::forceReset() {
  resetArduino();
}

#endif
