#include "UserInterface.h"

#include <stdint.h>
#include <Arduino.h>
#include "globals.h"
#include "RGB_LED.h"

const __FlashStringHelper* UserInterface::askMultipleChoiceQuestion(const String &question, const __FlashStringHelper **answers, uint8_t numAnswers, const __FlashStringHelper *defaultChoice) {
  // get index of default choice
  uint8_t defaultChoiceIndex = 0;
  for (uint8_t answerIndex = 0; answerIndex < numAnswers; answerIndex++) {
    if(defaultChoice == answers[answerIndex]) {
      defaultChoiceIndex = answerIndex;
      break;
    }
  }

  // convert answers to strings
  String answerStrings[numAnswers];
  for (uint8_t answerIndex = 0; answerIndex < numAnswers; answerIndex++) {
    answerStrings[answerIndex] = String(answers[answerIndex]);
  }

  uint8_t answerIndex = askMultipleChoiceQuestion(question, answerStrings, numAnswers, defaultChoiceIndex);
  return answers[answerIndex];
}

void UserInterface::drawProgressBar(uint32_t processedSize, uint32_t totalSize) {
  uint8_t current, i;
  static uint8_t previous;
  uint8_t steps = 20;

  //Find progressbar length and draw if processed size is not 0
  if (processedSize == 0) {
    previous = 0;
    printMsg(F("["));
    flushOutput();
    return;
  }

  // Progress bar
  current = (processedSize >= totalSize) ? steps : processedSize / (totalSize / steps);

  //Draw "*" if needed
  if (current > previous) {
    for (i = previous; i < current; i++) {
      // steps are 20, so 20 - 1 = 19.
      if (i == (19)) {
        //If end of progress bar, finish progress bar by drawing "]"
        printMsg(F("]"));
      }
      else {
        printMsg(F("*"));
      }
    }
    //update previous "*" status
    previous = current;
    //Update display
    flushOutput();
  }
}
