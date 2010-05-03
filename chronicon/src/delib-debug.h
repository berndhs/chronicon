#ifndef DELIB_DEBUG_H
#define DELIB_DEBUG_H

//
//  Copyright (C) 2010 - Bernd H Stramm 
//
// This file is distributed under the terms of 
// the GNU General Public License version 3 
//
// This software is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty 
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//
//


// 
// Usage:
// 
// - Static Log:
//
//    Make sure DELIBERATE_DEBUG is set to a non-zero value , e.g.
//    #define DELIBERATE_DEBUG=1
//
//    The static log will capture all messages that go to qDebug().
//
//    When you want to start seeing messages in the log (for example,
//    at the beginning of your program execution), call
//      StartDebugLog ();
//
//    When you cant to stop logging, simply call
//      StopDebugLog();
//
// - Dynamic Logging
//
//   Make sure DELIBERATE_DEBUG is set to a non-zeo value, e.g.
//   #define DELIBERATE_DEBUG=1
//
//   You can allocate as many DebugLog objects as you like, e.g.
//     DebugLog leftLog;
//     DebugLog rightLog;
//     DebugLog middleLog (this);  // where "this" points to a QWidget
//
//   These DebugLog objects will *not* capture messages from qDebug(),
//   only from calls to the Log(char*) function.
//
//   You can turn the individual logs on and off any time you want,
//   independent of each other:
//     leftLog.StartLogging ();
//     rightLog.StopLogging ();
//

#include <qapplication.h>
#include <iostream>
#include "ui_DebugLog.h"

#include <QDebug>
#include <QCloseEvent>

namespace deliberate {

void UseMyOwnMessageHandler ();

void StartDebugLog (bool gui=true);
void StopDebugLog ();
bool DebugLogRecording ();

void MyOwnMessageOutput (QtMsgType type, const char* msg);

class DebugLog : public QDialog, public Ui_LogDialog {
Q_OBJECT

public:

  DebugLog (QWidget * parent);
  DebugLog ();
  
  bool Log (const char * msg);
  bool Log (const char * kind, const char * msg);
  void closeEvent (QCloseEvent *event);
  
public slots:

  void Close ();
  void quit ();
  void StopLogging () { isLogging = false; }
  void StartLogging () { isLogging = true; }
  bool IsLogging () { return isLogging; }
  bool IsUsingGui () { return useGui; }
  bool UseGui (bool gui=true) { useGui=gui; return gui; }
  void SaveLog ();
  
private slots:

  
private:

  void  Connect ();

  bool  isLogging;
  bool  useGui;

};


}
#endif
