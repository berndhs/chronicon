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
#include "delib-debug.h"
#include <map>
#include <chron-network-reply.h>

namespace chronicon {


class NetworkIF : public QObject {

Q_OBJECT

public:

  NetworkIF (QObject *parent);

  void PullPublicTimeline ();

public slots:

  void handleReply (ChronNetworkReply *reply);
  void networkError (QNetworkReply::NetworkError err);

private:

  QNetworkAccessManager   network;

  typedef std::map <QNetworkReply *, ChronNetworkReply*>  ReplyMapType;

  ReplyMapType  replies;

};

} // namespace

#endif
