#include "SerialUserInterface.h"
#include "RGB_LED.h"
#include "filebrowser.h"
#include "options.h"
#include "utils.h"
#include <Arduino.h>

#ifndef enable_OLED

void SerialUserInterface::initialize() {
  // Serial Begin
  Serial.begin(9600);
  Serial.println(F("Cartridge Reader"));
  Serial.println(F("2019 sanni"));
  Serial.println("");
  // LED Error
  rgb.setColor(0, 0, 255);
}

byte SerialUserInterface::readByte() {
  while (Serial.available() == 0) {
    ;
  }
  return (byte) Serial.read();
}

String SerialUserInterface::readCommand() {
  while (Serial.available() == 0) {
    ;
  }
  String command = Serial.readStringUntil('\n');
  if (command.length() > 0 && command[command.length() - 1] == '\r') {
    command.remove(command.length() - 1);
  }
  return command;
}

void SerialUserInterface::displayMessage(const __FlashStringHelper *message) {
  Serial.println(message);
}

bool SerialUserInterface::supportsLargeMessages() {
  return true;
}

void SerialUserInterface::waitForUserInput() {
  readByte();
}

void SerialUserInterface::displayAbout(const String &aboutMessage) {
  printlnMsg(aboutMessage);
  printlnMsg(F("Press Button"));
  waitForUserInput();
  resetArduino();
}

bool SerialUserInterface::supportsSDInfoDisplay() {
  return true;
}

void SerialUserInterface::displaySDInfo(uint32_t capacityGB, uint8_t FATType) {
  Serial.print(F("SD Card: "));
  Serial.print(capacityGB);
  Serial.print(F("GB FAT"));
  Serial.println(FATType);
}

uint32_t SerialUserInterface::readNumber(uint8_t numDigits, uint32_t defaultValue, uint32_t maxValue, const String &prompt, const __FlashStringHelper *outOfRangeMessage) {
  while (true) {
    Serial.println(prompt);
    String numberString = readCommand();
    Serial.println(numberString);

    int8_t errorCode;
    uint32_t number = stringToNumber(numberString.c_str(), errorCode);
    if (errorCode != 0) {
      Serial.println(F("Not a number."));
    }
    else if (number > maxValue) {
      Serial.println(outOfRangeMessage);
    }
    else {
      return number;
    }
  }
}

uint8_t SerialUserInterface::askMultipleChoiceQuestion(const String &question, const String *answers, uint8_t numAnswers, uint8_t defaultChoice) {
  // Print menu to serial monitor
  Serial.println(question);
  for (uint8_t answerIndex = 0; answerIndex < numAnswers; answerIndex++) {
    Serial.print(answerIndex);
    Serial.print(F(")"));
    Serial.println(answers[answerIndex]);
  }

  while (true) {
    // Wait for user input
    Serial.println("");
    Serial.println(F("Enter a selection by typing a number followed by a newline: _ "));

    String command = readCommand();

    int8_t errorCode;
    uint32_t number = stringToNumber(command.c_str(), errorCode);
    if (errorCode == 0) {
      if (number < numAnswers) {
        return (uint8_t) number;
      }
      else {
        String message = String(F("Number must be 0-")) + String(numAnswers - 1) + String(F("."));
        Serial.println(message);
      }
    }
    else {
      Serial.println(F("Not a number."));
    }
  }
}

String SerialUserInterface::askQuestionWithPagedAnswers(const String &question, PagedAnswerSource &answerSource) {
  Serial.println(question);

  while (true) {
    const uint8_t pageSize = 15;
    String answers[pageSize];
    uint8_t answerIndex = 0;
    for (answerIndex = 0; answerIndex < pageSize; answerIndex++) {
      String answer = answerSource.GetNextAnswer();
      if (answer.length() == 0) {
        break;
      }

      answers[answerIndex] = answer;

      Serial.print(answerIndex);
      Serial.print(F(")"));
      Serial.println(answer);
    }

    Serial.println(F("Enter a selection by typing a number followed by a newline."));
    Serial.println(F("Go to next page with 'd' (down)."));
    Serial.println(F("Go back with 'b'."));

    while (true) {
      String command = readCommand();
      if (command == F("d")) {
        // break out of input checking loop, continue on to next page
        break;
      }
      else if (command == F("b")) {
        return String(F(""));
      }
      else {
        int8_t errorCode;
        uint32_t number = stringToNumber(command.c_str(), errorCode);
        if (errorCode == 0) {
          if (number < answerIndex) {
            return answers[number];
          }
          else {
            if (answerIndex > 0) {
              String message = String(F("Number must be 0-")) + String(answerIndex - 1) + String(F("."));
              Serial.println(message);
            }
            else {
              Serial.println(F("End of possible selections reached. Enter 'b' to go back."));
            }
          }
        }
        else {
          Serial.println(F("Not a number."));
        }
      }
    }
  }
}

void SerialUserInterface::clearOutput() {
  ; // nothing to do here
}

void SerialUserInterface::flushOutput() {
  delay(100);
}

void SerialUserInterface::printMsg(const __FlashStringHelper *message) {
  Serial.print(message);
}

void SerialUserInterface::printMsg(const char *message) {
  Serial.print(message);
}

void SerialUserInterface::printMsg(const String &message) {
  Serial.print(message);
}

void SerialUserInterface::printMsg(long unsigned int message) {
  Serial.print(message);
}

void SerialUserInterface::printMsg(byte message, int outputFormat) {
  Serial.print(message, outputFormat);
}

void SerialUserInterface::printlnMsg(const __FlashStringHelper *message) {
  Serial.println(message);
}

void SerialUserInterface::printlnMsg(const char *message) {
  Serial.println(message);
}

void SerialUserInterface::printlnMsg(const String &message) {
  Serial.println(message);
}

void SerialUserInterface::printlnMsg(long unsigned int message) {
  Serial.println(message);
}

void SerialUserInterface::printlnMsg(byte message, int outputFormat) {
  Serial.println(message, outputFormat);
}

void SerialUserInterface::forceReset() {
  printlnMsg(F("Fatal Error, please reset"));
  while (1) {
    ;
  }
}

#endif
