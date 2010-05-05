/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2010, Bernd Stramm
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include "chronicon.h"
#include <QApplication>
#include "delib-debug.h"
#include "cmdoptions.h"
#include <QSettings>
#include "deliberate.h"
#include "version.h"

int
main (int argc, char * argv[])
{
  
  QCoreApplication::setApplicationName ("chronicon");
  QCoreApplication::setOrganizationName ("BerndStramm");
  QCoreApplication::setOrganizationDomain ("bernd-stramm.com");
  deliberate::ProgramVersion pv ("Chronicon");
  QCoreApplication::setApplicationVersion (pv.Version());
  QSettings  settings;
  deliberate::SetSettings (settings);

  QApplication App (argc, argv);

  deliberate::CmdOptions  opts ("Chronicon");
  opts.AddSoloOption ("debug","D","show Debug log window");


  deliberate::UseMyOwnMessageHandler ();

  bool optsOk = opts.Parse (argc, argv);
  if (!optsOk) {
    opts.Usage ();
    exit(1);
  }
  if (opts.WantHelp ()) {
    opts.Usage ();
    exit (0);
  }
  pv.CLIVersion ();
  if (opts.WantVersion ()) {
    exit (0);
  }
  bool showDebug = opts.SeenOpt ("debug");

  deliberate::UseMyOwnMessageHandler ();
  deliberate::StartDebugLog (showDebug);

  chronicon::Chronicon chron;
 
  chron.SetApp (&App);
  chron.Start ();

  int status = App.exec ();
  
  return status;
}
