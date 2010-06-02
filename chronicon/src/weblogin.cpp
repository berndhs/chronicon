

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
#include "deliberate.h"

using namespace deliberate;

namespace chronicon {

WebLogin::WebLogin (QWidget *parent, WebAuth * wa)
:QDialog (parent),
 page (0),
 webAuth (wa),
 loadingFirst(false)
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
  QString agentString = page->UserAgent ();
  agentString = Settings().value ("weblogin/useragentstring",agentString).toString();
  Settings().setValue ("weblogin/useragentstring",agentString);
  page->SetUserAgent (agentString);
  if (Settings().contains("sizes/weblogin")) {
    QSize defaultSize = size();
    QSize newsize = Settings().value ("sizes/weblogin", defaultSize).toSize();
    resize (newsize);
  }
  show ();
  pinEntry->setText ("");
  if (!webAuth->InitDone()) {
    webAuth->Init ();
  }
  bool ok = webAuth->AskRequestToken ();
  if (ok) {
    webView->load (webAuth->WebUrlString());
    instructions->setText (tr("Loading Service  Web Page..."));
    loadingFirst = true;
  } else {
    instructions->setText (tr("Failure Contacting Service"));
  }
  exec ();
}

void
WebLogin::GrabPIN ()
{
  webPin = pinEntry->text();
  if (webPin.length() > 0) {
    bool worked = webAuth->AskAccessToken (webPin,atoken, asecret,
                                          screen_name, user_id);
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
    #if 0
    QString rawpage = webView->page()->mainFrame()->toHtml();
    QDateTime now = QDateTime::currentDateTime ();
    QString filename = QString 
           ("/home/bernd/mywork/twitter/testdata/pagedump-%1.html")
           .arg(now.toTime_t());
    QFile dump (filename);
    dump.open (QFile::WriteOnly);
    dump.write (rawpage.toUtf8());
    dump.close ();
    #endif
    if (loadingFirst) {
      instructions->setText (tr("Please Log in Below"));
      loadingFirst = false;
    } else {
      QString pin = SearchPin ();
      if (pin.length() > 0) {
        pinEntry->setText (pin);
        instructions->setText (tr("Verify the PIN and click OK"));
      } else {
        instructions->setText (tr("Cannot find PIN"));
      }
    }
  }
}

QString
WebLogin::SearchPin ()
{
  QString pin;
  QWebPage *page = webView->page();
  if (page == 0) { return QString(); }
  QWebFrame *frame = page->mainFrame ();
  if (frame == 0) { return QString(); }
  QWebElementCollection webParts = frame->documentElement ().findAll ("div");
  foreach (QWebElement divElt, webParts) {
    if (divElt.attribute ("id") == "oauth_pin") {
      pin = divElt.toPlainText().trimmed();
      return pin;
    }
  }
  // if we get here, we didn't find it
  return QString();
}
void
WebLogin::resizeEvent (QResizeEvent * event)
{
  QSize newsize = event->size();
  Settings().setValue ("sizes/weblogin",newsize);
  QDialog::resizeEvent (event);
}



} // namespace
