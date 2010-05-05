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
#include <QDateTime>

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
  if (!block.HasUserValue("screen_name")) {
     block.SetUserValue ("screen_name","anonymous");
  }
  paragraphs[id] = block;
}

void
TimelineView::AddOwn (StatusBlock block)
{
  QString id = block.Id();
  if (!block.HasUserValue("screen_name")) {
     block.SetUserValue ("screen_name","me");
  }
  QString text = block.Value("text");
  text.prepend ("POST: ");
  block.SetValue ("text", text);
  paragraphs[id] = block;
}


void
TimelineView::FormatParagraph (QString & html, const StatusBlock & para)
{
  QString bckCol = "f0f0f0"; // para.UserValue ("profile_background_color");
  QString txtCol = "000000"; // para.UserValue ("profile_text_color");
  QString paraHeadPat ( "<div style=\"width:100%;"
                        "float:left;"
                        "font-size:90%;background-color:#%1;"
                         "color:%2\">");
  html = paraHeadPat.arg (bckCol).arg(txtCol);
  QString imgPattern ("<div style=\"float:left;margin:3px;\">"
                    "<img border=\"0\"src=\"%1\" width=\"48\" height=\"48\" "
                      " style=\"vertical-align:text-top;\" />"
                     "</div>");
  html.append (imgPattern.arg(para.UserValue("profile_image_url")));
  html.append (FormatTextBlock (para.Value("text")));
  QString urlPattern ("&nbsp;<a style=\"font-weight:bold;font-size:90%;\" "
                       "href=\"http://twitter.com/%1\">%1</a>");
  html.append (urlPattern.arg(para.UserValue("screen_name")));
  QDateTime now = QDateTime::currentDateTime().toUTC();
  QDateTime date = QDateTime::fromString 
                              (para.Value("created_at"),
                               "ddd MMM dd HH:mm:ss +0000 yyyy");
  date.setTimeSpec (Qt::UTC);
  int ago = date.secsTo (now);
  html.append (QString("&nbsp;<span style=\"font-size:90%;font-style:italic;\">%1</span>")
                      .arg( Ago(ago)));
  html.append ("</div><br>");
}

QString 
TimelineView::FormatTextBlock (const QString & text)
{
  void (*anchorFunc) (QString&, QString);
  anchorFunc = &chronicon::HttpAnchor;
  QString subHttp = Anchorize (text + QString(" "), 
                             QRegExp ("(https?://)(\\S*)"), 
                             anchorFunc);
  anchorFunc = &chronicon::TwitAtAnchor;
  QString subAt = Anchorize (subHttp, 
                             QRegExp ("@(\\S*)"),
                             anchorFunc);
  anchorFunc = &chronicon::TwitHashAnchor;
  QString subHash = Anchorize (subAt, 
                             QRegExp ("#(\\S*)"),
                             anchorFunc);
  QString span ("<span style=\"font-size:90%;\">%1</span>");
  return span.arg(subHash);
}

QString
TimelineView::Anchorize (const QString &text, QRegExp regular, 
                         void (*anchorFunc)(QString&, QString))
{
  int where;
  int offset(0);
  int lenSub;
  QString newtext;
  QString chunk;
  while ((where  = regular.indexIn (text,offset)) >= 0) {
    lenSub = regular.matchedLength();
    chunk = text.mid (offset, where - offset);
    newtext.append (chunk);
    QString anchor;
    (*anchorFunc) (anchor, regular.cap(0));
    newtext.append (anchor);
    offset = where + lenSub;
  }
  chunk = text.mid (offset,-1);
  newtext.append (chunk);
  return newtext;
}

void
HttpAnchor (QString & anchor, QString ref)
{
  anchor =  QString("<a href=\"%1\">%1</a>").arg(ref);
}

void
TwitAtAnchor (QString & anchor, QString ref)
{
  if (ref.length() == 1) {
    anchor = ref;
    return;
  }
  anchor = QString ("@<a href=\"http://twitter.com/%1\">%1</a>")
                 .arg(ref.mid(1));
}

void
TwitHashAnchor (QString & anchor, QString ref)
{
  if (ref.length() == 1) {
    anchor = ref;
  }
  anchor = QString ("#<a href=\"http://twitter.com/#search?q=%1\">%1</a>")
                   .arg(ref);
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

  QString headlinePattern ("<h3>%1</h3>");
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
