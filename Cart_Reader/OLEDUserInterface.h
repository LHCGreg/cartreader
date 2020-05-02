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
  boolean buttonLast = HIGH;         // buffered value of the button's previous state
  boolean DCwaiting = false;         // whether we're waiting for a double click (down)
  boolean DConUp = false;            // whether to register a double click on next release, or whether to wait and click
  boolean singleOK = true;           // whether it's OK to do a single click
  long downTime = -1;                // time the button was pressed down
  long upTime = -1;                  // time the button was released
  boolean ignoreUp = false;          // whether to ignore the button release because the click+hold was triggered
  boolean waitForUp = false;         // when held, whether to wait for the up event
  boolean holdEventPast = false;     // whether or not the hold event happened already
  boolean longholdEventPast = false; // whether or not the long hold event happened already
};

struct MenuWindow {
  uint8_t Start;
  uint8_t End;
};

class OLEDUserInterface final : public UserInterface {
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
  uint8_t askMultipleChoiceQuestion(const String &question, const String *answers, uint8_t numAnswers, uint8_t defaultChoice, bool wrapSelectionMovement);
  using UserInterface::askMultipleChoiceQuestion; // Make the overload in the base class available to callers that have an OLEDUserInterface and are not accessing via a UserInterface pointer
  String askQuestionWithPagedAnswers(const String &question, PagedAnswerSource &answerSource) override final;

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
  // Graphic I2C LCD
  // SSD1306 display connected to I2C (SDA, SCL pins)
  Adafruit_SSD1306 m_display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  static const uint8_t MAX_LINES = 8;
  static const uint8_t MAX_ANSWERS_DISPLAYABLE = MAX_LINES - 1;
  static const uint8_t ANSWERS_BEFORE_CURRENT_WHEN_IN_MIDDLE = 3;
  static const uint8_t ANSWERS_AFTER_CURRENT_WHEN_IN_MIDDLE = 3;
  static const uint8_t ANSWER_NEXT_PAGE = UINT8_MAX;
  static const uint8_t ANSWER_GO_BACK = UINT8_MAX - 1;
  static const int16_t CENTER = 64;
  static const int16_t N64_GRAPH_CENTER_X = 22 + 24; // midpoint x
  static const int16_t N64_GRAPH_CENTER_Y = 24;      // midpoint y

  MenuWindow getMenuWindow(uint8_t currentChoice, uint8_t numAnswers);
  void drawOrClearMoreIndicators(bool drawMoreAboveIndicator, bool drawMoreBelowIndicator);

  ButtonState buttonsState[2];

  // Read button state
  ButtonEvent waitForButtonEvent();
  ButtonEvent checkButtons();
  ButtonEvent checkButton(uint8_t button);
  ButtonEvent checkButton1();
  ButtonEvent checkButton2();

  void printAtPosition(const String &str, int x, int y);
};
#endif // #if defined(enable_OLED) && !defined(UNIT_TEST)

#endif
