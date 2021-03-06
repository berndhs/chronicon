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
#ifndef WEBAUTH_H
#define WEBAUTH_H

#include "QtOAuth-local.h"
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QUrl>

#define CHRON_DEBUG_OAUTH 0

using namespace QOAuth;

namespace chronicon {

/** \brief WebAuth is somewhat weird. This is deliberate, in following the
  * obscurity strategy proposed by the twitter people.
  */

class WebAuth : public QObject {
Q_OBJECT

public:

  WebAuth (QObject *parent);
  void Init (QByteArray name);
  bool InitDone ();

  QOAuth::Interface * QOAuth () { return authIF; }

  bool AskRequestToken ();
  bool AskAccessToken (QString pin, 
                         QByteArray & atoken, 
                         QByteArray & asecret,
                         QByteArray & screen_name,
                         QByteArray & user_id);

  QString WebUrlString ();

private slots:

  void FinishAuth ();


private:

  void SetKeys ();

  QString AuthService (QString path = QString());
  QString WebService  (QString path = QString());

  QOAuth::Interface *authIF;
  bool               initComplete;

  QString    service;
  QString    webservice;

  QByteArray callback_confirm_key;
  QByteArray req_token_key;
  QByteArray req_token_secret_key;

  QByteArray req_token_value;
  QByteArray req_token_secret_value;

  QByteArray acc_token;
  QByteArray acc_secret;

  QByteArray key1,key2,key3;
  QByteArray part2;


};


} // namespace

#endif
