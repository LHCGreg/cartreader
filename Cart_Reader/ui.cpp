#include "MockUserInterface.h"
#include "OLEDUserInterface.h"
#include "SerialUserInterface.h"
#include "UserInterface.h"
#include "options.h"

#ifdef UNIT_TEST
MockUserInterface mockInterface;
UserInterface *ui = &mockInterface;
#elif defined(enable_OLED)
OLEDUserInterface oledInterface;
UserInterface *ui = &oledInterface;
#else
SerialUserInterface serialInterface;
UserInterface *ui = &serialInterface;
#endif
