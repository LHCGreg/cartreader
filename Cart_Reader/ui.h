#ifndef cartreader_ui_h
#define cartreader_ui_h

#include "UserInterface.h"
#include "options.h"
#include "OLEDUserInterface.h"
#include "SerialUserInterface.h"
#include "MockUserInterface.h"

#ifdef UNIT_TEST
extern MockUserInterface ui;
#elif defined(enable_OLED)
extern OLEDUserInterface ui;
#else
extern SerialUserInterface ui;
#endif

#endif
