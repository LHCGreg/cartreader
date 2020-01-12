#ifndef cartreader_filebrowser_h
#define cartreader_filebrowser_h

#include <Arduino.h>

extern boolean filebrowse;
extern byte currPage;
extern byte lastPage;
extern byte numPages;
extern boolean root;

void fileBrowser(const __FlashStringHelper* browserTitle);

#endif
