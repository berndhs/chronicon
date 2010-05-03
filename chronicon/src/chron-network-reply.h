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
#include "chronicon-types.h"

namespace chronicon {


class ChronNetworkReply : public QObject {

Q_OBJECT

public:


  ChronNetworkReply (QUrl &theUrl, QNetworkReply * qnr, TimelineKind req);

  QNetworkReply * NetReply() { return reply; }
  TimelineKind     Kind () { return kind; }
  QUrl            Url () { return url; }
  
public slots:

  void handleReturn ();

signals:

  void Finished (ChronNetworkReply *);

private:


  QUrl           url;
  QNetworkReply  *reply;  
  TimelineKind    kind;


};

} // namespace


#endif
