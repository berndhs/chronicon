

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
 ********************************************************************/

#include "webauth.h"
#include "deliberate.h"
#include "delib-debug.h"

using namespace deliberate;

namespace chronicon {


WebAuth::WebAuth (QObject *parent)
:QObject(parent),
 authIF(0)
{
  authIF = new QOAuth::Interface;
  SetKeys ();
}

void
WebAuth::SetKeys ()
{
  callback_confirm_key = "oauth_callback_confirmed";
  req_token_key = "oauth_token";
  req_token_secret_key = "oauth_req_token_secret";
}

void
WebAuth::Init ()
{
  // set the consumer key and secret
  authIF->setConsumerKey( "6Jy201BBEeQ7C0nAkUj7ig" );
  authIF->setConsumerSecret( "YZJ5i2Sd8zZbdWj95bmBPDMt3qTRft4LXArLQyMERMM" );
  // set a timeout for requests (in msecs)
  authIF->setRequestTimeout( 15000 );
  service = "https://api.twitter.com/oauth";
  service = Settings().value ("oauth/service",service).toString();
  Settings().setValue ("oauth/service",service);
  webservice = "https://mobile.twitter.com/oauth";
  webservice = Settings().value("oauth/webservice",webservice).toString();
  Settings().setValue ("oauth/webservice",webservice);
}

bool
WebAuth::AskRequestToken ()
{
// send a request for an unauthorized token
  QOAuth::ParamMap extra;
  extra.insert ("oauth_callback","oob");
  QOAuth::ParamMap reply =
      authIF->requestToken( AuthService ("request_token"),
                          QOAuth::GET, QOAuth::HMAC_SHA1, extra );
  QOAuth::ParamMap::iterator pit;
  for (pit = reply.begin(); pit != reply.end(); pit++) {
    qDebug () << " reply part " << QString(pit.key())
                 << " => " << QString (pit.value());
  }
  QByteArray req_token_confirm = reply.value (callback_confirm_key);
  if (authIF->error() != QOAuth::NoError) {
    return false;
  }
  if (req_token_confirm == "true") {
    req_token_value = reply.value (req_token_key);
    req_token_secret_value = reply.value (req_token_secret_key);
    return true;
  } else {
    return false;
  }
}

QString
WebAuth::WebUrlString ()
{
  QString urlString = AuthService("authorize?oauth_token=") 
                     + req_token_value;
  return urlString;
}

bool
WebAuth::AskAccessToken (QString pin, 
                         QByteArray & atoken, 
                         QByteArray & asecret,
                         QByteArray & screen_name,
                         QByteArray & user_id)
{
qDebug () << " ask access token ";
  QOAuth::ParamMap extra;
  extra.insert ("oauth_verifier",pin.toUtf8());
  QOAuth::ParamMap reply = 
      authIF->accessToken (AuthService("access_token"),
                          QOAuth::POST, req_token_value,
                          req_token_secret_value, QOAuth::HMAC_SHA1, extra);
qDebug () << " accessToken says " << authIF->error ();
  if (authIF->error() != QOAuth::NoError) {
qDebug () << " bad experience getting access token";
    return false;
  }
  
qDebug () << " reply size " << reply.size();
  QOAuth::ParamMap::iterator pit;
  for (pit = reply.begin(); pit != reply.end(); pit++) {
    qDebug () << " reply part " << QString(pit.key())
                 << " => " << QString (pit.value());
  }
  atoken = reply.value ("oauth_token");
  acc_token = atoken;
  asecret = reply.value ("oauth_secret");
  acc_secret = asecret;
  screen_name = reply.value ("screen_name");
  user_id = reply.value ("user_id");
qDebug () << " done with askAccess";
  return true;
}

QString
WebAuth::AuthService (QString path)
{
  static QString slash ("/");
  return service + slash + path;
}

QString
WebAuth::WebService (QString path)
{
  static QString slash ("/");
  return webservice + slash + path;
}


} // namespace

