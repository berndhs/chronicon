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

#include "deliberate.h"
#include "network-if.h"
#include "ui_enterpass.h"
#include <QDomDocument>

namespace chronicon {

NetworkIF::NetworkIF (QObject *parent)
:network(parent),
 serviceKind (R_Public),
 user (QString()),
 pass (QString())
{
  serviceKind = R_Private;
  SwitchTimeline();
  connect (&network, SIGNAL (authenticationRequired(QNetworkReply*, QAuthenticator*)),
           this, SLOT (authProvide (QNetworkReply*, QAuthenticator*)));
}

void
NetworkIF::SwitchTimeline ()
{
  switch (serviceKind) {
    case R_Private:
      timelineName = "home_timeline";
      break;
    case R_Public:
    default:
      timelineName = "public_timeline";
      break;
  }
}

void
NetworkIF::PullTimeline ()
{
  QNetworkRequest request;
  QUrl url  (QString("http://api.twitter.com/1/statuses/%1.xml")
                    .arg(timelineName));
  request.setUrl(url);
  QNetworkReply *reply = network.get (request);
  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  serviceKind);
  ExpectReply (reply, chReply);
}

void
NetworkIF::handleReply (ChronNetworkReply * reply)
{
  if (reply) {
    QNetworkReply * netReply = reply->NetReply();
    TimelineKind kind (reply->Kind());
    if (kind == R_Public || kind == R_Private) {
      QDomDocument doc;
      doc.setContent (netReply);
      ParseDom (doc, kind);
      emit ReplyComplete ();
    } else if (kind == R_Update) {
      QDomDocument update;
      update.setContent (netReply);
      ParseUpdate (update, kind);
      emit ReplyComplete ();
    }
    ReplyMapType::iterator index;
    index = replies.find (netReply);
    if (index != replies.end()) {
      replies.erase(index);
    }
    CleanupReply (netReply, reply);    
  }
}


void
NetworkIF::SetBasicAuth (QString us, QString pa)
{
  user = us;
  pass = pa;
}

void
NetworkIF::login (int * reply)
{
  Ui_TextEnter textenter;
  QDialog  textDialog;
  textenter.setupUi (&textDialog);
  textenter.passEdit->setEchoMode (QLineEdit::Password);
  textenter.userEdit->setText (user);
  int ok = textDialog.exec ();
  if (ok) {
    user = textenter.userEdit->text();
    pass = textenter.passEdit->text();
    serviceKind = R_Private;
    SwitchTimeline ();
    emit RePoll (serviceKind);
    deliberate::Settings().setValue ("lastuser",user);
  } else {
    serviceKind = R_Public;
    SwitchTimeline ();
  }
  if (reply) {
    *reply = ok;
  }
}

void
NetworkIF::authProvide (QNetworkReply *reply, QAuthenticator *au)
{
  if (reply && au) {
    int tryAgain (0);
    login (&tryAgain);
    if (tryAgain == 1) {
      au->setPassword (pass);
      au->setUser (user);
    } else {
      reply->close();
    }
  }
}

void
NetworkIF::networkError (QNetworkReply::NetworkError err)
{
  qDebug () << "network error " << err;
}

void
NetworkIF::ParseDom (QDomDocument & doc, TimelineKind kind)
{
  QDomElement root = doc.documentElement();
  QDomElement child;
  for (child = root.firstChildElement(); !child.isNull(); 
       child = child.nextSiblingElement()) {
    if (child.tagName() == "status") {
       ParseStatus (child, kind);
    }
  }
}

void
NetworkIF::ParseUpdate (QDomDocument & doc, TimelineKind kind)
{
  QDomElement root = doc.documentElement();
  if (root.tagName() == "status") {  // should be
     ParseStatus (root, kind);
  }
}

void
NetworkIF::ParseStatus (QDomElement & elt, TimelineKind kind)
{
  StatusBlock  block (elt);
  if (block.HasValue ("id")) {
    emit NewStatusItem (block, kind);
  }
}

void
NetworkIF::PushUserStatus (QString status)
{
  QByteArray encoded = QUrl::toPercentEncoding (status);
  QUrl url ("http://api.twitter.com/1/statuses/update.xml");
  url.addEncodedQueryItem (QString("status").toLocal8Bit(),
                           encoded);
  QNetworkRequest  req(url);
  QByteArray nada;

  QNetworkReply * reply = network.post (req,nada);

  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  chronicon::R_Update); 
  ExpectReply (reply, chReply);
}

void
NetworkIF::ExpectReply (QNetworkReply *reply, ChronNetworkReply * chReply)
{
  replies[reply] = chReply;
  connect (reply, SIGNAL (finished()), chReply, SLOT (handleReturn()));
  connect (chReply, SIGNAL (Finished(ChronNetworkReply*)),
           this, SLOT (handleReply (ChronNetworkReply*)));
  connect (reply, SIGNAL(error(QNetworkReply::NetworkError)),
         this, SLOT(networkError(QNetworkReply::NetworkError)));
}

void
NetworkIF::CleanupReply (QNetworkReply * reply, ChronNetworkReply *chReply)
{
  if (reply && chReply) {
    replies.erase (reply);
    disconnect (reply, SIGNAL (finished()), chReply, SLOT (handleReturn()));
    disconnect (chReply, SIGNAL (Finished(ChronNetworkReply*)),
           this, SLOT (handleReply (ChronNetworkReply*)));
    disconnect (reply, SIGNAL(error(QNetworkReply::NetworkError)),
           this, SLOT(networkError(QNetworkReply::NetworkError)));
    delete chReply;
    reply->deleteLater ();
  }
}


} // namespace

