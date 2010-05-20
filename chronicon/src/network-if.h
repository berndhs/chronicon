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

#ifndef NETWORK_IF_H
#define NETWORK_IF_H

#include <QNetworkAccessManager>
#include <QAuthenticator>
#include <QTimer>
#include <QUuid>
#include "delib-debug.h"
#include <map>
#include "chronicon-types.h"
#include "chron-network-reply.h"
#include "bitly-network-reply.h"
#include "status-block.h"
#include "login-dialog.h"
#include "weblogin.h"
#include "webauth.h"


namespace chronicon {

/** \brief The connection to the twitter/statusnet server and shortening server.
  *
  * The NetworkIF sends requests to the twitter/stausnet server, as well as
  * the shortening service, and handles the replies.
  * 
  * NetworkIF knows the different kinds of requests, and 2 different
  * authorization modes. It decides when authorization is needed, and when
  * a user has to log in.
  * When replies come back with data for statuses (tweets) or for shortening
  * results (http links), NetworkIF handles the basic decoding of XML
  * data and makes the results available by emitting signals.
  */

class NetworkIF : public QObject {

Q_OBJECT

public:

  NetworkIF (QWidget *parent);

  void ResetNetwork ();

  void Init ();

  void PullTimeline ();
  void PullTimeline (QString otherUser);
  void ChangeFollow (QString otherUser, int change);
  void PushUserStatus (QString status, QString refId);
  void ReTweet (QString id);
  void PushDelete (QString id);

  void SetTimeline (TimelineKind k);
  void SetBasicAuth (QString us, QString pa=QString());
  void TestBasicAuth (QString us, QString pa);
  void PushTwitterLogout ();
  void SetUserAgent (QString ua) { userAgent = ua; }
  void SetServiceRoot (QString root);

  void AutoLogin (QByteArray u, QByteArray key1, QByteArray key2, bool oauth);
  bool HaveUser () { return haveUser; }
  
  QString Service (QString path=QString());
  QUrl    ServiceUrl (QString path=QString());
  QString OAuthService (QString path=QString());

  void ShortenHttp (QUuid tag, QStringList httpList);

public slots:
  
  void login (int * reply = 0);

  void handleReply (ChronNetworkReply *reply);
  void handleReply (BitlyNetworkReply *reply);
  void networkError (QNetworkReply::NetworkError err);
  void networkErrorInt (ApiRequestKind ark, int err);
  void twitterAuthBad (ChronNetworkReply * reply, int err);
  void twitterAuthGood (ChronNetworkReply * reply);

  void authProvide (QNetworkReply * reply,
                     QAuthenticator * authenticator);
  void twitterAuthProvide (QNetworkReply * reply,
                     QAuthenticator * authenticator);
  void bitlyAuthProvide (QNetworkReply * reply,
                     QAuthenticator * authenticator);
  void DirectMessage (QString toName, QString msg);
  void PullUserBlock ();

signals:

  void NewStatusItem (StatusBlock item, TimelineKind kind);
  void ReplyComplete (TimelineKind kind);
  void RePoll (TimelineKind kind);
  void ShortenReply (QUuid tag, QString shortUrl, QString longUrl, bool good);
  void ClearList ();
  void TwitterAuthGood ();
  void TwitterAuthBad ();
  void StopPoll (bool stopit);
  

private:

  QWidget   *parentWidget;
  QNetworkAccessManager *Network ();

  void ConnectNetwork ();
  void AskBitly (QUuid tag, QString http);
  void plainLogin (int *reply);
  void webLogin   (int *reply);

  void PostOA (QString  & urlString, 
                   QOAuth::ParamMap & paramContent,
                   TimelineKind      kind);
  void PostBasic (QUrl &url, 
                      QNetworkRequest &req, 
                      QByteArray & data,
                      TimelineKind kind);

  void PushUserStatusOA (QString status, QString refId);
  void PushUserStatusBasic (QString status, QString refId);
  void DirectMessageOA (QString toName, QString msg);
  void DirectMessageBasic (QString toName, QString msg);
  void ChangeFollowOA (QString otherUser, int change);
  void ChangeFollowBasic (QString otherUser, int change);
  void PullTimelineOA (QString otherUser = QString());
  void PullTimelineBasic (QString otherUser = QString());
  void PullUserBlockOA ();
  void PullUserBlockBasic ();
  void PushDeleteOA (QString id);
  void PushDeleteBasic (QString id);
  void ReTweetOA (QString id);
  void ReTweetBasic (QString id);
  void ParseTwitterDoc (QDomDocument &doc, TimelineKind kind);
  void ParseUpdate (QDomDocument &doc, TimelineKind kind);
  void ParseUserBlock (QDomDocument &doc, TimelineKind kind);
  void ParseStatus (QDomElement &elt, TimelineKind kind);
  void ParseBitlyDoc (QDomDocument &doc, 
                      QString & shortUrl,
                      QString & longUrl,
                      bool    & good);
  void ParseBitlyData (QDomElement &data, 
                      QString & shortUrl,
                      QString & longUrl,
                      bool    & good);
  void ExpectReply (QNetworkReply *reply, 
                    ChronNetworkReply *chReply);
  void CleanupReply (QNetworkReply * reply, ChronNetworkReply *chReply);
  void ExpectReply (QNetworkReply *reply, 
                    BitlyNetworkReply *bitReply);
  void CleanupReply (QNetworkReply * reply, BitlyNetworkReply *bitReply);
  QByteArray prepareOAuthString( const QString &requestUrl, 
                                     QOAuth::HttpMethod method,
                               const QOAuth::ParamMap &params );
  void oauthForPost (QNetworkRequest & reg,
                         const QString   & urlString,
                         const QOAuth::ParamMap & params);
  void DebugShow (const QNetworkRequest &req );

  
  QNetworkAccessManager   *nam;

  QString                 serverRoot;

  TimelineKind            serviceKind;
  LoginDialog             plainLoginDialog;
  WebAuth                 webAuth;
  WebLogin                webLoginDialog;
  QString                 user;
  QString                 pass;
  QString                 userAgent;
  int                     authRetries;
  bool                    insideLogin;
  bool                    oauthMode;
  bool                    haveUser;

  int                     numItems;
  QString                 myName ;
  QByteArray              acc_token;
  QByteArray              acc_secret;

  typedef std::map <QNetworkReply *, ChronNetworkReply*>  ReplyMapType;
  typedef std::map <QNetworkReply *, BitlyNetworkReply*>  BitlyMapType;

  ReplyMapType  twitterReplies;
  BitlyMapType  bitlyReplies;

  QList <QNetworkAccessManager*> oldNAMs;

};

} // namespace

#endif
