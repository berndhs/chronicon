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
#include <QAbstractNetworkCache>

using namespace deliberate;

namespace chronicon {

NetworkIF::NetworkIF (QWidget *parent)
:QObject(parent),
 parentWidget (parent),
 nam(0),
 serviceKind (R_Public),
 askUser (parent,this),
 weblogin (parent),
 user (QString()),
 pass (QString()),
 authRetries (0),
 insideLogin (false),
 numItems (25)
{
  serverRoot = QString ("http://api.twitter.com/1/");
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
qDebug () << " new network " << nam;
  return nam;
}

QString
NetworkIF::Service (QString path)
{
  static QString slash ("/");
  return serverRoot + slash + path;
}

QUrl
NetworkIF::ServiceUrl (QString path)
{
  return QUrl (Service(path));
}

void
NetworkIF::SetServiceRoot (QString root)
{
  serverRoot = root;
  Settings().setValue ("network/service",serverRoot);
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
  qDebug () << " reset network at " << nam;
  if (nam) {
    ReplyMapType::iterator rit;
    for (rit = twitterReplies.begin(); rit != twitterReplies.end(); rit++) {
      rit->second->Abort ();
      disconnect (rit->first, 0,0,0);
      disconnect (rit->second, 0,0,0);
    }
    BitlyMapType::iterator brit;
    for (brit = bitlyReplies.begin(); brit != bitlyReplies.end(); brit++) {
      rit->second->Abort ();
      disconnect (brit->first, 0,0,0);
      disconnect (brit->second, 0,0,0);
    }
    bitlyReplies.clear ();
    QAbstractNetworkCache * cache = nam->cache();
    if (cache) {
      cache->clear();
    }
    oldNAMs << nam;
    nam = 0;
  }
}

void
NetworkIF::SetTimeline (TimelineKind kind)
{
  serviceKind = kind;
  SwitchTimeline ();
}

void
NetworkIF::Init ()
{
  numItems = Settings().value ("network/numitems",numItems).toInt();
  Settings ().setValue ("network/numitems",numItems);
  serverRoot = Settings().value ("network/service",serverRoot).toString();
  Settings ().setValue ("network/service",serverRoot);
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
  QUrl url  (QString(Service ("statuses/%1.xml?count=%2"))
                    .arg(timelineName).arg(numItems));
  request.setUrl(url);
  QNetworkReply *reply = Network()->get (request);
  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  serviceKind);
  ExpectReply (reply, chReply);
qDebug () << " pull for " << reply->url();
}

void
NetworkIF::TestBasicAuth (QString us, QString pa)
{
  user = us;
  pass = pa;
  QNetworkRequest request;
  QUrl url (QString (Service("account/verify_credentials.xml")));
  request.setUrl (url);
  QNetworkReply * reply = Network()->get (request);
  ChronNetworkReply * chReply = new ChronNetworkReply 
                                 (url, reply, R_None, A_AuthVerify);
  ExpectReply (reply, chReply);
  connect (chReply, SIGNAL (AuthVerifyError (ChronNetworkReply *,
                              int )),
           this,    SLOT (twitterAuthBad (ChronNetworkReply*, 
                               int )));
  connect (chReply, SIGNAL (AuthVerifyGood (ChronNetworkReply *)),
            this, SLOT (twitterAuthGood (ChronNetworkReply *)));
  chReply->SetTimeout (30*1000);
}

