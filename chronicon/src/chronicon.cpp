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
#include <QFileDialog>
#include <QRegExp>
#include <QTextCursor>


using namespace deliberate;

namespace chronicon {


Chronicon::Chronicon (QWidget *parent)
:QMainWindow(parent),
 pollTimer (this),
 pollPeriod (5*60*1000),
 pollRemain (0),
 pollTick  (1000),
 polling (true),
 debugTimer (this),
 network (this),
 theView (this),
 currentView (R_Public),
 startMenu (this),
 actionMenu (this),
 helpMenu (this),
 itemDialog (this),
 helpView (this),
 configEdit (this),
 shortener (this),
 directDialog (this),
 switchDialog (this),
 followDialog (this),
 picPreview (this),
 pApp(0),
 rerun (false)
{
  setupUi (this);
  theView.SetView (messageView);
  theView.SetNetwork (&network);
  normalEditVertical = ownMessage->sizePolicy().verticalStretch();

  Connect ();
  SetupMenus ();
  SetupTimers (true);
  network.SetUserAgent (Settings().value("program").toString());

  itemDialog.SetNetwork (&network);
  shortener.SetNetwork (&network);
  directDialog.SetNetwork (&network);
}

bool
Chronicon::RunAgain ()
{
  bool again = rerun;
  rerun = false;
  return again;
}

void
Chronicon::Connect ()
{
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
  connect (&network, SIGNAL (NewUserInfo (UserBlock)),
           &theView, SLOT (CatchUserInfo (UserBlock)));
  connect (&network, SIGNAL (ClearList()),
           &theView, SLOT (ClearList()));
  connect (&network, SIGNAL (ReplyComplete(TimelineKind, bool)),
           this, SLOT (PollComplete(TimelineKind, bool)));
  connect (&network, SIGNAL (RePoll(TimelineKind)),
           this, SLOT (RePoll(TimelineKind)));
  connect (&network, SIGNAL (StopPoll (bool)),
           this, SLOT (SuspendPoll (bool)));
  connect (&network, SIGNAL (SecondaryMessage (QString)),
           this, SLOT (startMessage (QString)));

  connect (&theView, SIGNAL (ItemDialog (QString, StatusBlock, QString)),
           &itemDialog, SLOT (Exec(QString , StatusBlock, QString)));
  connect (&theView, SIGNAL (Search (QString)), 
           &network, SLOT (PullSearch (QString)));
  connect (&theView, SIGNAL (TimelineSwitch (int, QString)),
            this, SLOT (ChangeTimeline (int, QString)));
  connect (&theView, SIGNAL (GetMixedUsers (QString)),
            this, SLOT (ChangeMixedView (QString)));

  connect (&itemDialog, SIGNAL (SendMessage (QString,QString)), 
           this, SLOT (startMessage (QString,QString)));
  connect (&itemDialog, SIGNAL (MakeDirect (QString)),
           &directDialog, SLOT (WriteMessage (QString)));
  connect (&itemDialog, SIGNAL (MaybeFollow (StringBlock)),
           &followDialog, SLOT (Exec (StringBlock)));
  connect (&itemDialog, SIGNAL (GetTimeline (int, QString)),
           this, SLOT (ChangeTimeline (int, QString)));

  connect (&directDialog, SIGNAL (SendDirect (QString, QString)),
           &network, SLOT (DirectMessage (QString, QString)));

  connect (&switchDialog, SIGNAL (TimelineSwitch (int, QString)),
            this, SLOT (ChangeTimeline (int, QString)));

  connect (&followDialog, SIGNAL (Follow (QString, int)),
            this, SLOT (ChangeFollow (QString, int)));

  connect (&picPreview, SIGNAL (SendPic (QString, QString)),
            &network, SLOT (PushPicOA (QString, QString)));

  connect (&shortener, SIGNAL (DoneShortening (QString )),
           this, SLOT (ReallyFinishMessage (QString)));

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
Chronicon::SetupMenus ()
{

  /** start/stop menu */

  menubar->addAction (tr("Start/Stop..."), this, SLOT (startStartMenu()));
  startMenu.addAction (tr("Login"), &network, SLOT (login()));
  startMenu.addAction (tr("Auto Login"), this, SLOT (AutoLogin()));
  startMenu.addAction (tr("Configure"), this, SLOT (Configure()));
  startMenu.addAction (tr("Flush Data"), &theView, SLOT (FlushTimelines()));
  startMenu.addAction (tr("Restart"), this, SLOT (ReStart()));
  startMenu.addAction (tr("Quit"), this, SLOT (quit()));

  /** action menu */

  menubar->addAction (tr("Actions..."), this, SLOT (startActionMenu()));
  actionMenu.addAction (tr("Start New Update"), this, SLOT (startMessage()));
  actionMenu.addAction (tr("(Un-) Follow Someone"), 
                       &followDialog, SLOT (Exec()));
  actionMenu.addAction (tr("My Timeline"),
                        this, SLOT (PollUserPrivate()));
  actionMenu.addAction (tr("Choose Timeline"), 
                       &switchDialog, SLOT (Exec()));
  actionMenu.addAction (tr("Twitpic Image Upload"), 
                       &picPreview, SLOT (Exec ()));


  /** Help Menu */
  menubar->addAction (tr("Help..."), this, SLOT (startHelpMenu()));
  helpMenu.addAction (tr("About"), this, SLOT (About()));
  helpMenu.addAction (tr("Manual"), this, SLOT (Manual()));
  helpMenu.addAction (tr("License"), this, SLOT (License()));
}

void
Chronicon::SetupDefaults ()
{
  QString bitly_user ("");
  bitly_user = Settings().value("network/bitly_user",bitly_user).toString();
  Settings().setValue ("network/bitly_user",bitly_user);
  QString bitly_key ("");
  bitly_key = Settings().value ("network/bitly_key",bitly_key).toString();
  Settings().setValue ("network/bitly_key", bitly_key);
  Settings().sync();
}

void
Chronicon::startStartMenu ()
{
  startMenu.SetPos (QCursor::pos());
  QTimer::singleShot (50, &startMenu, SLOT (Popup()));
}

void
Chronicon::startActionMenu ()
{
  actionMenu.SetPos (QCursor::pos());
  QTimer::singleShot (50, &actionMenu, SLOT (Popup()));
}



void
Chronicon::startHelpMenu ()
{
  helpMenu.SetPos (QCursor::pos());
  QTimer::singleShot (50, &helpMenu, SLOT (Popup()));
}

void
Chronicon::ReStart ()
{
  rerun = true;
  quit();
}


void
Chronicon::Start ()
{
  if (Settings().contains("sizes/main")) {
    QSize defaultSize = size();
    QSize newsize = Settings().value ("sizes/main", defaultSize).toSize();
    resize (newsize);
  }
  if (Settings().contains("network/lastuser")) {
    QString lastuser = Settings().value("network/lastuser",QString("")).toString();
    network.SetBasicAuth (lastuser);
  }
  SmallEdit ();
  show ();
  pollRemain = 0;
  QTimer::singleShot (500, this, SLOT (Poll()));

  QString defaultDir = QDesktopServices::storageLocation 
                        (QDesktopServices::DocumentsLocation);
  defaultDir = Settings ().value ("files/savedir",defaultDir).toString();
  Settings().setValue ("files/savedir",defaultDir);

  theView.LoadSettings ();
  itemDialog.LoadSettings ();
  int startPrivate = 0;
  startPrivate = Settings().value ("network/start_private",startPrivate).toInt();
  Settings().setValue ("network/start_private",startPrivate);
  if (startPrivate) {
    currentView = R_Private;
    switchDialog.SetCurrent (currentView);
qDebug () << " starting private ";
  } else {
    currentView = R_Public;
    switchDialog.SetCurrent (currentView);
qDebug () << " starting public ";
  }
  SetupDefaults ();
  network.SetTimeline (currentView);
  network.Init ();
}


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
Chronicon::Configure ()
{
  SuspendPoll (true);
  int changed = configEdit.Exec ();
  if (changed != 0) {
    network.Init ();
  }
  SuspendPoll (false);
}

void
Chronicon::startMessage ()
{
  inReplyTo = "";
  BigEdit ();
}

void
Chronicon::startMessage (QString msg, QString oldId)
{
  if (oldId.length() > 0) {
    inReplyTo = oldId;
  } else {
    inReplyTo = "";
  }
  ownMessage->setText (msg);
  ownMessage->moveCursor (QTextCursor::Start);
  BigEdit();
}


void
Chronicon::finishMessage ()
{
  QString msg;
  ownMessage->extractPlain (msg);
  bool wait (false);
  if (Settings().contains ("network/bitly_user")) {
    QString bitly_user = Settings().value("network/bitly_user",QString())
                                   .toString();
    if (bitly_user.length() > 0) {
      shortener.ShortenHttp (msg, wait);
    }
  } 
  if (!wait) {
    ReallyFinishMessage (msg);
  }
}

void
Chronicon::ReallyFinishMessage (QString msg)
{
  msg = msg.trimmed();
  int len = msg.length();
  bool sendit(true);
  bool didsend (false);
  if (len > 0) {
    QByteArray data = msg.toUtf8();
    int datalen = data.size();
    if (datalen > 140) {
       QMessageBox toolong;
       int toomany = datalen - 140;
       QString msgpattern (tr("Message %1 characters too long, send anyway?"));
       toolong.setText (msgpattern.arg(toomany));
       QAbstractButton * yesBut = toolong.addButton (QMessageBox::Ok);
       toolong.addButton (QMessageBox::Cancel);
       toolong.exec ();
       sendit = (toolong.clickedButton() == yesBut);
    }
    if (sendit) {
      network.PushUserStatus (msg,inReplyTo);
       didsend = true;
    } else {
      startMessage (msg, inReplyTo);
    }
  }
  if (didsend) {
    SmallEdit ();
  }
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
Chronicon::SuspendPoll (bool stopit)
{
  if (stopit) {
    pollTimer.stop ();
    loadLabel->setText (tr("stopped"));
  } else {
    pollTimer.start (pollTick);
    LabelSecs (pollRemain/1000);
  }
  polling = !stopit;
}

void
Chronicon::Poll ()
{
  if (!polling) {
    return;
  }
  pollRemain -= pollTick;
  if (pollRemain <= 0) {
    if (currentView == R_OtherUser) {
      network.PullTimeline (otherUser);
    } else {
      network.PullTimeline ();
    }
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
Chronicon::PollUserPrivate ()
{
qDebug () << " poll private, was " << currentView;
  network.SetTimeline (R_Private);
  RePoll (R_Private);
qDebug () << " did repoll now for " << currentView;
}

void
Chronicon::RePoll (TimelineKind kind)
{
  if (kind != R_None) {
    currentView = kind; 
    switchDialog.SetCurrent (currentView);
  }
  pollTimer.stop();
  pollRemain = pollPeriod;
  pollTimer.start (pollTick);
  theView.Display (currentView);
  if (currentView == R_OtherUser) {
    network.PullTimeline (otherUser);
  } else {
    network.PullTimeline ();
  }
  loadLabel->setText (tr("reload..."));
}

// \brief PollComplete - start the redisplay

void
Chronicon::PollComplete (TimelineKind kind, bool resumePoll)
{
  if (polling) {
    LabelSecs (pollRemain/1000);
  }
  if (kind == R_Public 
      || kind == R_Private 
      || kind == R_SearchResults
      || kind == R_MixedUsers) {
    theView.Display (kind);
  }
  theView.Show ();
  if (kind == R_Private) {
    QTimer::singleShot (pollTick - 1,&network, SLOT(PullUserBlock ()));
  }
  if (resumePoll && !polling) {
    SuspendPoll (false);
  }
}

void
Chronicon::ChangeFollow (QString user, int change)
{
  network.ChangeFollow (user, change);
}

void
Chronicon::ChangeTimeline (int timeline, QString user)
{
  TimelineKind tl;
  if (timeline <= R_None || timeline >= R_Top) {  
    tl = R_None;
  } else {
    tl = static_cast <TimelineKind>(timeline);
  }
  if (tl == R_SearchResults) {
    network.PullSearch (user);
    theView.Display (tl);
  }
  currentView = tl;
  switchDialog.SetCurrent (currentView);
 
  if (currentView != R_Public) {
    if (!network.HaveUser()) {
      AutoLogin();
    }
  }
  if (currentView == R_OtherUser) {
    otherUser = user;
    theView.Display (currentView);
  }
  network.SetTimeline (currentView);
  theView.ClearList (currentView);
  RePoll (currentView);
}

void
Chronicon::ChangeMixedView (QString otherUsers)
{
  currentView = R_MixedUsers;
  switchDialog.SetCurrent (currentView);
  network.PullMixedUsers (currentView, otherUsers);
  theView.ClearList (currentView);
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
  sendButton->show ();
  cancelButton->show ();
  QSizePolicy editPoli = ownMessage->sizePolicy ();
  QSizePolicy viewPoli = messageView->sizePolicy ();
  editPoli.setVerticalStretch (viewPoli.verticalStretch());
  ownMessage->setSizePolicy (editPoli);
}

void
Chronicon::SmallEdit ()
{
  sendButton->hide ();
  cancelButton->hide ();
  QSizePolicy editPoli = ownMessage->sizePolicy();
  editPoli.setVerticalStretch (normalEditVertical);
  ownMessage->setSizePolicy (editPoli);
}

void
Chronicon::AutoLogin ()
{
  QByteArray user = "nobody";
  QByteArray key1 = "";
  QByteArray key2 = "";
  user = Settings().value ("oauth/screen_name",user).toByteArray();
  key1 = Settings().value ("oauth/token",key1).toByteArray();
  key2 = Settings().value ("oauth/secret",key2).toByteArray();
  QString mode = Settings().value ("network/login_type",QString("basic")).toString();
  if (mode == "oauth") {
    network.AutoLogin (user, key1, key2, true);
  } else {
    network.login ();
  }
  if (currentView == R_Public) {
    currentView = R_Private;
    switchDialog.SetCurrent (currentView);
  }
  network.SetTimeline (currentView);
  theView.ClearList ();
  RePoll (currentView);
}

void
Chronicon::resizeEvent (QResizeEvent * event)
{
  QSize newsize = event->size();
  Settings().setValue ("sizes/main",newsize);
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
