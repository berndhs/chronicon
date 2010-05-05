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
#if USE_OAUTH
  #include <QtCrypto>
#endif
#include <QAuthenticator>
#include "delib-debug.h"
#include <map>
#include "chronicon-types.h"
#include "chron-network-reply.h"
#include "status-block.h"

namespace chronicon {


class NetworkIF : public QObject {

Q_OBJECT

public:

  NetworkIF (QObject *parent);

  void PullTimeline ();
  void PushUserStatus (QString status);

  void SetTimeline (TimelineKind k);
  void SetBasicAuth (QString us, QString pa=QString());

public slots:
  
  void login (int * reply = 0);

  void handleReply (ChronNetworkReply *reply);
  void networkError (QNetworkReply::NetworkError err);
  void authProvide (QNetworkReply * reply,
                     QAuthenticator * authenticator);

signals:

  void NewStatusItem (StatusBlock item, TimelineKind kind);
  void ReplyComplete ();
  void RePoll (TimelineKind kind);

private:

  void ParseDom (QDomDocument &doc, TimelineKind kind);
  void ParseStatus (QDomElement &elt, TimelineKind kind);
  void SwitchTimeline ();
  void ExpectReply (QNetworkReply *reply, 
                    ChronNetworkReply *chReply);
  void CleanupReply (QNetworkReply * reply, ChronNetworkReply *chReply);
  

  QNetworkAccessManager   network;

  TimelineKind            serviceKind;
  QString                 timelineName;
  QString                 user;
  QString                 pass;

  typedef std::map <QNetworkReply *, ChronNetworkReply*>  ReplyMapType;

  ReplyMapType  replies;

};

} // namespace

#endif