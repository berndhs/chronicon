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
#include <QMessageBox>
#include <qjson/parser.h>
#include <QFile>

using namespace deliberate;

namespace chronicon {

NetworkIF::NetworkIF (QWidget *parent)
:QObject(parent),
 parentWidget (parent),
 nam(0),
 serviceKind (R_Public),
 plainLoginDialog (parent,this),
 webAuth (this),
 webLoginDialog (parent,&webAuth),
 user (QString()),
 pass (QString()),
 authRetries (0),
 insideLogin (false),
 haveUser (false),
 numItems (25),
 myName ("Chronicon"),
 acc_token (""),
 acc_secret ("")
{
  serverRoot = QString ("http://api.twitter.com/1/");
  searchRoot = QString ("http://search.twitter.com/");
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
NetworkIF::OAuthService (QString path)
{
  static QString oauthServerRoot ("https://api.twitter.com/1");
  static QString slash ('/');
  if (oauthServerRoot.endsWith ('/') || path.startsWith('/')) {
    return oauthServerRoot + path;
  } else {
    return oauthServerRoot + slash + path;
  }
}

QString
NetworkIF::Service (QString path)
{
  static QString slash ('/');
  if (serverRoot.endsWith ('/') || path.startsWith('/')) {
    return serverRoot + path;
  } else {
    return serverRoot + slash + path;
  }
}

QUrl
NetworkIF::ServiceUrl (QString path)
{
  return QUrl (Service(path));
}

QString
NetworkIF::SearchService (QString path)
{
  static QString slash ('/');
  if (searchRoot.endsWith ('/') || path.startsWith('/')) {
    return searchRoot + path;
  } else {
    return searchRoot + slash + path;
  }
}

void
NetworkIF::SetServiceRoot (QString root)
{
  serverRoot = root;
  Settings().setValue ("network/service",serverRoot);
}

void
NetworkIF::SetSearchRoot (QString sroot)
{
  searchRoot = sroot;
  Settings().setValue ("network/searchservice",searchRoot);
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
      disconnect (rit->first, 0,0,0);
      disconnect (rit->second, 0,0,0);
    }
    BitlyMapType::iterator brit;
    for (brit = bitlyReplies.begin(); brit != bitlyReplies.end(); brit++) {
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
}

void
NetworkIF::Init ()
{
  numItems = Settings().value ("network/numitems",numItems).toInt();
  Settings ().setValue ("network/numitems",numItems);
  serverRoot = Settings().value ("network/service",serverRoot).toString();
  Settings ().setValue ("network/service",serverRoot);
  searchRoot = Settings().value ("network/searchservice",searchRoot).toString();
  Settings ().setValue("network/searchservice",searchRoot);
  myName = Settings().value ("program",myName).toString();
  webAuth.Init ();
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
qDebug () << " net reply at " << __FILE__ << __LINE__ ;
qDebug () << " net reply Status header " << netReply->rawHeader ("Status");
qDebug () << " net reply Content-Type " << netReply->rawHeader ("Content-Type");
qDebug () << " net reply X-Ratelimit-Remaining " << netReply->rawHeader ("X-Ratelimit-Remaining");
qDebug () << " net reply X-Ratelimit-Limit " << netReply->rawHeader ("X-Ratelimit-Limit");
    TimelineKind kind (reply->Kind());
qDebug () << " net reply is for " << timelineName (kind);
    ApiRequestKind ark (reply->ARKind());
qDebug () << " net reply ark is " << ark;

    QDomDocument doc;
    switch (ark) {
    case A_AuthVerify:
      if (reply->error() == 0) {
         emit TwitterAuthGood ();
      } else {
         emit TwitterAuthBad ();
      }
      break;
    case A_UserInfo:
      doc.setContent (netReply);
      ParseUserInfo (doc);
      break;
    case A_Search:
      ParseSearchResult (netReply);
      emit ReplyComplete (R_SearchResults);
      break;
    case A_Timeline:
    default:
      doc.setContent (netReply);
      switch (kind) {
      case R_Public:
      case R_Private:
      case R_Mentions:
      case R_OwnRetweets:
      case R_FriendRetweets:
        ParseTwitterDoc (doc, kind);
        emit ReplyComplete (kind);
        break;
      case R_Update:
        ParseUpdate (doc, kind);
        emit ReplyComplete (kind);
        break;
      case R_ThisUser:
      case R_OtherUser:
        ParseUserBlock (doc, kind);
        emit ReplyComplete (kind);
        break;
      case R_Ignore:
        qDebug () << " ignoring reply from " << netReply->url();
        qDebug () << " net error " << netReply->error ();
        qDebug () << " xml content " << doc.toString();
        break;
      default:
        // ignore
        break;
      }
      break;
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
  QString plain("basic"), oauth ("oauth");
  QString logintype (plain);
  logintype = Settings().value ("network/login_type",logintype).toString().toLower();
  if (insideLogin) {
    plainLoginDialog.AuthBad ();
  } else if (logintype == plain ){
    QTimer::singleShot (500,this,SLOT (login()));
  }
}

void
NetworkIF::twitterAuthGood (ChronNetworkReply * reply)
{
  if (insideLogin) {
    plainLoginDialog.AuthOK ();
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
  QString plain("basic"), oauth ("oauth");
  QString logintype (plain);
  logintype = Settings().value ("network/login_type",logintype).toString().toLower();
  if (logintype == plain) {
    Settings().setValue ("network/login_type",logintype);
    plainLogin (reply);
  } else if (logintype == oauth) {
    Settings().setValue ("network/login_type",logintype);
    webLogin (reply);
  } else {
    QMessageBox badconfig (parentWidget);
    QString message (tr("Bad login_type config %1\nnetwork/login_type must be %1 or %2"));
    badconfig.setText (message.arg(plain).arg(oauth));
    badconfig.exec ();
    reply = 0;
    user = "";
  }
}

void
NetworkIF::AutoLogin (QByteArray u, QByteArray key1, QByteArray key2, bool oauth)
{
  oauthMode = oauth;
  user = QString (u);
  if (oauth) {
    acc_token = key1;
    acc_secret = key2;
  } else {
    pass = QString (key1);
  }
  haveUser = true;
}

void
NetworkIF::webLogin (int * reply)
{

  emit StopPoll (true);
  if (reply ) {
    *reply = 0;
  }
  webLoginDialog.Start ();
  if (webLoginDialog.IsValid()) {
    user = webLoginDialog.User ();
    acc_token = webLoginDialog.AccToken ();
    acc_secret = webLoginDialog.AccSecret ();
    oauthMode = true;
    if (reply ) {
      *reply = 1;
    }
    serviceKind = R_Private;
    emit ClearList ();
    emit RePoll (serviceKind);
  }
  emit StopPoll (false);
}

void
NetworkIF::plainLogin (int * reply)
{
  insideLogin = true;
  emit StopPoll (true);
  QString oldUser (user);
  QString oldPass (pass);
  int response = plainLoginDialog.Exec (user);
  switch (response) {
  case 1:
     ResetNetwork ();
     oauthMode = false;
     user = plainLoginDialog.User ();
     pass = plainLoginDialog.Pass ();
     if (user != oldUser) {
       ResetNetwork ();
     }
     if (user == "") {
       serviceKind = R_Public;
     } else {
       serviceKind = R_Private;
     }
     authRetries = 1;
     emit ClearList ();
     emit RePoll (serviceKind);
     deliberate::Settings().setValue ("network/lastuser", user);
     Settings().sync();
     haveUser = true;
     break;
  case -1:
     PushTwitterLogout ();
     ResetNetwork ();
     user = "";
     pass = "";
     serviceKind = R_Public;
     emit ClearList ();
     emit RePoll (serviceKind);
     deliberate::Settings().setValue ("network/lastuser",user);
     Settings().sync ();
     haveUser = false;
     break;
  case 0:
  default:
     break;
  }
  if (reply) {
    *reply = response;
  }
  insideLogin = false;
  emit StopPoll (false);
}


void
NetworkIF::twitterAuthProvide (QNetworkReply *reply, QAuthenticator *au)
{
  if (oauthMode) {
    return;
  } else if (reply && au) {
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
NetworkIF::ParseUserBlock (QDomDocument & doc, TimelineKind kind)
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
NetworkIF::ParseSearchResult (QNetworkReply * reply)
{
  StatusBlock block;
  QJson::Parser pars;
  bool good (false);
  QVariant parts = pars.parse (reply, &good);
  
  if (good) {
    if (parts.type() == QMetaType::QVariantMap) {
      QVariantMap partsMap = parts.toMap();
      QVariantMap::const_iterator mit;
      for (mit = partsMap.begin(); mit != partsMap.end(); mit++) {
         if (mit.key() == "results") {
           ParseSearchResultList (mit.value());
         } else {
         qDebug () << "map entry [" << mit.key() << " => " << mit.value() << "]";
         }
      }
    }
  }
  emit SearchComplete ();
}

void
NetworkIF::ParseSearchResultList (const QVariant & resList)
{
  if (resList.type() != QMetaType::QVariantList) {
    qDebug () << __FILE__ << __LINE__ << " bad result type";
    return;
  }
  const QVariantList & res = resList.toList();
  QVariantList::const_iterator lit;
  for (lit = res.begin(); lit != res.end(); lit++) {
    if (lit->type() == QMetaType::QVariantMap) {
      StatusBlock  block;
      block.SetSearchContent (lit->toMap());
      emit NewStatusItem (block, R_SearchResults);
    }
  }
}


void
NetworkIF::ParseUserInfo (QDomDocument & userDoc)
{
  QDomElement root = userDoc.documentElement();
  if (root.tagName() == "user") {
    UserBlock userInfo (root);
    emit NewUserInfo (userInfo);
  } else {
    qDebug () << " bad user block tag " << root.tagName();
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
NetworkIF::ShortenHttp (QUuid tag, QStringList httpList)
{
  QStringList::iterator index;
  for (index = httpList.begin(); index != httpList.end(); index++) {
    AskBitly (tag, *index);
  }
  
}

void
NetworkIF::AskBitly (QUuid tag, QString http)
{
  QString request 
        ("http://api.bit.ly/v3/shorten");
  QUrl url (request);
  url.addQueryItem ("login",Settings().value ("network/bitly_user",
                             QString("anonymous")).toString());
  url.addQueryItem ("apiKey",Settings().value ("network/bitly_key",
                             QString()).toString());
  url.addQueryItem ("format","xml");
  url.addEncodedQueryItem ("uri",QUrl::toPercentEncoding (http));
  QNetworkRequest req (url);
  QNetworkReply * reply = Network()->get (req);
  BitlyNetworkReply * bitlyReply = 
              new BitlyNetworkReply (url, reply, tag);
  ExpectReply (reply, bitlyReply);
}


void
NetworkIF::PullTimeline ()
{
  if (oauthMode) {
    PullTimelineOA ();
  } else {
    PullTimelineBasic ();
  }
}

void
NetworkIF::PullTimeline (QString otherUser)
{
  if (oauthMode) {
    PullTimelineOA (otherUser);
  } else {
    PullTimelineBasic (otherUser);
  }
}

void
NetworkIF::PullTimelineBasic (QString otherUser)
{
  QNetworkRequest request;
  QUrl url  (QString(Service ("statuses/%1.xml"))
                    .arg(timelineName (serviceKind)));
  url.addQueryItem ("count",QString::number (numItems));
  if (otherUser.length() > 0 ) {
    url.addQueryItem ("screen_name",otherUser);
  }

  request.setUrl(url);
  request.setRawHeader ("User-Agent","Chronicon; WebKit");
  DebugShow (request);
  QNetworkReply *reply = Network()->get (request);
  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  serviceKind,
                                                  A_Timeline);
  ExpectReply (reply, chReply);
qDebug () << " basic pull timeline GET " << timelineName (serviceKind);
qDebug () << " GET for " << reply->url();
}

void
NetworkIF::PullTimelineOA (QString otherUser)
{
  QString urlString = OAuthService ("statuses/%1.xml")
                              .arg(timelineName (serviceKind));
  QOAuth::ParamMap  args;
  args.insert ("count",QString::number(numItems).toUtf8());
  if (otherUser.length() > 0) {
    args.insert ("screen_name",otherUser.toUtf8());
  }
  QByteArray  parms = prepareOAuthString (urlString, 
                                          QOAuth::GET,
                                          args);
  QNetworkRequest req;
  req.setRawHeader ("Authorization", parms);
  urlString.append (webAuth.QOAuth()->inlineParameters 
                      (args, QOAuth::ParseForInlineQuery));
  QUrl url (urlString);
  req.setUrl (url);
  DebugShow (req);
  QNetworkReply *reply = Network()->get (req);
  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  serviceKind,
                                                  A_Timeline);
  ExpectReply (reply, chReply);
qDebug () << " oauth pull timeline GET " << timelineName (serviceKind);
qDebug () << " GET for " << reply->url();
  
}

void
NetworkIF::PullSearch (QString needle)
{
qDebug () << " pull search for " << needle;
  QString urlString = SearchService ("search.json");
  QUrl url(urlString);
  url.addQueryItem ("q",needle);
  QNetworkRequest req (url);
  req.setRawHeader ("User-Agent","Chronicon; WebKit");
  DebugShow (req);
  QNetworkReply *reply = Network()->get (req);
  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  R_None,
                                                  A_Search);
  ExpectReply (reply, chReply);
qDebug () << " basic Search GET " << timelineName (serviceKind);
qDebug () << " GET for " << reply->url();
}


void
NetworkIF::PullUserBlock ()
{
  if (oauthMode) {
    PullUserBlockOA ();
  } else {
    PullUserBlockBasic ();
  }
}

void
NetworkIF::PullUserBlockBasic ()
{
  QNetworkRequest request;
  QUrl url  (Service ("users/show.xml"));
  url.addQueryItem ("screen_name",user);
  request.setUrl(url);
  request.setRawHeader ("User-Agent","Chronicon; WebKit");
  DebugShow (request);
  QNetworkReply *reply = Network()->get (request);
  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  R_None,
                                                  A_UserInfo);
  ExpectReply (reply, chReply);
}

void
NetworkIF::PullUserBlockOA ()
{
  QString urlString = OAuthService ("users/show.xml");
  QOAuth::ParamMap  args;
  args.insert ("screen_name",QUrl::toPercentEncoding(user.toUtf8()));
  QByteArray  parms = prepareOAuthString (urlString, 
                                          QOAuth::GET,
                                          args);
  QNetworkRequest req;
  req.setRawHeader ("Authorization", parms);
  urlString.append (webAuth.QOAuth()->inlineParameters 
                      (args, QOAuth::ParseForInlineQuery));
  QUrl url (urlString);
  req.setUrl (url);
  DebugShow (req);
  QNetworkReply *reply = Network()->get (req);
  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  R_None,
                                                  A_UserInfo);
  ExpectReply (reply, chReply);
  
}


void
NetworkIF::PushTwitterLogout ()
{
  if (oauthMode) {
    return;
  }
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
NetworkIF::PushUserStatus (QString status, QString refId)
{
  if (oauthMode) {
    PushUserStatusOA (status, refId);
  } else {
    PushUserStatusBasic (status, refId);
  }
}

void
NetworkIF::PushUserStatusBasic (QString status, QString refId)
{
  QByteArray encoded = QUrl::toPercentEncoding (status);
  QUrl url (Service ("statuses/update.xml"));
  url.addEncodedQueryItem (QString("status").toLocal8Bit(),
                           encoded);
  if (refId.length() > 0) {
     encoded = QUrl::toPercentEncoding (refId);
     url.addEncodedQueryItem (QString("in_reply_to_status_id").toUtf8(),
                              encoded);
  }
  QNetworkRequest  req(url);
  QByteArray nada;

  req.setRawHeader ("User-Agent","Chronicon WebKit");
  DebugShow (req);
  PostBasic (url, req, nada, R_Update);
}

void
NetworkIF::PushUserStatusOA (QString status, QString refId)
{
  QOAuth::ParamMap  paramContent;
  QByteArray update = QUrl::toPercentEncoding (status.toUtf8());
  paramContent.insert ("status",update);
  if (refId.length() > 0) {
     paramContent.insert ("in_reply_to_status_id",refId.toUtf8());
  }
  QString urlStr = OAuthService ("statuses/update.xml");
  PostOA (urlStr, paramContent, R_Update);
}

void
NetworkIF::DirectMessage (QString toName, QString msg)
{
  if (oauthMode) {
    DirectMessageOA (toName, msg);
  } else {
    DirectMessageBasic (toName, msg);
  }
}

void
NetworkIF::DirectMessageBasic (QString toName, QString msg)
{
  QString urlString (Service ("direct_messages/new.xml"));
  QUrl  url (urlString);
  url.addQueryItem ("screen_name",toName);
  QByteArray encoded = QUrl::toPercentEncoding (msg);
  url.addEncodedQueryItem ("text",encoded);
  QNetworkRequest req (url);
  QByteArray nada;
  PostBasic (url, req, nada, R_Update);
}

void
NetworkIF::DirectMessageOA (QString toName, QString msg)
{
  QOAuth::ParamMap  paramContent;
  paramContent.insert ("screen_name",QUrl::toPercentEncoding (toName.toUtf8()));
  QByteArray update = QUrl::toPercentEncoding (msg.toUtf8());
  paramContent.insert ("text",update);
  QString urlString (OAuthService ("direct_messages/new.xml"));
  PostOA (urlString, paramContent, R_Update);
}

void
NetworkIF::ChangeFollow (QString otherUser, int change)
{
qDebug () << " NetworkIF::ChangeFollow " << otherUser << " by " << change;
  if (oauthMode) {
    ChangeFollowOA (otherUser, change);
  } else {
    ChangeFollowBasic (otherUser, change);
  }
}

void
NetworkIF::ChangeFollowBasic (QString otherUser, int change)
{
  QString action;
  if (change < 0) {
    action = QString ("friendships/destroy.xml");
  } else if (change > 0) {
    action = QString ("friendships/create/twitterapi.xml");
  } else {
    return;
  }
  QString urlString (Service (action));
  QUrl  url (urlString);
  QByteArray encoUser = QUrl::toPercentEncoding (otherUser);
  url.addEncodedQueryItem ("screen_name",encoUser);
  QNetworkRequest req (url);
  QByteArray nada;
  PostBasic (url, req, nada, R_Ignore);
}

void
NetworkIF::ChangeFollowOA (QString otherName, int change)
{
  QString action;
  if (change < 0) {
    action = QString ("friendships/destroy.xml");
  } else if (change > 0) {
    action = QString ("friendships/create/twitterapi.xml");
  } else {
    return;
  }
  QOAuth::ParamMap  paramContent;
  paramContent.insert ("screen_name",QUrl::toPercentEncoding 
                                     (otherName.toUtf8()));

  QString urlString (OAuthService (action));
qDebug () << " Change url " << urlString;
qDebug () << " Change parms " << paramContent;
  PostOA (urlString, paramContent, R_Ignore);
}

void
NetworkIF::PushDelete (QString id)
{
  if (oauthMode) {
    PushDeleteOA (id);
  } else {
    PushDeleteBasic (id);
  }
}


void
NetworkIF::PushDeleteBasic (QString id)
{
  
  QString urlString (Service ("statuses/destroy.xml?id=%1"));
  QUrl url (urlString.arg(id));
  QNetworkRequest  req(url);
  QByteArray nada;
  PostBasic (url, req, nada, R_Destroy);
}

void
NetworkIF::PushDeleteOA (QString id)
{
  QOAuth::ParamMap  paramContent;
  paramContent.insert ("id",id.toUtf8());
  QString urlStr = OAuthService ("statuses/destroy.xml");
  PostOA (urlStr, paramContent, R_Destroy);
}



void
NetworkIF::ReTweet (QString id)
{
  if (oauthMode) {
    ReTweetOA (id);
  } else {
    ReTweetBasic (id);
  }
}
void
NetworkIF::ReTweetOA (QString id)
{
  QOAuth::ParamMap  paramContent;
  QString urlStr = OAuthService ("statuses/retweet/%1.xml").arg(id);
  PostOA (urlStr, paramContent, R_Update);
}



void
NetworkIF::ReTweetBasic (QString id)
{
  QString urlString (Service ("statuses/retweet/%1.xml"));
  QUrl url (urlString.arg(id));
  QNetworkRequest  req(url);
  QByteArray nada;
  QNetworkReply * reply = Network()->post (req, nada);
  ChronNetworkReply * chReply = new ChronNetworkReply (url,
                                        reply,
                                        chronicon::R_Update, A_Post);
  ExpectReply (reply, chReply);
}



void
NetworkIF::PostBasic (QUrl &url, 
                      QNetworkRequest &req, 
                      QByteArray & data,
                      TimelineKind kind)
{
qDebug () << " debug for post basic: " ;
  DebugShow (req);
  QNetworkReply * reply = Network()->post (req, data);
  ChronNetworkReply * chReply = new ChronNetworkReply (url, reply, kind, A_Post);
  ExpectReply (reply, chReply);
}


void
NetworkIF::PushPicOA (QString picname, QString msg)
{
qDebug () << "!!!!!!!! PushPicOA";
  msg = "testing";
  QByteArray twitPicKey ("20e7048922bdd9a6c41ef2a79c828d53");
  QString picurl ("http://api.twitpic.com/2/upload.xml");
  QString twiturl = OAuthService ("ZZZZZZZZZZZZZZZaccount/verify_credentials.xml");
  QFile file (picname);
  file.open (QFile::ReadOnly);
  QByteArray data = file.readAll ();
  file.close();
  QByteArray mediaData = data.toPercentEncoding();
  QString urlString (twiturl);
  QNetworkRequest req;
  QByteArray boundary ("BownTarriex");
  QOAuth::ParamMap params;
  params.insert ("realm","https://api.twitter.com");
  oauthForPicPost (req, urlString, params, boundary, twiturl);

  QByteArray content;
  boundary.prepend ("--");
  QByteArray endbound = boundary;
  endbound.append ("--\r\n");
  boundary.append ("\r\n");
  content.append (boundary);
  content.append ("content-disposition: form-data; name=\"key\"\r\n");
  content.append (twitPicKey + "\r\n");
  content.append (boundary);

  content.append ("content-disposition: form-data; name=\"message\"\r\n");
  content.append (msg.toUtf8() + "\r\n");

  content.append (boundary);
 
  content.append ("content-dispositon: form-data; name=\"media\"; filename=\""
                  + picname.toUtf8() + "\"\r\n");
  content.append ("content-type: image/jpg\r\n");
  //content.append (data);
  content.append (QByteArray (256,'!'));
qDebug () << " appended " << data.size() << " bytes";
  content.append ("\r\n");
  content.append (endbound);

qDebug () << " POST content size " << content.size();
qDebug () << " POST content " << content;
  QUrl url (picurl);
  url.addQueryItem ("key",twitPicKey);
  req.setUrl (url);
  req.setRawHeader ("key",twitPicKey);
qDebug () << " debug for post twitpic OA: " << urlString;
qDebug () << " url from req " << req.url();
  DebugShow (req);
  QNetworkReply * reply = Network()->post (req, content);  
  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  R_Ignore, A_Post); 
  ExpectReply (reply, chReply);

}



void 
NetworkIF::PostOA (QString  & urlString, 
                   QOAuth::ParamMap & paramContent,
                   TimelineKind      kind)
{
  QNetworkRequest req;
  oauthForPost (req, urlString, paramContent);

  QUrl url (urlString);
  req.setUrl (url);
qDebug () << " debug for post OA: ";
  DebugShow (req);
  QByteArray content = webAuth.QOAuth()->inlineParameters (paramContent);
  QNetworkReply * reply = Network()->post (req, content);  
  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  kind, A_Post); 
  ExpectReply (reply, chReply);
}

QByteArray 
NetworkIF::prepareOAuthString( const QString &requestUrl, 
                                     QOAuth::HttpMethod method,
                               const QOAuth::ParamMap &params )
{
qDebug () << "----------------------- param map";
qDebug () << params;
qDebug () << "........................ call ";
  QByteArray content = 
         webAuth.QOAuth()->createParametersString( requestUrl, method, 
                                acc_token, acc_secret,
                                QOAuth::HMAC_SHA1, 
                                params, 
                                QOAuth::ParseForHeaderArguments );
qDebug () << "vvvvvvvvvvvvvvvvvvvvvv OAUTH STRING " ;
qDebug () << content;
qDebug () << ">>>>>>>>>>>>>>>>>>>>>>>";
  return content;
}

void
NetworkIF::oauthForPost (QNetworkRequest & req,
                         const QString   & urlString,
                         const QOAuth::ParamMap & params)
{
  QByteArray parmdata = prepareOAuthString (urlString, QOAuth::GET, params);
  req.setRawHeader ("Authorization", parmdata);
  req.setHeader (QNetworkRequest::ContentTypeHeader,
                 "application/x-www-form-urlencoded");
}

void
NetworkIF::oauthForPicPost (QNetworkRequest & req,
                         const QString   & urlString,
                         const QOAuth::ParamMap & params,
                         const QByteArray & boundary,
                         const QString & authUrl)
{
  QByteArray parmData = prepareOAuthString (urlString, QOAuth::POST, params);
  parmData.insert (6,QByteArray("realm=\"http://api.twitter.com\""));
qDebug () << " OAuth string is " << parmData;
  req.setRawHeader ("X-Verify-Credentials-Authorization", parmData);
  req.setRawHeader ("X-Auth-Service-Provider",
                        authUrl.toUtf8().toPercentEncoding());
  QByteArray contentType ("multipart/form-data: boundary=");
  contentType.append (boundary);
  req.setHeader (QNetworkRequest::ContentTypeHeader,
                 contentType);
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
    if (twitterReplies.find(reply) != twitterReplies.end()) {
      twitterReplies.erase (reply);
    }
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
    if (bitlyReplies.find(reply) != bitlyReplies.end()) {
      bitlyReplies.erase (reply);
    }
    disconnect (reply, 0, 0, 0);
    disconnect (bitReply, 0, 0, 0);
    delete bitReply;
    reply->deleteLater ();
  }
}


void
NetworkIF::DebugShow (const QNetworkRequest & req)
{
  qDebug () << " request to " << req.url();
  QList<QByteArray>::iterator  hit;
  QList<QByteArray>  hdrs = req.rawHeaderList();
  for (hit = hdrs.begin(); hit != hdrs.end(); hit++) {
    qDebug () << " header " << *hit
              << ": "
              << req.rawHeader (*hit);
  }
}



} // namespace

