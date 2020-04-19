#include <Arduino.h>
#include "filebrowser.h"
#include "menu.h"
#include "globals.h"
#include "SD.h"

/******************************************
  Filebrowser Module
*****************************************/

boolean filebrowse = 0;
char fileName[FILENAME_LENGTH];
char filePath[FILEPATH_LENGTH];
byte currPage;
byte lastPage;
byte numPages;
boolean root = 0;
char fileOptions[30][FILEOPTS_LENGTH];

void fileBrowser(const __FlashStringHelper* browserTitle) {
  char fileNames[30][FILENAME_LENGTH];
  int currFile;
  filebrowse = 1;

  // Empty filePath string
  filePath[0] = '\0';

  // Temporary char array for filename
  char nameStr[FILENAME_LENGTH];

browserstart:

  // Print title
  println_Msg(browserTitle);

  // Set currFile back to 0
  currFile = 0;
  currPage = 1;
  lastPage = 1;

  // Read in File as long as there are files
  SafeSDFile vwd = SafeSDFile::getVwd();
  SafeSDFile dirEntry;
  while ((dirEntry = vwd.readNextDirectoryEntry()).isOpen()) {

    // Get name of file
    dirEntry.getName(nameStr, FILENAME_LENGTH);

    // Ignore if hidden
    if (dirEntry.isHidden()) {
    }
    // Indicate a directory.
    else if (dirEntry.isDir()) {
      // Copy full dirname into fileNames
      snprintf(fileNames[currFile], FILENAME_LENGTH, "%s%s", "/", nameStr);
      // Copy short string into fileOptions
      snprintf(fileOptions[currFile], FILEOPTS_LENGTH, "%s%s", "/", nameStr);
      currFile++;
    }
    // It's just a file
    else if (dirEntry.isFile()) {
      // Copy full filename into fileNames
      snprintf(fileNames[currFile], FILENAME_LENGTH, "%s", nameStr);
      // Copy short string into fileOptions
      snprintf(fileOptions[currFile], FILEOPTS_LENGTH, "%s", nameStr);
      currFile++;
    }
    dirEntry.close();
  }

  // "Calculate number of needed pages"
  if (currFile < 8)
    numPages = 1;
  else if (currFile < 15)
    numPages = 2;
  else if (currFile < 22)
    numPages = 3;
  else if (currFile < 29)
    numPages = 4;
  else if (currFile < 36)
    numPages = 5;

  // Fill the array "answers" with 7 options to choose from in the file browser
  char answers[7][20];

page:

  // If there are less than 7 entries, set count to that number so no empty options appear
  byte count;
  if (currFile < 8)
    count = currFile;
  else if (currPage == 1)
    count = 7;
  else if (currFile < 15)
    count = currFile - 7;
  else if (currPage == 2)
    count = 7;
  else if (currFile < 22)
    count = currFile - 14;
  else if (currPage == 3)
    count = 7;
  else if (currFile < 29)
    count = currFile - 21;
  else {
    display_Clear();

    println_Msg(F("Too many files"));
    display_Update();
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
  }

  for (byte i = 0; i < count; i++ ) {
    // Copy short string into fileOptions
    snprintf( answers[i], FILEOPTS_LENGTH, "%s", fileOptions[ ((currPage - 1) * 7 + i)] );
  }

  // Create menu with title and 1-7 options to choose from
  unsigned char answer = question_box(browserTitle, answers, count, 0);

  // Check if the page has been switched
  if (currPage != lastPage) {
    lastPage = currPage;
    goto page;
  }

  // Check if we are supposed to go back to the root dir
  if (root) {
    // Empty filePath string
    filePath[0] = '\0';
    // Rewind filesystem
    //sd.vwd()->rewind();
    // Change working dir to root
    chdir("/");
    // Start again
    root = 0;
    goto browserstart;
  }

  // wait for user choice to come back from the question box menu
  switch (answer)
  {
    case 0:
      strncpy(fileName, fileNames[0 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 1:
      strncpy(fileName, fileNames[1 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 2:
      strncpy(fileName, fileNames[2 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 3:
      strncpy(fileName, fileNames[3 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 4:
      strncpy(fileName, fileNames[4 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 5:
      strncpy(fileName, fileNames[5 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 6:
      strncpy(fileName, fileNames[6 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 7:
      // File import
      break;
  }

  // Add directory to our filepath if we just entered a new directory
  if (fileName[0] == '/') {
    // add dirname to path
    strcat(filePath, fileName);
    // Remove / from dir name
    char* dirName = fileName + 1;
    // Change working dir
    chdir(dirName);
    // Start browser in new directory again
    goto browserstart;
  }
  else {
    // Afer everything is done change SD working directory back to root
    chdir("/");
  }
  filebrowse = 0;
}
