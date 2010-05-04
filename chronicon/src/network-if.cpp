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
  SwitchTimeline();
  connect (&network, SIGNAL (authenticationRequired(QNetworkReply*, QAuthenticator*)),
           this, SLOT (authProvide (QNetworkReply*, QAuthenticator*)));
}

void
NetworkIF::SwitchTimeline ()
{
  switch (serviceKind) {
    case R_Private:
      timelineName = "user_timeline";
      break;
    case R_Public:
    default:
      timelineName = "public_timeline";
      break;
  }
}

void
NetworkIF::PullPublicTimeline ()
{
  QNetworkRequest request;
  QUrl url  (QString("http://api.twitter.com/1/statuses/%1.xml")
                    .arg(timelineName));
  request.setUrl(url);
  QNetworkReply *reply = network.get (request);
  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  serviceKind);
  replies[reply] = chReply;
  connect (reply, SIGNAL (finished()), chReply, SLOT (handleReturn()));
  connect (chReply, SIGNAL (Finished(ChronNetworkReply*)),
           this, SLOT (handleReply (ChronNetworkReply*)));
  connect (reply, SIGNAL(error(QNetworkReply::NetworkError)),
         this, SLOT(networkError(QNetworkReply::NetworkError)));
}

void
NetworkIF::handleReply (ChronNetworkReply * reply)
{
  if (reply) {
    QNetworkReply * netReply = reply->NetReply();
    TimelineKind kind (reply->Kind());
    if (kind == chronicon::R_Public || kind == chronicon::R_Private) {
      QDomDocument doc;
      bool ok = doc.setContent (netReply);   
      ParseDom (doc, kind);
      emit ReplyComplete ();
    }
    ReplyMapType::iterator index;
    index = replies.find (netReply);
    if (index != replies.end()) {
      replies.erase(index);
    }
    delete reply;
    if (netReply) {
      netReply->deleteLater();
    }
  }
}

void
NetworkIF::login (int * reply)
{
  Ui_TextEnter textenter;
  QDialog  textDialog;
  textenter.setupUi (&textDialog);
  textenter.passEdit->setEchoMode (QLineEdit::Password);
  int ok = textDialog.exec ();
  if (ok) {
    user = textenter.userEdit->text();
    pass = textenter.passEdit->text();
    serviceKind = R_Private;
    SwitchTimeline ();
  } else {
    serviceKind = R_Public;
    SwitchTimeline ();
  }
  if (reply) {
    *reply = ok;
  }
}

void
NetworkIF::authProvide (QNetworkReply *reply, QAuthenticator *authenticator)
{
  if (reply && authenticator) {
    int tryAgain (0);
    login (&tryAgain);
    if (tryAgain == 0) {
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
NetworkIF::ParseStatus (QDomElement & elt, TimelineKind kind)
{
  QDomElement  sub;
  QString      id ("0");
  for (sub = elt.firstChildElement(); !sub.isNull();
       sub = sub.nextSiblingElement()) {
    if (sub.tagName() == "id") {
      id = sub.text();
      break;
    }
  }
  if (id != "0") {
    StatusBlock block (id,elt);
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
}

} // namespace

