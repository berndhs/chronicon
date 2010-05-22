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

#ifndef CHRON_NETWORK_REPLY_H
#define CHRON_NETWORK_REPLY_H

#include <QObject>
#include <QNetworkReply>
#include <QUrl>
#include <QTimer>
#include "chronicon-types.h"

namespace chronicon {

/** \brief Encapsulate a QNetworkReply and provide additional info
  *
  * Used for connection a QNetworkReply signal (e.g. finished() )
  * to a particular object that knows what the reply object is,
  * and has additional information about what to do with the reply.
  */

class ChronNetworkReply : public QObject {

Q_OBJECT

public:


  ChronNetworkReply (QUrl &theUrl, 
                    QNetworkReply * qnr, 
                    TimelineKind req,
                    ApiRequestKind ark);
  ~ChronNetworkReply ();

  QNetworkReply * NetReply() { return reply; }
  TimelineKind     Kind () { return kind; }
  QUrl            Url () { return url; }
  ApiRequestKind  ARKind () { return arKind; }

  void Abort();
  void Close ();
  void SetTimeout (int msec);

  int error ();
  
public slots:

  void handleReturn ();
  void handleError (QNetworkReply::NetworkError err);
  void timedOut ();

signals:

  void Finished (ChronNetworkReply *);
  void AuthVerifyError (ChronNetworkReply * chReply, 
                        int err);
  void AuthVerifyGood  (ChronNetworkReply *chReply);
  void Timeout (ChronNetworkReply * chReply);
  void networkError (ApiRequestKind ark, int err);

private:


  QUrl           url;
  QNetworkReply  *reply;  
  TimelineKind    kind;
  ApiRequestKind  arKind;
  QTimer         *expireTimer;
  bool            hasExpired;


};

} // namespace


#endif
