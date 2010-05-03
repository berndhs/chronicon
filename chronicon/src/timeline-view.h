#ifndef TIMELINE_VIEW_H
#define TIMELINE_VIEW_H

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

#include <QWebView>
#include <QWebElement>
#include <QObject>
#include "chronicon-types.h"
#include "timeline-doc.h"
#include "status-block.h"
#include <map>

namespace chronicon {

class Paragraph {
public:
  Paragraph (){}
  Paragraph (QString tx, QString au, QString url, QDateTime dt, bool tr)
           : text(tx),author(au),auth_url(url),date(dt),truncated(tr)
             {}
  Paragraph (const Paragraph &other)
           : text(other.text),
             author(other.author),
             auth_url(other.auth_url),
             date(other.date),
             truncated(other.truncated)
             {}
  QString text;
  QString author;
  QString auth_url;
  QDateTime date;
  bool    truncated;
};

class TimelineView : public QObject {

Q_OBJECT

public:

  TimelineView (QObject *parent=0);

  void SetView (QWebView * pv) { view = pv; }

  TimelineDoc & CurrentDoc () { return doc[currentKind]; }
  TimelineDoc & Doc (TimelineKind k);

  int DisplayKind () { return currentKind; }
  void         Display (TimelineKind k);
  void         Show ();

public slots:

  void CatchStatusItem (StatusBlock block, TimelineKind kind);

private:

  void AddCurrent (StatusBlock block);
  bool ParseBlock (      StatusBlock & block,
                         QString     & text,
                         QString     & author,
                         QString     & auth_url,
                         QDateTime   & date,
                         bool        & truncated);
  bool ParseUser (const QDomElement & elt,
                        QString     & author,
                        QString     & auth_url);
  void FormatParagraph (QString & html, const Paragraph & para);

  int            currentKind;
  TimelineDoc    doc[R_Top];
  QWebView      *view;

  typedef  std::map <QString, Paragraph>   PagePartMap;

  PagePartMap   paragraphs;
  QString       dtd;
  QString       head;
  

};

} // namespace

#endif
