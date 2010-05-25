#ifndef CH_NAM_H
#define CH_NAM_H

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
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QTime>
#include <QByteArray>
#include <QIODevice>

namespace chronicon {

class ChNam : public QNetworkAccessManager {
Q_OBJECT

public:

  ChNam ( QObject * parent = 0 );
 
QNetworkReply * get (  QNetworkRequest & request );
QNetworkReply * post (   QNetworkRequest & request, 
                       const QByteArray & data );
QNetworkReply * post (  QNetworkRequest & request,
                            QIODevice * data);

  int Elapsed ();

public slots:
 
  void	needAuthentication ( QNetworkReply * reply, 
                              QAuthenticator * authenticator );

signals:

  void	authRequired ( QNetworkReply * reply, QAuthenticator * authenticator );

private:

  void FixUserAgent ( QNetworkRequest & req);

  void DumpHeader (const QNetworkRequest & request, QString msg = QString());
  void CheckSignature (QByteArray authHeader);

  QTime  lifetime;
  int    requestCount;

};

} // namespace

#endif
