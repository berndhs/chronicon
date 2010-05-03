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

#include "delib-debug.h"
#include "timeline-view.h"
#include <QWebPage>
#include <QWebFrame>
#include <QWebElementCollection>

namespace chronicon {

TimelineView::TimelineView (QObject *parent)
:QObject (parent),
 currentKind (R_Public),
 view(0)
{
  dtd = QString 
("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\""
 " \"http://www.w3.org/TR/html4/loose.dtd\">");
  head = QString ("<head><title>Tweet List</title><meta http-equiv="
             "\"Content-Type\" content=\"text/html;charset=utf-8\" ></head>");
}

TimelineDoc &
TimelineView::Doc (TimelineKind k)
{
  if (k < R_None || k >= R_Top) {
    k = R_None;
  }
  return doc[k];
}

void
TimelineView::Display (TimelineKind k)
{
  if (k < R_None || k >= R_Top) {
    k = R_None;
  }
  currentKind = k;
}

void
TimelineView::CatchStatusItem (StatusBlock block, TimelineKind kind)
{
  qDebug () << " caught new block " << block.Id() << " kind " << kind;
  if (kind > R_None && kind < R_Top) {
    doc[kind].AddStatus (block);
    if (kind == currentKind) {
       AddCurrent (block);
    }
  } else {
    qDebug () << __FILE__ << __LINE__ << " bad timeline kind " << kind;
  }
}

void
TimelineView::AddCurrent (StatusBlock block)
{
  QString id = block.Id();
  QString text ("");
  QString author("anonymous");
  QString auth_url;
  bool    truncated ("false");
  QDateTime date;
  bool good = ParseBlock (block, text, author, auth_url, date, truncated);
  if (good) {
    paragraphs[id] = Paragraph (text,author,auth_url, date,truncated);
  }
}

bool
TimelineView::ParseBlock (StatusBlock & block,
                         QString     & text,
                         QString     & author,
                         QString     & auth_url,
                         QDateTime   & date,
                         bool        & truncated)
{
  QDomElement top = block.DomData ();
  QDomElement child;
  int expectMore = 4; 
  for (child = top.firstChildElement(); !child.isNull();
       child = child.nextSiblingElement ()) {
    QString tag = child.tagName();
    if (tag == "text") {
      text = child.text ();
      expectMore --;
    } else if (tag == "truncated") {
      truncated = (child.text() == "true");
      expectMore --;
    } else if (tag == "user") {
      if (ParseUser (child, author, auth_url)) {
        expectMore --;
      }
    } else if (tag == "created_at") {
      date = QDateTime::fromString (child.text(),"ddd MMM dd HH:mm:ss +0000 yyyy");
      expectMore --;
    }
  }
  return expectMore == 0;
}

bool
TimelineView::ParseUser (const QDomElement & elt,
                               QString     & author,
                               QString     & auth_url)
{
  QDomElement child;
  int expectMore = 2;
  for (child = elt.firstChildElement(); !child.isNull();
       child = child.nextSiblingElement()) {
    QString tag = child.tagName();
    if (tag == "screen_name") {
      author = child.text();
      expectMore --;
    } else if (tag == "url") {
      auth_url = child.text();
      expectMore --;
    }
  }
  return expectMore == 0;
}

void
TimelineView::FormatParagraph (QString & html, const Paragraph & para)
{
  html = "<br><p>";
  html.append (para.text);
  html.append ("<br>&nbsp;");
  QString urlPattern ("<a href=\"%1\">%2</a>");
  html.append (urlPattern.arg(para.auth_url).arg(para.author));
  QDateTime now = QDateTime::currentDateTime();
  int ago = para.date.secsTo (now);
  QString durationPattern (" %1 seconds ago");
  html.append (durationPattern.arg(ago));
  html.append ("</p><br>");
}


void
TimelineView::Show ()
{
  qDebug () << " timeline show";
  if (view == 0) {
    return;
  }

  QString html (dtd);
  html.append ("\n<html>\n");
  html.append (head);
  html.append ("\n<body>\n");

  QString parHtml;
  qDebug () << " have " << paragraphs.size() << " paragraphs";
  PagePartMap::reverse_iterator para;
  for (para = paragraphs.rbegin(); para != paragraphs.rend(); para++) {
     FormatParagraph (parHtml, para->second);
     html.append (parHtml);
  }
  html.append ("\n</body>\n</html>");
  view->setHtml (html);
  view->update ();
}


} // namespace
