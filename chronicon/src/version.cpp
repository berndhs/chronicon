#include "version.h"
#include "deliberate.h"
#include <QMessageBox>
#include <QTimer>
#include <QCoreApplication>

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

namespace deliberate {

  QString ProgramVersion::VersionNumber ("0.1.2");

  QString ProgramVersion::ProgramName("Chronicon");
  QString ProgramVersion::copyright ("Copyright (C) 2010 Bernd H Stramm");
  
  ProgramVersion::ProgramVersion (QString pgmname)
  {
    ProgramName = pgmname;
  }
  
  QString ProgramVersion::Version ()
  { 
    return ProgramName + QString (" Version " )
                   + VersionNumber + " " + QString(__DATE__) + " "
		       + QString(__TIME__)
		       + QString("\r\n")
		       + copyright;
  }

  void ProgramVersion::ShowVersionWindow ()
  {
    QString versionMessage(Version());
    QMessageBox box;
    box.setText (versionMessage);
    QTimer::singleShot(30000,&box,SLOT(accept()));
    box.exec();
  }
  
  void ProgramVersion::CLIVersion ()
  {
    StdOut() << Version() << endl;
  }
  
  QString ProgramVersion::MyName()
  { 
    return ProgramName;
  }

}
