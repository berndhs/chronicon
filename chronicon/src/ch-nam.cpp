
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

#include "ch-nam.h"
#include <QDebug>
#include <QString>
#include <QList>
#include <QByteArray>

namespace chronicon {

ChNam::ChNam (QObject *parent)
:QNetworkAccessManager (parent)
{
  connect (this, SIGNAL (authenticationRequired(QNetworkReply*, QAuthenticator*)),
           this, SLOT (needAuthentication (QNetworkReply*, QAuthenticator*)));
  lifetime.start();
}

int
ChNam::Elapsed ()
{
  return lifetime.elapsed ();
}

void
ChNam::needAuthentication ( QNetworkReply * reply, 
                              QAuthenticator * authenticator )
{
  qDebug () << "ChNam wants auth for " << reply << " from " << reply->url();
  emit authRequired (reply, authenticator);
}


QNetworkReply * 
ChNam::get (  QNetworkRequest & request )
{
  QNetworkReply * reply;
  FixUserAgent (request);
  DumpHeader (request,"GET");
  reply = QNetworkAccessManager::get (request);
  return reply;
}

QNetworkReply * 
ChNam::post ( QNetworkRequest & request, QIODevice * data )
{
  QNetworkReply * reply;
  FixUserAgent (request);
  DumpHeader (request,"POST iodev ");
  reply = QNetworkAccessManager::post (request, data);
  return reply;
}

QNetworkReply * 
ChNam::post (  QNetworkRequest & request, const QByteArray & data )
{
  QNetworkReply * reply;
  FixUserAgent (request);
  DumpHeader (request,"POST bytearray");
  reply = QNetworkAccessManager::post (request, data);
  return reply;
}

void
ChNam::DumpHeader (const QNetworkRequest & request, QString msg)
{
  qDebug () << " ____ start ChNam Info for request " << msg << " at lifetime " << Elapsed();
  
  qDebug () << " request to " << request.url();
  QList<QByteArray>  hdrs = request.rawHeaderList();
  QList<QByteArray>::iterator  hit;
  for (hit = hdrs.begin(); hit != hdrs.end(); hit++) {
    QByteArray header = request.rawHeader(*hit);
    qDebug () << " header " << *hit
              << ": "
              << header;
    if (*hit == "Authorization") {
      CheckSignature (header);
    }
  }
  qDebug () << " ____ end ChNam Info";
}

void
ChNam::FixUserAgent (QNetworkRequest & req)
{
  req.setRawHeader ("User-Agent","Chronicon; WebKit");
}

void 
ChNam::CheckSignature (QByteArray authHeader)
{
  QString header (authHeader);
  if (!header.contains ("oauth_signature=")) {
    qDebug () << ">>>>>>>>> NO SIGNATURE";
  }
}

} // namespace

