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
#include "deliberate.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QWidget>
#include <QLineEdit>
#include <QByteArray>
#include <QDesktopServices>


using namespace deliberate;

namespace chronicon {


Chronicon::Chronicon (QWidget *parent)
:QMainWindow(parent),
 pollTimer (this),
 pollPeriod (5*60*1000),
 pollRemain (0),
 pollTick  (1000),
 debugTimer (this),
 network (this),
 theView (this),
 currentView (R_Private),
 itemDialog (this),
 helpView (this),
 pApp(0)
{
  setupUi (this);
  theView.SetView (messageView);
  theView.SetNetwork (&network);
  normalEditVertical = ownMessage->sizePolicy().verticalStretch();

  Connect ();
  SetupTimers (true);
  network.SetUserAgent (Settings().value("program").toString());

  itemDialog.SetNetwork (&network);
}

void
Chronicon::Connect ()
{
  connect (actionQuit, SIGNAL (triggered()), this, SLOT (quit()));
  connect (actionLogin, SIGNAL (triggered()), &network, SLOT (login()));
  connect (actionConfigure, SIGNAL (triggered()), this, SLOT (NotYet()));
  connect (actionAbout, SIGNAL (triggered()), this, SLOT (About()));
  connect (actionManual, SIGNAL (triggered()), this, SLOT (Manual()));
  connect (actionLicense, SIGNAL (triggered()), this, SLOT (License()));

  connect (typeButton, SIGNAL (clicked()), this, SLOT (startMessage()));
  connect (updateButton, SIGNAL (clicked()), this, SLOT (RePoll()));
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
  connect (&network, SIGNAL (RePoll(TimelineKind)),
           this, SLOT (RePoll(TimelineKind)));

  connect (&theView, SIGNAL (ItemDialog (QString, StatusBlock, QString)),
           &itemDialog, SLOT (Exec(QString , StatusBlock, QString)));
}

void
Chronicon::SetupTimers (bool debug)
{
  pollPeriod = Settings().value("timers/pollperiod",pollPeriod).toInt();
  Settings().setValue ("timers/pollperiod",pollPeriod);
  connect (&pollTimer, SIGNAL (timeout()), this, SLOT (Poll()));
  pollRemain = pollPeriod;
  pollTimer.start (pollTick);

  connect (&debugTimer, SIGNAL (timeout()), this, SLOT (DebugCheck()));
  if (debug) {
    //debugTimer.start (15*1000);
  }
}

void
Chronicon::Start ()
{
  if (Settings().contains("size")) {
    QSize defaultSize = size();
    QSize newsize = Settings().value ("size", defaultSize).toSize();
    resize (newsize);
  }
  if (Settings().contains("lastuser")) {
    QString lastuser = Settings().value("lastuser",QString("")).toString();
    network.SetBasicAuth (lastuser);
  }
  show ();
  pollRemain = 0;
  QTimer::singleShot (500, this, SLOT (Poll()));

  QString defaultDir = QDesktopServices::storageLocation 
                        (QDesktopServices::DocumentsLocation);
  defaultDir = Settings ().value ("defaultdir",defaultDir).toString();
  Settings().setValue ("defaultdir",defaultDir);

  theView.LoadSettings ();
  itemDialog.LoadSettings ();
  network.Init ();
}

#if USE_OAUTH
void
Chronicon::ReadRSA (QCA::SecureArray & secure)
{
  
}
#endif

void
Chronicon::License ()
{
  helpView.Show ("qrc:/LICENSE.txt");
}

void
Chronicon::Manual ()
{
  helpView.Show ("qrc:/helpman.html");
}
void
Chronicon::NotYet ()
{
  QMessageBox dontHaveIt (this);
  Qt::WindowFlags flags = dontHaveIt.windowFlags ();
  flags |= Qt::FramelessWindowHint;
  dontHaveIt.setWindowFlags (flags);
  dontHaveIt.setText ("Function not yet available");
  dontHaveIt.exec (); 
}

void
Chronicon::About ()
{
  deliberate::ProgramVersion::ShowVersionWindow ();
}

void
Chronicon::quit ()
{
  if (pApp) {
    Settings().sync();
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
  msg = msg.trimmed();
  int len = msg.length();
  bool sendit(true);
  if (len > 0) {
    QByteArray data = msg.toUtf8();
    int datalen = data.size();
    if (datalen > 140) {
       QMessageBox toolong;
       toolong.setText (tr("Message too long for Twitter, send anyway?"));
       QAbstractButton * yesBut = toolong.addButton (QMessageBox::Ok);
       toolong.addButton (QMessageBox::Cancel);
       toolong.exec ();
       sendit = (toolong.clickedButton() == yesBut);
    }
    if (sendit) {
      network.PushUserStatus (msg);
    }
  }
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
  pollRemain -= pollTick;
  if (pollRemain <= 0) {
    network.PullTimeline ();
    loadLabel->setText (tr("load..."));
    pollRemain = pollPeriod;
  } else {
    LabelSecs (pollRemain/1000);
  }
}

void
Chronicon::LabelSecs (int secs)
{
  int mins = secs/60;
  secs = secs - (mins*60);
  QString pattern ("%1:%2");
  QChar zero ('0');
  loadLabel->setText (pattern.arg(mins,2,10,zero).arg(secs,2,10,zero));
}

void
Chronicon::RePoll (TimelineKind kind)
{
  if (kind != R_None) {
    currentView = kind; 
  }
  pollTimer.stop();
  pollRemain = pollPeriod;
  pollTimer.start (pollTick);
  theView.Display (currentView);
  network.PullTimeline ();
  loadLabel->setText (tr("reload..."));
}

void
Chronicon::PollComplete ()
{
  LabelSecs (pollRemain/1000);
  theView.Display (R_Private);
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

void
Chronicon::resizeEvent (QResizeEvent * event)
{
  QSize newsize = event->size();
  Settings().setValue ("size",newsize);
  QMainWindow::resizeEvent (event);
}


void
Chronicon::closeEvent (QCloseEvent *event)
{
  Settings().sync();
  QMainWindow::closeEvent (event);
}

void
Chronicon::hideEvent (QHideEvent *event)
{
  theView.SetNotify (true);
  QMainWindow::hideEvent (event);
}

void
Chronicon::showEvent (QShowEvent *event)
{
  theView.SetNotify (false);
  QMainWindow::showEvent (event);
}

} // namespace
