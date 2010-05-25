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

#include "chron-network-reply.h"
#include "delib-debug.h"
#include "ch-nam.h"


namespace chronicon {

ChronNetworkReply::ChronNetworkReply (ChNam* nam,
                                      QUrl & theUrl, 
                                      QNetworkReply * qnr, 
                                      TimelineKind req,
                                       ApiRequestKind ark)
:netMgr(nam),
 url(theUrl),
 reply(qnr),
 kind (req),
 arKind (ark),
 expireTimer (0),
 hasExpired (false)
{}

ChronNetworkReply::~ChronNetworkReply ()
{
  if (expireTimer) {
    expireTimer->stop();
    expireTimer->disconnect();
  }
}

void
ChronNetworkReply::handleReturn ()
{
qDebug () << " ____ start ChronNetworkReply reply at " << netMgr->Elapsed() << "for  " << kind << " from " << url;
qDebug () << " for timeline " << kind << " ARK " << arKind;
  QList<QByteArray>::iterator  hit;
  QList<QByteArray>  hdrs = reply->rawHeaderList();
  for (hit = hdrs.begin(); hit != hdrs.end(); hit++) {
    qDebug () << " header " << *hit
              << ": "
              << reply->rawHeader (*hit);
  }
  if (expireTimer) {
    expireTimer->stop();
  }
  if (arKind == A_DumpEcho) {  // dump content to qDebug and ignore
     QByteArray content = reply->readAll();
     qDebug () << " A_DumpEcho reply data to be ignored:";
     qDebug () << content;
  } else if (arKind == A_AuthVerify) {
    if (error() == 0) {
      emit AuthVerifyGood (this);
    } else {
      emit AuthVerifyError (this, error());
    }
  } else {
    emit Finished (this);
  }
  qDebug () << " ____ end ChronNetworkReply reply";
}

int
ChronNetworkReply::error ()
{
  if (reply) {
    return reply->error();
  } else if (hasExpired) {
    return CHERR_Timeout;
  } else {
    return CHERR_Internal;
  }
}

void
ChronNetworkReply::handleError (QNetworkReply::NetworkError err)
{
  if (expireTimer) {
    expireTimer->stop();
  }
  qDebug () << __FILE__ << __LINE__ << " Chron network error " << err << " for reply " << reply;
  qDebug () << " for timeline " << kind << " ARK " << arKind;
  if ((arKind == A_AuthVerify)) {
  qDebug () << " emit AVE " << err;
    emit AuthVerifyError (this, err);
  } else {
  qDebug () << " emit NetErr " << err;
QByteArray data;
    data = reply->readAll();
qDebug () << " reply data " << QString (data);
    emit networkError (arKind, err);
  }
}

void
ChronNetworkReply::SetTimeout (int msec)
{
  if (expireTimer == 0) {
    expireTimer = new QTimer (this);
  }
  if (expireTimer) {  
    connect (expireTimer, SIGNAL (timeout()), 
             this, SLOT (timedOut()));
    expireTimer->setSingleShot (true);
    expireTimer->start (msec);
  }
}

void
ChronNetworkReply::timedOut ()
{
  if (expireTimer) {
    expireTimer->stop();
    hasExpired = true;
    emit Timeout (this);
    if (arKind == A_AuthVerify) {
      emit AuthVerifyError (this, CHERR_Timeout);
    }
  }
}

void
ChronNetworkReply::Abort ()
{ 
  if (reply) {
    reply->abort();
  }
}

void
ChronNetworkReply::Close ()
{
  if (reply) {
    reply->close ();
  }
}

} // namespace
