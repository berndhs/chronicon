

#include "weblogin.h"


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
 
#include "delib-debug.h"
#include <QFile>
#include <QWebFrame>
#include <QWebElement>
#include <QWebElementCollection>

namespace chronicon {

WebLogin::WebLogin (QWidget *parent, WebAuth * wa)
:QDialog (parent),
 page (0),
 webAuth (wa)
{
  setupUi (this);
  connect (cancelButton, SIGNAL (clicked()), this, SLOT (reject()));
  connect (okButton, SIGNAL (clicked()), this, SLOT (GrabPIN()));
  connect (webView, SIGNAL (loadFinished (bool)),
           this, SLOT (PageArrived (bool)));
  page = new MiniPage (this);
  webView->setPage (page);
}


void
WebLogin::Start ()
{
  authenticated = false;
  user = "";
  uid = "0";
  show ();
  pinEntry->setText ("");
qDebug () << " have webview ";
  webAuth->Init ();
qDebug () << " have Init ";
  bool ok = webAuth->AskRequestToken ();
qDebug () << " did request token part " << ok;
  if (ok) {
    webView->load (webAuth->WebUrlString());
  }
qDebug () << " now what? ";
  exec ();
}

void
WebLogin::GrabPIN ()
{
  qDebug () << " grabbing PIN ";
  webPin = pinEntry->text();
qDebug () << " pin is " << webPin;
  QString rawpage = webView->page()->mainFrame()->toHtml();
  QFile dump ("/home/bernd/mywork/twitter/testdata/pin-page.dmp");
  dump.open (QFile::WriteOnly);
  dump.write (rawpage.toUtf8());
  dump.close ();
  if (webPin.length() > 0) {
    bool worked = webAuth->AskAccessToken (webPin,atoken, asecret,
                                          screen_name, user_id);
qDebug () << " back from ask access " << worked;
    if (worked) {
       user = QString (screen_name);
       uid  = QString (user_id);
       authenticated = true;
       accept ();
    }
  }
}

void
WebLogin::PageArrived (bool good)
{
  if (good) {
    QString rawpage = webView->page()->mainFrame()->toHtml();
    QDateTime now = QDateTime::currentDateTime ();
    QString filename = QString 
           ("/home/bernd/mywork/twitter/testdata/pagedump-%1.html")
           .arg(now.toTime_t());
    QFile dump (filename);
    dump.open (QFile::WriteOnly);
    dump.write (rawpage.toUtf8());
    dump.close ();
    QString pin = SearchPin ();
    if (pin.length() > 0) {
      pinEntry->setText (pin);
    }
  }
}

QString
WebLogin::SearchPin ()
{
qDebug () << " search for pin";
  QString pin;
  QWebPage *page = webView->page();
  if (page == 0) { return QString(); }
  QWebFrame *frame = page->mainFrame ();
  if (frame == 0) { return QString(); }
qDebug () << " have frame";
  QWebElementCollection webParts = frame->documentElement ().findAll ("div");
  foreach (QWebElement divElt, webParts) {
qDebug () << " element has attribugs " << divElt.attributeNames ();
    if (divElt.attribute ("id") == "oauth_pin") {
      pin = divElt.toPlainText().trimmed();
      return pin;
    }
  }
  // if we get here, we didn't find it
  return QString();
}


} // namespace
