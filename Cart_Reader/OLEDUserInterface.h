#ifndef cartreader_OLEDUserInterface_h
#define cartreader_OLEDUserInterface_h

#include "options.h"

#if defined(enable_OLED) && !defined(UNIT_TEST)

#include "UserInterface.h"
#include <Adafruit_SSD1306.h>
#include <Wire.h>

const uint8_t SCREEN_WIDTH = 128; // OLED display width, in pixels
const uint8_t SCREEN_HEIGHT = 64; // OLED display height, in pixels
const int8_t OLED_RESET = -1;     // Reset pin # (or -1 if sharing Arduino reset pin)

enum class ButtonEvent : byte {
  NoEvent = 0,
  Click = 1,
  DoubleClick = 2,
  Hold = 3,
  LongHold = 4,
};

struct ButtonState {
  boolean buttonLast = HIGH; // buffered value of the button's previous state
  boolean DCwaiting = false; // whether we're waiting for a double click (down)
  boolean DConUp = false; // whether to register a double click on next release, or whether to wait and click
  boolean singleOK = true; // whether it's OK to do a single click
  long downTime = -1; // time the button was pressed down
  long upTime = -1; // time the button was released
  boolean ignoreUp = false; // whether to ignore the button release because the click+hold was triggered
  boolean waitForUp = false; // when held, whether to wait for the up event
  boolean holdEventPast = false; // whether or not the hold event happened already
  boolean longholdEventPast = false; // whether or not the long hold event happened already
};

struct MenuWindow {
  uint8_t Start;
  uint8_t End;
};

class OLEDUserInterface : public UserInterface {
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

  private:
  // Graphic I2C LCD
  // SSD1306 display connected to I2C (SDA, SCL pins)
  Adafruit_SSD1306 m_display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  static const uint8_t MAX_LINES = 8;
  static const uint8_t MAX_ANSWERS_DISPLAYABLE = MAX_LINES - 1;
  static const uint8_t ANSWERS_BEFORE_CURRENT_WHEN_IN_MIDDLE = 3;
  static const uint8_t ANSWERS_AFTER_CURRENT_WHEN_IN_MIDDLE = 3;
  static const uint8_t ANSWER_NEXT_PAGE = UINT8_MAX;
  static const uint8_t ANSWER_GO_BACK = UINT8_MAX - 1;

  MenuWindow getMenuWindow(uint8_t currentChoice, uint8_t numAnswers);
  void drawOrClearMoreIndicators(bool drawMoreAboveIndicator, bool drawMoreBelowIndicator);

  ButtonState buttonsState[2];

  // Read button state
  ButtonEvent waitForButtonEvent();
  ButtonEvent checkButtons();
  ButtonEvent checkButton(uint8_t button);
  ButtonEvent checkButton1();
  ButtonEvent checkButton2();
};
#endif // #if defined(enable_OLED) && !defined(UNIT_TEST)

#endif
