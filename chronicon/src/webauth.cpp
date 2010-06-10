

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
#include <QTimer>

using namespace deliberate;

namespace chronicon {


WebAuth::WebAuth (QObject *parent)
:QObject(parent),
 authIF(0),
 initComplete (false)
{
  authIF = new QOAuth::Interface;
  key1 = "DS8CFikiJxgIKQdAGT48OQ45LQwqLg==";
  key2 = "CQMoIgVaBTwlMx8qGBYIOQoqOl8nVwgCDQYXLB8+DD4TUgtfMzEYNg==";
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
WebAuth::Init (QByteArray name)
{
  // set the consumer key and secret
  QByteArray k (key1);
  deliberate::Rot2 (k,name);
  authIF->setConsumerKey(k);
  k = "https://api.twitter.com/oauth";
  // set a timeout for requests (in msecs)
  authIF->setRequestTimeout( 15000 );
  service = "https://api.twitter.com/oauth";
  service = Settings().value ("oauth/service",service).toString();
  Settings().setValue ("oauth/service",service);
  webservice = "https://mobile.twitter.com/oauth";
  webservice = Settings().value("oauth/webservice",webservice).toString();
  Settings().setValue ("oauth/webservice",webservice);
  key3 = name;
  QTimer::singleShot (50,this, SLOT (FinishAuth()));
}

bool
WebAuth::InitDone ()
{ 
  return initComplete;
}

void
WebAuth::FinishAuth ()
{
  QByteArray k = key2;
  deliberate::Rot2 (k, key3);
  authIF->setConsumerSecret (k);
  for (int i=0;i<k.size();i++) { k[i] = 0; }
  initComplete = true;
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
  #if CHRON_DEBUG_OAUTH
  for (pit = reply.begin(); pit != reply.end(); pit++) {
    qDebug () << " reply part " << QString(pit.key())
                 << " => " << QString (pit.value());
  }
  #endif
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
  QOAuth::ParamMap extra;
  extra.insert ("oauth_verifier",pin.toUtf8());
  QOAuth::ParamMap reply = 
      authIF->accessToken (AuthService("access_token"),
                          QOAuth::POST, req_token_value,
                          req_token_secret_value, QOAuth::HMAC_SHA1, extra);
  if (authIF->error() != QOAuth::NoError) {
    #if CHRON_DEBUG_OAUTH
    qDebug () << " bad experience getting access token";
    #endif
    return false;
  }
  #if CHRON_DEBUG_OAUTH
  QOAuth::ParamMap::iterator pit;
  for (pit = reply.begin(); pit != reply.end(); pit++) {
    qDebug () << " reply part " << QString(pit.key())
                 << " => " << QString (pit.value());
  }
  #endif
  atoken = reply.value ("oauth_token");
  acc_token = atoken;
  asecret = reply.value ("oauth_token_secret");
  acc_secret = asecret;
  screen_name = reply.value ("screen_name");
  user_id = reply.value ("user_id");
  Settings().setValue ("oauth/token",acc_token);
  Settings().setValue ("oauth/secret",acc_secret);
  Settings().setValue ("oauth/screen_name",screen_name);
  Settings().setValue ("oauth/user_id",user_id);
  Settings().sync();
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

