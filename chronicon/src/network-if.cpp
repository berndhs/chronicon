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
#include "login-dialog.h"
#include <QDomDocument>
#include <QDateTime>

using namespace deliberate;

namespace chronicon {

NetworkIF::NetworkIF (QWidget *parent)
:QObject(parent),
 parentWidget (parent),
 nam(0),
 serviceKind (R_Public),
 user (QString()),
 pass (QString()),
 numItems (25)
{
  serviceKind = R_Private;
  SwitchTimeline();
}

QNetworkAccessManager *
NetworkIF::Network ()
{
  if (nam != 0) {
    return nam;
  }
  nam = new QNetworkAccessManager (this);
  ConnectNetwork ();
  return nam;
}

void
NetworkIF::ConnectNetwork ()
{
  connect (nam, SIGNAL (authenticationRequired(QNetworkReply*, QAuthenticator*)),
           this, SLOT (authProvide (QNetworkReply*, QAuthenticator*)));
}

void
NetworkIF::ResetNetwork ()
{
  if (nam) {
    ReplyMapType::iterator rit;
    for (rit = twitterReplies.begin(); rit != twitterReplies.end(); rit++) {
      disconnect (rit->first, 0,0,0);
      disconnect (rit->second, 0,0,0);
      rit->first->close();
      rit->first->deleteLater();
      delete rit->second;
    }
    twitterReplies.clear ();
    BitlyMapType::iterator brit;
    for (brit = bitlyReplies.begin(); brit != bitlyReplies.end(); brit++) {
      disconnect (brit->first, 0,0,0);
      disconnect (brit->second, 0,0,0);
      brit->first->close();
      brit->first->deleteLater();
      delete brit->second;
    }
    bitlyReplies.clear ();
    delete nam;
    nam = 0;
  }
}

void
NetworkIF::Init ()
{
  numItems = Settings().value ("network/numitems",numItems).toInt();
  Settings ().setValue ("network/numitems",numItems);
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
  QUrl url  (QString("http://api.twitter.com/1/statuses/%1.xml?count=%2")
                    .arg(timelineName).arg(numItems));
  request.setUrl(url);
  QNetworkReply *reply = Network()->get (request);
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
      ParseTwitterDoc (doc, kind);
      emit ReplyComplete ();
    } else if (kind == R_Update) {
      QDomDocument update;
      update.setContent (netReply);
      ParseUpdate (update, kind);
      emit ReplyComplete ();
    }
    ReplyMapType::iterator index;
    index = twitterReplies.find (netReply);
    if (index != twitterReplies.end()) {
      twitterReplies.erase(index);
    }
    CleanupReply (netReply, reply);    
  }
}


void
NetworkIF::handleReply (BitlyNetworkReply * reply)
{
  if (reply) {
    QNetworkReply * netReply = reply->NetReply();
    QDomDocument doc;
    doc.setContent (netReply);
    QString shortUrl, longUrl;
    bool good;
    ParseBitlyDoc (doc, shortUrl, longUrl, good);
    emit ShortenReply (reply->Tag(), shortUrl, longUrl, good);

    BitlyMapType::iterator index;
    index = bitlyReplies.find (netReply);
    if (index != bitlyReplies.end()) {
      bitlyReplies.erase(index);
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
  LoginDialog  askUser (parentWidget);
  int response = askUser.Exec (user);
  switch (response) {
  case 1:
     user = askUser.User ();
     pass = askUser.Pass ();
     serviceKind = R_Private;
     SwitchTimeline ();
     emit RePoll (serviceKind);
     deliberate::Settings().setValue ("lastuser", user);
     Settings().sync();
     break;
  case -1:
     user = "";
     pass = "";
     serviceKind = R_Public;
     SwitchTimeline ();
     ResetNetwork ();
     emit RePoll (serviceKind);
     deliberate::Settings().setValue ("lastuser",user);
     Settings().sync ();
     break;
  case 0:
  default:
     break;
  }
  if (reply) {
    *reply = response;
  }
}

void
NetworkIF::twitterAuthProvide (QNetworkReply *reply, QAuthenticator *au)
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
NetworkIF::bitlyAuthProvide (QNetworkReply *reply, QAuthenticator *au)
{
  if (au) {
    au->setUser (QString("berndhs"));
    au->setPassword (QString("R_8c4fa0c06974f73aa61fc1e9742e98c5"));
  }
qDebug () << " sent bitly auth " ;
}

void
NetworkIF::authProvide (QNetworkReply *reply, QAuthenticator *au)
{
  if (reply) {
    QUrl url = reply->url();
    QString host = url.host();
    if (host.contains ("api.bit.ly")) {
       bitlyAuthProvide (reply,au);
    } else if (host.contains ("twitter.com")) {
       twitterAuthProvide (reply, au);
    }
  } 
}

void
NetworkIF::networkError (QNetworkReply::NetworkError err)
{
  qDebug () << "network error " << err;
}

void
NetworkIF::ParseBitlyDoc (QDomDocument & doc, 
                          QString & shortUrl,
                          QString & longUrl,
                          bool & good)
{
qDebug () << __FILE__ << __LINE__ << " short " << shortUrl << " long " << longUrl;
  QDomElement root = doc.documentElement ();
  QDomElement child;
  QString     tag;
  QString     status;
  for (child = root.firstChildElement(); !child.isNull(); 
       child = child.nextSiblingElement()) {
    tag = child.tagName();
    if (tag == "status_code") {
      status = child.text();
    } else if (tag == "status_txt") {
      qDebug () << " status text " << child.text();
    } else if (tag == "data") {
      ParseBitlyData (child, shortUrl, longUrl, good);
    }
  }
}

void
NetworkIF::ParseBitlyData (QDomElement & data, 
                           QString & shortUrl,
                           QString & longUrl,
                           bool & good)
{
  QDomElement child;
  QString tag;
  for (child = data.firstChildElement(); !child.isNull(); 
       child = child.nextSiblingElement()) {
    tag = child.tagName();
    if (tag == "url") {
      shortUrl = child.text();
    } else if (tag == "long_url") {
      longUrl = child.text ();
    }
  }
  good = (shortUrl.length() > 0) && longUrl.length();
}

void
NetworkIF::ParseTwitterDoc (QDomDocument & doc, TimelineKind kind)
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
NetworkIF::ShortenHttp (int tag, QStringList httpList)
{
  QStringList::iterator index;
  for (index = httpList.begin(); index != httpList.end(); index++) {
    AskBitly (tag, *index);
  }
  
}

void
NetworkIF::AskBitly (int tag, QString http)
{
  QString requestPat 
        ("http://api.bit.ly/v3/shorten?login=%1&apiKey=%2&format=xml&uri=%3");
  QString bitlyUser ("berndhs");
  QString bitlyKey ("R_8c4fa0c06974f73aa61fc1e9742e98c5");
  QString request = requestPat.arg (bitlyUser)
                              .arg (bitlyKey)
                              .arg (http);
  qDebug () << " will ask bitly " << request;
  QUrl url (request);
  QNetworkRequest req (url);
  QNetworkReply * reply = Network()->get (req);
  BitlyNetworkReply * bitlyReply = 
              new BitlyNetworkReply (url, reply, tag);
  ExpectReply (reply, bitlyReply);
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

  QNetworkReply * reply = Network()->post (req,nada);

  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  chronicon::R_Update); 
  ExpectReply (reply, chReply);
}

void
NetworkIF::PushDelete (QString id)
{
  QString urlString ("http://api.twitter.com/1/statuses/destroy.xml?id=%1");
  QUrl url (urlString.arg(id));
  QNetworkRequest  req(url);
  QByteArray nada;

  QNetworkReply * reply = Network()->post (req,nada);

  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  chronicon::R_Destroy); 
  ExpectReply (reply, chReply);
}


