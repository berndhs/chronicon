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

#include "network-if.h"
#include <QDomDocument>

namespace chronicon {

NetworkIF::NetworkIF (QObject *parent)
:network(parent)
{
}

void
NetworkIF::PullPublicTimeline ()
{
  QNetworkRequest request;
  QUrl url  ("http://api.twitter.com/1/statuses/public_timeline.xml");
  request.setUrl(url);
  QNetworkReply *reply = network.get (request);
  ChronNetworkReply *chReply = new ChronNetworkReply (url,
                                                  reply, 
                                                  ChronNetworkReply::R_Public);
  replies[reply] = chReply;
  connect (reply, SIGNAL (finished()), chReply, SLOT (handleReturn()));
  connect (chReply, SIGNAL (Finished(ChronNetworkReply*)),
           this, SLOT (handleReply (ChronNetworkReply*)));
  connect (reply, SIGNAL(error(QNetworkReply::NetworkError)),
         this, SLOT(networkError(QNetworkReply::NetworkError)));
}

void
NetworkIF::handleReply (ChronNetworkReply * reply)
{
  if (reply) {
    QNetworkReply * netReply = reply->NetReply();
    if (netReply && reply->Kind() == ChronNetworkReply::R_Public) {
      QDomDocument doc;
      bool ok =doc.setContent (netReply);   
      qDebug () << " doc text " << doc.toString();
      qDebug () << " reply contains doc "   << doc.doctype ().name();
      qDebug () << " reply doc ok " << ok;
    }
    ReplyMapType::iterator index;
    index = replies.find (netReply);
    if (index != replies.end()) {
      replies.erase(index);
    }
    delete reply;
    if (netReply) {
      netReply->deleteLater();
    }
  }
}

void
NetworkIF::networkError (QNetworkReply::NetworkError err)
{
  qDebug () << "network error " << err;
}

} // namespace

