#include "deliberate.h"


//
//  Copyright (C) 2010 - Bernd H Stramm 
//
// This program is distributed under the terms of 
// the GNU General Public License version 3 
//
// This software is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty 
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//

namespace deliberate {

QTextStream & StdOut ()
{
  static QTextStream *out(0);
  
  if (out == 0) {
    out = new QTextStream (stdout);
  }
  return *out;
}


static QSettings * mySettings(0);
  
void
SetSettings (QSettings & settings)
{
  if (mySettings) {
    delete mySettings;
  }
  mySettings = &settings;
}

QSettings &
Settings ()
{
  if (mySettings) {
    mySettings = new QSettings;
  }
  return *mySettings;
}

bool
IsMaemo ()
{
  #ifdef Q_WS_MAEMO_5
  return true;
  #else
  return false;
  #endif
}

bool
IsFingerInterface ()
{
  return IsMaemo ();
}


}
