#ifndef SATVIEW_VERSION_H
#define SATVIEW_VERSION_H

//
//  Copyright (C) 2010 - Bernd H Stramm 
//
// This program is distributed under the terms of 
// the GNU General Public License version 2 
//
// This software is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty 
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//

#include <QString>
#include "delib-debug.h"

namespace deliberate {

class ProgramVersion {

public:

  ProgramVersion (QString pgmname);
  
  static QString Version (); 
  static QString MyName ();
  
  static void ShowVersionWindow ();
  static void CLIVersion ();
  
private:

  static QString VersionNumber;
  static QString ProgramName;
  static QString copyright;

};

}

#endif
