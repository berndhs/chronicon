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

#ifndef CHRONICON_H
#define CHRONICON_H

#include <QMainWindow>
#include <QApplication>
#if USE_OAUTH
  #include <QtOAuth>
  #include <QtCrypto>
#endif
#include <QTimer>

#include "ui_chronicon.h"

#include "network-if.h"
#include "timeline-view.h"
#include "item-dialog.h"
#include "status-block.h"
#include "version.h"
#include "helpview.h"

using namespace deliberate;

namespace chronicon {


class Chronicon : public QMainWindow, public Ui_ChroniconWindow {
Q_OBJECT

public:

Chronicon (QWidget * parent=0);

void SetApp (QApplication *a) {pApp = a;}

void Start ();

public slots:

  void quit ();
  void PollComplete ();

private slots:

  void startMessage ();
  void finishMessage ();
  void discardMessage ();

  void NotYet ();
  void About ();
  void License ();
  void Manual ();
  

  void firstKey (int key);
  void returnKey ();

  void Poll ();
  void RePoll (TimelineKind kind=R_None);

  void DebugCheck ();

protected:

  void closeEvent (QCloseEvent * event);
  void resizeEvent ( QResizeEvent * event )  ;
  void hideEvent (QHideEvent * event);
  void showEvent (QShowEvent * event);

private:

  void BigEdit ();
  void SmallEdit ();
  void Connect ();
  void SetupTimers (bool debug=false);
  void LabelSecs (int secs);
#if USE_OAUTH
  void ReadRSA (QCA::SecureArray & secure);
#endif  
  int  normalEditVertical;
  QTimer  pollTimer;
  int     pollPeriod;
  int     pollRemain;
  int     pollTick;

  QTimer  debugTimer;

  NetworkIF   network;

  TimelineView  theView;
  TimelineKind  currentView;
  ItemDialog    itemDialog;
  HelpView      helpView;

  QApplication * pApp;

#if USE_OAUTH
  QOAuth::Interface    auth;
#endif

};

} // namespace


#endif
