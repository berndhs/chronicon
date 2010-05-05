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
#include <QDesktopServices>

namespace chronicon {

TimelineView::TimelineView (QObject *parent)
:QObject (parent),
 currentKind (R_Public),
 view(0)
{
  dtd = QString 
("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\""
 " \"http://www.w3.org/TR/html4/loose.dtd\">");
  QString headPattern = QString ("<head><title>Tweet List</title><meta http-equiv="
             "\"Content-Type\" content=\"text/html;charset=utf-8\" >%1</head>");
  QString style ("<style type=\"text/css\"> body { background-color:#e0e0e0;} "
                 "p { font-size:11pt; background-color:#e0e0ff; "
                    " padding:2px; margin:2x; "
                    " font-family:Times New Roman; } </style>");
  head = headPattern.arg (style);
}

void
TimelineView::SetView (QWebView *pv)
{
  view = pv;
  if (view) {
    connect (view, SIGNAL (linkClicked (const QUrl&)),
             this, SLOT (LinkClicked (const QUrl&)));
    view->page ()->setLinkDelegationPolicy (QWebPage::DelegateAllLinks);
  }
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
TimelineView::LinkClicked (const QUrl & url)
{
  if (url.isValid()) {
    QString scheme = url.scheme();
    if (scheme == "http" || scheme == "https" || scheme == "mailto") {
      QDesktopServices::openUrl (url);
    } else if (scheme == "chronicon") {
      CustomLink (url);
    } else {
      qDebug () << "bad URL? " << url;
    }
  }
}

void
TimelineView::CustomLink (const QUrl & url)
{
  qDebug () << " deal with special url " << url;
}

void
TimelineView::CatchStatusItem (StatusBlock block, TimelineKind kind)
{
  if (kind > R_None && kind < R_Top) {
    doc[kind].AddStatus (block);
    if (kind == currentKind) {
       AddCurrent (block);
    }
    if (kind == R_Update) {
       AddOwn (block);
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
  QString authUrl;
  QString imgUrl;
  bool    truncated ("false");
  QDateTime date;
  bool good = ParseBlock (block, text, author, authUrl, date, imgUrl, truncated);
  if (good) {
    paragraphs[id] = Paragraph (text,author,authUrl, date, imgUrl, truncated);
  } else {
    qDebug () << " bad block ";
  }
}

void
TimelineView::AddOwn (StatusBlock block)
{
  QString id = block.Id();
  QString text ("");
  QString author("myself");
  QString authUrl;
  QString imgUrl;
  bool    truncated ("false");
  QDateTime date;
  bool good = ParseBlock (block, text, author, authUrl, date, imgUrl, truncated);
  if (good) {
    text.prepend (" POST : ");
    paragraphs[id] = Paragraph (text,author,authUrl, date, imgUrl, truncated);
  } else {
    qDebug () << " bad block ";
  }
}

bool
TimelineView::ParseBlock (StatusBlock & block,
                         QString     & text,
                         QString     & author,
                         QString     & authUrl,
                         QDateTime   & date,
                         QString     & imgUrl,
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
      if (ParseUser (child, author, authUrl, imgUrl)) {
        expectMore --;
      }
    } else if (tag == "created_at") {
      date = QDateTime::fromString (child.text(),"ddd MMM dd HH:mm:ss +0000 yyyy");
      date.setTimeSpec (Qt::UTC);
      expectMore --;
    }
  }
  return true; //expectMore == 0;
}

bool
TimelineView::ParseUser (const QDomElement & elt,
                               QString     & author,
                               QString     & authUrl,
                               QString     & imgUrl)
{
  QDomElement child;
  int expectMore = 3;
  for (child = elt.firstChildElement(); !child.isNull();
       child = child.nextSiblingElement()) {
    QString tag = child.tagName();
    if (tag == "screen_name") {
      author = child.text();
      expectMore --;
    } else if (tag == "url") {
      authUrl = child.text();
      expectMore --;
    } else if (tag == "profile_image_url") {
      imgUrl = child.text ();
      expectMore --;
    }
  }
  return expectMore == 0;
}

void
TimelineView::FormatParagraph (QString & html, const Paragraph & para)
{
  html = "<p>";
  QString imgPattern ("<img border=\"0\"src=\"%1\" width=\"48\" height=\"48\" />");
  html.append (imgPattern.arg(para.imgUrl));
  html.append (MakeCustomLink (para.text, "text-decoration:none;color:black;",
                               "text"));
  QString urlPattern ("&nbsp;<a href=\"%1\">%2</a>");
  html.append (urlPattern.arg(para.authUrl).arg(para.author));
  QDateTime now = QDateTime::currentDateTime().toUTC();
  int ago = para.date.secsTo (now);
  html.append ("&nbsp;" + Ago(ago));
  html.append ("</p>");
}

QString 
TimelineView::MakeCustomLink (const QString & body, 
                              const QString & style, 
                              const QString & auth)
{
  QString pat ("<a style=\"%1\" href=\"chronicon://%2/%4\">%3</a>");
  QString link = pat.arg(style).arg(auth).arg(body).
                     arg(QString(QUrl::toPercentEncoding(body)));
  return link;
}

QString
TimelineView::Ago (int secs)
{
  if (secs < 0) {
    return QString (tr("%1 seconds in the future")).arg( - secs);
  }
  if (secs < 91) {
    return QString (tr("%1 seconds ago")).arg (secs);
  }
  int days = secs / (24*60*60);
  int hours = secs / (60*60);
  int mins = secs / 60;
  if (hours < 1) {
    return QString (tr("%1 minutes ago")).arg(secs/60);
  }
  if (days < 1) {
    mins = mins - (hours*60);
    return QString (tr("%1 hours %2 minutes ago")).arg(hours).arg(mins);
  }
  hours -= (days*24);
  return QString (tr("%1 days %2 hours ago")).arg(days).arg(hours);
}


void
TimelineView::Show ()
{
  if (view == 0) {
    return;
  }

  QString html (dtd);
  html.append ("\n<html>\n");
  html.append (head);
  html.append ("\n<body>\n");

  QString headlinePattern ("<h2>%1</h2>");
  QString date = QDateTime::currentDateTime().toString("ddd hh:mm:ss");
  html.append (headlinePattern.arg(date));
  QString parHtml;
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
