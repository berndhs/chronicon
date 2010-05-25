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
#include <QTimer>
#include <map>

#include "ui_chronicon.h"

#include "network-if.h"
#include "timeline-view.h"
#include "item-dialog.h"
#include "status-block.h"
#include "config-edit.h"
#include "version.h"
#include "helpview.h"
#include "shortener.h"
#include "direct-dialog.h"
#include "switch-dialog.h"
#include "follow-dialog.h"
#include "pic-preview.h"
#include "ch-menu.h"

using namespace deliberate;

namespace chronicon {

/*! \brief The main window of the chronicon application 
  *
  * This is the top level object, contains the polling loop,
  * and controls the initialization flow of the other objects.
  * The network interface object lives here, and other objects
  * get pointer to it. Note that the NetworkIF object is not 
  * identical to a QNetworkAccessManager, that issue is up to
  * the NetworkIF class.
  */

class Chronicon : public QMainWindow, public Ui_ChroniconWindow {
Q_OBJECT

public:

Chronicon (QWidget * parent=0);

void SetApp (QApplication *a) {pApp = a;}

void Start ();

bool RunAgain ();

void SetupDefaults ();

public slots:

  void quit ();
  void PollComplete (TimelineKind kind);
  void ReallyFinishMessage (QString msg);
  void ChangeTimeline (int timeline, QString user);
  void ChangeFollow (QString user, int change);

private slots:

  void ReStart ();

  void startHelpMenu ();
  void startStartMenu ();
  void startActionMenu ();

  void startMessage ();
  void startMessage (QString msg, QString oldId = QString());
  void finishMessage ();
  void discardMessage ();
  void AutoLogin ();

  void NotYet ();
  void About ();
  void License ();
  void Manual ();
  void Configure ();

  void SuspendPoll (bool stopit);
  

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
  void SetupMenus ();
  void LabelSecs (int secs);
  
  int  normalEditVertical;
  QTimer  pollTimer;
  int     pollPeriod;
  int     pollRemain;
  int     pollTick;

  QTimer  debugTimer;

  NetworkIF   network;

  TimelineView  theView;
  TimelineKind  currentView;

  ChMenu        startMenu;
  ChMenu        actionMenu;
  ChMenu        helpMenu;

  ItemDialog    itemDialog;
  HelpView      helpView;
  ConfigEdit    configEdit;
  Shortener     shortener;
  DirectDialog  directDialog;
  SwitchDialog  switchDialog;
  FollowDialog  followDialog;
  PicPreview    picPreview;

  QApplication * pApp;

  bool     rerun;

  QString           inReplyTo;
  QString           otherUser;

#if USE_OAUTH
  QOAuth::Interface    auth;
#endif

};

} // namespace


#endif
