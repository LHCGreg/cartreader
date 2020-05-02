#include "MockUserInterface.h"
#include "OLEDUserInterface.h"
#include "SerialUserInterface.h"
#include "UserInterface.h"
#include "options.h"

#ifdef UNIT_TEST
MockUserInterface ui;
#elif defined(enable_OLED)
OLEDUserInterface ui;
#else
SerialUserInterface ui;
#endif
