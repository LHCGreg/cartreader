#include <Arduino.h>
#include "filebrowser.h"
#include "globals.h"
#include "ui.h"
#include "SDFolderPagedAnswerSource.h"

/******************************************
  Filebrowser Module
*****************************************/

String fileBrowser(const __FlashStringHelper* browserTitle) {
  String folderPath = F("/");
  while (true) {
    SDFolderPagedAnswerSource answerSource(sd, folderPath);
    String fileChoice = ui->askQuestionWithPagedAnswers(browserTitle, answerSource);

    // If user chose to go back, set folder to root
    if (fileChoice.length() == 0) {
      folderPath = F("/");
    }
    else if (fileChoice[0] == '/') {
      // append everything after the leading / that directories get returned with
      folderPath.concat(fileChoice.c_str() + 1);
    }
    else {
      return folderPath;
    }
  }
}