void
NetworkIF::ReTweet (QString id)
{
  QString urlString ("http://api.twitter.com/1/statuses/retweet/%1.xml");
  QUrl url (urlString.arg(id));
  QNetworkRequest  req(url);
  QByteArray nada;
  QNetworkReply * reply = Network()->post (req, nada);
  ChronNetworkReply * chReply = new ChronNetworkReply (url,
                                        reply,
                                        chronicon::R_Update);
  ExpectReply (reply, chReply);
}

void
NetworkIF::ExpectReply (QNetworkReply *reply, ChronNetworkReply * chReply)
{
  twitterReplies[reply] = chReply;
  connect (reply, SIGNAL (finished()), chReply, SLOT (handleReturn()));
  connect (chReply, SIGNAL (Finished(ChronNetworkReply*)),
           this, SLOT (handleReply (ChronNetworkReply*)));
  connect (reply, SIGNAL(error(QNetworkReply::NetworkError)),
         this, SLOT(networkError(QNetworkReply::NetworkError)));
}

void
NetworkIF::ExpectReply (QNetworkReply *reply, BitlyNetworkReply * bitReply)
{
  bitlyReplies[reply] = bitReply;
  connect (reply, SIGNAL (finished()), bitReply, SLOT (handleReturn()));
  connect (bitReply, SIGNAL (Finished(BitlyNetworkReply*)),
           this, SLOT (handleReply (BitlyNetworkReply*)));
  connect (reply, SIGNAL(error(QNetworkReply::NetworkError)),
         this, SLOT(networkError(QNetworkReply::NetworkError)));
}

void
NetworkIF::CleanupReply (QNetworkReply * reply, ChronNetworkReply *chReply)
{
  if (reply && chReply) {
    twitterReplies.erase (reply);
    disconnect (reply, SIGNAL (finished()), chReply, SLOT (handleReturn()));
    disconnect (chReply, SIGNAL (Finished(ChronNetworkReply*)),
           this, SLOT (handleReply (ChronNetworkReply*)));
    disconnect (reply, SIGNAL(error(QNetworkReply::NetworkError)),
           this, SLOT(networkError(QNetworkReply::NetworkError)));
    delete chReply;
    reply->deleteLater ();
  }
}

void
NetworkIF::CleanupReply (QNetworkReply * reply, BitlyNetworkReply *bitReply)
{
  if (reply && bitReply) {
    bitlyReplies.erase (reply);
    disconnect (reply, SIGNAL (finished()), bitReply, SLOT (handleReturn()));
    disconnect (bitReply, SIGNAL (Finished(BitlyNetworkReply*)),
           this, SLOT (handleReply (BitlyNetworkReply*)));
    disconnect (reply, SIGNAL(error(QNetworkReply::NetworkError)),
           this, SLOT(networkError(QNetworkReply::NetworkError)));
    delete bitReply;
    reply->deleteLater ();
  }
}



} // namespace

