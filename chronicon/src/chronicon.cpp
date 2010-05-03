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
#include <QDebug>
#include <QDateTime>



namespace chronicon {

Chronicon::Chronicon (QWidget *parent)
:QMainWindow(parent),
 pollTimer (this),
 pollPeriod (60*1000),
 debugTimer (this),
 network (this),
 theView (this),
 pApp(0)
{
  setupUi (this);
  theView.SetView (messageView);
  normalEditVertical = ownMessage->sizePolicy().verticalStretch();

  Connect ();
  SetupTimers ();

  QTimer::singleShot (2000, this, SLOT (Poll()));
}

void
Chronicon::Connect ()
{
  connect (actionQuit, SIGNAL (triggered()), this, SLOT (quit()));
  connect (typeButton, SIGNAL (clicked()), this, SLOT (startMessage()));
  connect (updateButton, SIGNAL (clicked()), this, SLOT (Poll()));
  connect (sendButton, SIGNAL (clicked()), this, SLOT (finishMessage()));
  connect (cancelButton, SIGNAL (clicked()), this, SLOT (discardMessage()));
  connect (ownMessage, SIGNAL (KeyPressed (int)),
           this, SLOT (firstKey(int)));
  connect (ownMessage, SIGNAL (ReturnPressed ()),
           this, SLOT (returnKey()));

  connect (&network, SIGNAL (NewStatusItem (StatusBlock, TimelineKind)),
           &theView, SLOT (CatchStatusItem (StatusBlock, TimelineKind)));
  connect (&network, SIGNAL (ReplyComplete()),
           this, SLOT (PollComplete()));
}

void
Chronicon::SetupTimers ()
{
  connect (&pollTimer, SIGNAL (timeout()), this, SLOT (Poll()));
  pollTimer.start (pollPeriod);

  connect (&debugTimer, SIGNAL (timeout()), this, SLOT (DebugCheck()));
  debugTimer.start (15*1000);
}

void
Chronicon::quit ()
{
  if (pApp) {
    pApp->quit();
  }
}

void
Chronicon::startMessage ()
{
  BigEdit ();
}

void
Chronicon::finishMessage ()
{
  QString msg;
  ownMessage->extractPlain (msg);
  qDebug () << " message is " << msg;
  SmallEdit ();
}

void
Chronicon::discardMessage ()
{
  ownMessage->clearText ();
  SmallEdit ();
}

void
Chronicon::firstKey (int key)
{
  startMessage ();
}

void
Chronicon::returnKey ()
{
  finishMessage ();
}

void
Chronicon::Poll ()
{
  network.PullPublicTimeline ();
}

void
Chronicon::PollComplete ()
{
  theView.Display (R_Public);
  theView.Show ();
}

void
Chronicon::DebugCheck ()
{
  qDebug() << " debug check " << 
      QDateTime::currentDateTime().toString ("ddd hh:mm:ss");
}

void
Chronicon::BigEdit ()
{
  QSizePolicy editPoli = ownMessage->sizePolicy ();
  QSizePolicy viewPoli = messageView->sizePolicy ();
  editPoli.setVerticalStretch (viewPoli.verticalStretch());
  ownMessage->setSizePolicy (editPoli);
}

void
Chronicon::SmallEdit ()
{
  QSizePolicy editPoli = ownMessage->sizePolicy();
  editPoli.setVerticalStretch (normalEditVertical);
  ownMessage->setSizePolicy (editPoli);
}

} // namespace
