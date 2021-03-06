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

#if USE_NOTIFY
#include <libnotify/notify.h>
#endif
#include "chronicon-global.h"
#include "chronicon.h"
#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include "delib-debug.h"
#include "cmdoptions.h"
#include <QSettings>
#include "deliberate.h"
#include "version.h"


const char chronicon::ChroniconName[] = "Chronicon";



int
main (int argc, char * argv[])
{
  QCoreApplication::setApplicationName ("chronicon");
  QCoreApplication::setOrganizationName ("BerndStramm");
  QCoreApplication::setOrganizationDomain ("bernd-stramm.com");
  deliberate::ProgramVersion pv (chronicon::ChroniconName);
  QCoreApplication::setApplicationVersion (pv.Version());
#if USE_NOTIFY
  notify_init (chronicon::ChroniconName);
#endif

  QApplication App (argc, argv);
  QSettings  settings;
  deliberate::SetSettings (settings);
  settings.setValue ("program",pv.MyName());
  deliberate::CmdOptions  opts ("Chronicon");
  opts.AddSoloOption ("debug","D","show Debug log window");
  opts.AddIntOption ("maxitems","M","maximum length of timeline shown");


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

  deliberate::StartDebugLog (showDebug);
  deliberate::SetQuietDebug (!showDebug);
  bool runit (true);
  int status (1);
  while (runit) {
  
    chronicon::Chronicon chron;

    int maxitems (100);
    maxitems = settings.value("view/maxitems",maxitems).toInt();
    opts.SetIntOpt ("view/maxitems",maxitems);
    settings.setValue ("view/maxitems",maxitems);
 
    chron.SetApp (&App);
    chron.Start ();

    status = App.exec ();
    if (showDebug ) {
      deliberate::SaveStaticLog ("/tmp/chronicon-debug.log");
    }
    runit = chron.RunAgain();
  }
  return status;
}

