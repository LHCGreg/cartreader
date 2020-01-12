#ifndef cartreader_filebrowser_h
#define cartreader_filebrowser_h

#include <Arduino.h>

#define FILENAME_LENGTH 32
#define FILEPATH_LENGTH 64
#define FILEOPTS_LENGTH 20

extern boolean filebrowse;
extern char fileName[FILENAME_LENGTH];
extern char filePath[FILEPATH_LENGTH];
extern byte currPage;
extern byte lastPage;
extern byte numPages;
extern boolean root;

void fileBrowser(const __FlashStringHelper* browserTitle);

#endif
