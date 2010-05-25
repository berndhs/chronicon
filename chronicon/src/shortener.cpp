
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

#include <QRegExp>
#include <QString>
#include <QStringList>
#include "shortener.h"
#include "network-if.h"

namespace chronicon {

Shortener::Shortener (QObject *parent)
:QObject (parent),
 network (0)
{
}

void
Shortener::SetNetwork (NetworkIF * net)
{
  network = net;
 
  connect (network, SIGNAL (ShortenReply (QUuid, QString, QString, bool)),
           this, SLOT (CatchShortening (QUuid, QString, QString, bool)));
}



void
Shortener::ShortenHttp (QString status, bool & wait)
{
  if (network == 0) {
    emit DoneShortening (status);
    return;
  }
  QRegExp regular ("(https?://)(\\S*)");
  status.append (" ");
  QStringList  linkList;
  QStringList  wholeList;
  int where (0), offset(0), lenSub(0);
  QString link, beforeLink;
  while ((where = regular.indexIn (status,offset)) > 0) {
    lenSub = regular.matchedLength();
    beforeLink = status.mid (offset, where - offset);
    link = regular.cap(0);
    if ((!link.contains ("bit.ly"))
        && (!link.contains ("twitpic.com"))
        && (link.length() > QString("http://bit.ly/12345678").length())) {
      linkList << link;
    }
    wholeList << beforeLink;
    wholeList << link;
    offset = where + lenSub;
  }
  wholeList << status.mid (offset, -1);
  shortenTag = QUuid::createUuid();
  if (linkList.isEmpty ()) {
    wait = false;
  } else {
    messageParts[shortenTag] = wholeList;
    linkParts   [shortenTag] = linkList;
    network->ShortenHttp (shortenTag,linkList);
    wait = true;
  }
}

void
Shortener::CatchShortening (QUuid tag, QString shortUrl, QString longUrl, bool good)
{
  /// replace the longUrl with shortUrl in the messageParts[tag]
  //  remove the longUrl from the linkParts[tag]
  //  if the linkParts[tag] is empty, we have replaced all the links
  //  so send append all the messageParts[tag] and finish the message
  if (messageParts.find(tag) == messageParts.end()) {
    return; // extra, perhaps duplicates in original, or not for me
  }
  if (linkParts.find(tag) == linkParts.end()) {
    return;
  }
  QStringList::iterator chase;
  for (chase = messageParts[tag].begin(); 
       chase != messageParts[tag].end(); 
       chase++) {
    if (*chase == longUrl) {
       *chase = shortUrl;
    }
  }
  linkParts[tag].removeAll (longUrl);
  if (linkParts[tag].isEmpty()) {
    QString message = messageParts[tag].join (QString());
    emit DoneShortening (message);
    messageParts.erase (tag);
  }
}


} // namespace