void
NetworkIF::handleReply (ChronNetworkReply * reply)
{
  if (reply) {
    QNetworkReply * netReply = reply->NetReply();
    TimelineKind kind (reply->Kind());
    ApiRequestKind ark (reply->ARKind());
    if (ark == A_AuthVerify) {
      if (reply->error() == 0) {
         emit TwitterAuthGood ();
      } else {
         emit TwitterAuthBad ();
      }
    } else if (kind == R_Public || kind == R_Private) {
      QDomDocument doc;
      doc.setContent (netReply);
      ParseTwitterDoc (doc, kind);
      emit ReplyComplete (kind);
    } else if (kind == R_Update) {
      QDomDocument update;
      update.setContent (netReply);
      ParseUpdate (update, kind);
      emit ReplyComplete (kind);
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
NetworkIF::networkError (QNetworkReply::NetworkError err)
{
  networkErrorInt (A_None, err);
}

void
NetworkIF::networkErrorInt (ApiRequestKind ark, int err)
{   
  qDebug () << __FILE__ << __LINE__ << " CHRON network error " << err;
  if (err == QNetworkReply::AuthenticationRequiredError) {
     twitterAuthBad (0,err);
  }
}

void
NetworkIF::twitterAuthBad (ChronNetworkReply * reply, int err)
{
  qDebug () << " twitter auth is bad " << err << " for reply " << reply;
  if (insideLogin) {
    askUser.AuthBad ();
  } else {
    QTimer::singleShot (500,this,SLOT (login()));
  }
}

void
NetworkIF::twitterAuthGood (ChronNetworkReply * reply)
{
  if (insideLogin) {
    askUser.AuthOK ();
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
#if 0
  OAuth silliness to go here
  weblogin.Start (QString ("http://mobile.twitter.com"));
#endif
  insideLogin = true;
  int response = askUser.Exec (user);
  QString oldUser (user);
  QString oldPass (pass);
  switch (response) {
  case 1:
     ResetNetwork ();
     user = askUser.User ();
     pass = askUser.Pass ();
     if (user != oldUser) {
       ResetNetwork ();
     }
     if (user == "") {
       serviceKind = R_Public;
     } else {
       serviceKind = R_Private;
     }
     authRetries = 1;
     SwitchTimeline ();
     emit ClearList ();
     emit RePoll (serviceKind);
     deliberate::Settings().setValue ("network/lastuser", user);
     Settings().sync();
     break;
  case -1:
     PushTwitterLogout ();
     ResetNetwork ();
     user = "";
     pass = "";
     serviceKind = R_Public;
     SwitchTimeline ();
     emit ClearList ();
     emit RePoll (serviceKind);
     deliberate::Settings().setValue ("network/lastuser",user);
     Settings().sync ();
     break;
  case 0:
  default:
     break;
  }
  if (reply) {
    *reply = response;
  }
  insideLogin = false;
}


void
NetworkIF::twitterAuthProvide (QNetworkReply *reply, QAuthenticator *au)
{
  if (reply && au) {
    if (pass == QString("") || user == QString("")) {
       return ; // no point, will just be denied
    }
    au->setPassword (pass);
    au->setUser (user);
  }
}

void
NetworkIF::bitlyAuthProvide (QNetworkReply *reply, QAuthenticator *au)
{
  if (au) {
    QString bitly_user = Settings().value ("network/bitly_user",
                               QString("anonymous")).toString();
    QString bitly_key  = Settings().value ("network/bitly_key",
                               QString()).toString();
    au->setUser (bitly_user);
    au->setPassword (bitly_key);
  }
}

void
NetworkIF::authProvide (QNetworkReply *reply, QAuthenticator *au)
{
qDebug () << " asking for auth: " << reply->url();
qDebug () << " current service " << Service();
qDebug () << " serviceUrl.host " << ServiceUrl().host();
  if (reply) {
    QUrl url = reply->url();
    QString host = url.host();
    if (host.contains ("api.bit.ly")) {
       bitlyAuthProvide (reply,au);
    } else if (host.contains (ServiceUrl().host())) {
       twitterAuthProvide (reply, au);
    }
  } 
}

void
NetworkIF::ParseBitlyDoc (QDomDocument & doc, 
                          QString & shortUrl,
                          QString & longUrl,
                          bool & good)
{
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
  QString bitly_user = Settings().value ("network/bitly_user",
                             QString("anonymous")).toString();
  QString bitly_key  = Settings().value ("network/bitly_key",
                             QString()).toString();
  QString request = requestPat.arg (bitly_user)
                              .arg (bitly_key)
                              .arg (http);
  QUrl url (request);
  QNetworkRequest req (url);
  QNetworkReply * reply = Network()->get (req);
  BitlyNetworkReply * bitlyReply = 
              new BitlyNetworkReply (url, reply, tag);
  ExpectReply (reply, bitlyReply);
}

void
NetworkIF::PushTwitterLogout ()
{
  QNetworkRequest request;
  QUrl url (Service("account/end_session.xml"));
  request.setUrl (url);
  QByteArray nada;
  QNetworkReply * reply = Network()->post (request, nada);
  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                  reply, chronicon::R_None,
                                  A_Logout);
  ExpectReply (reply, chReply);
}


void
NetworkIF::PushUserStatus (QString status)
{
  QByteArray encoded = QUrl::toPercentEncoding (status);
  QUrl url (Service ("statuses/update.xml"));
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
  QString urlString (Service ("statuses/destroy.xml?id=%1"));
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
  QString urlString (Service ("statuses/retweet/%1.xml"));
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
         chReply, SLOT(handleError(QNetworkReply::NetworkError)));
  connect (chReply, SIGNAL (networkError (ApiRequestKind, int)),
           this, SLOT (networkErrorInt (ApiRequestKind, int)));
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
    disconnect (reply, 0, 0, 0);
    disconnect (chReply, 0, 0, 0);
    delete chReply;
    reply->deleteLater ();
  }
}

void
NetworkIF::CleanupReply (QNetworkReply * reply, BitlyNetworkReply *bitReply)
{
  if (reply && bitReply) {
    bitlyReplies.erase (reply);
    disconnect (reply, 0, 0, 0);
    disconnect (bitReply, 0, 0, 0);
    delete bitReply;
    reply->deleteLater ();
  }
}



} // namespace

