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

#include <libnotify/notify.h>
#include "chronicon-global.h"
#include "delib-debug.h"
#include "deliberate.h"
#include "timeline-view.h"
#include "network-if.h"
#include "ui_itemdetail.h"
#include <QWebPage>
#include <QWebFrame>
#include <QDesktopServices>
#include <QDateTime>
#include <QCursor>
#include <QFontMetrics>
#include <QSize>

using namespace deliberate;

namespace chronicon {

TimelineView::TimelineView (QWidget *parent)
:QObject (parent),
 parentWidget (parent),
 currentKind (R_Public),
 doNotify (false),
 notifyDelay (10*1000),
 view(0),
 mypage(0),
 network (0),
 detailTip (0),
 maxParagraphs (100)
{
  HtmlStyles ();
}

void
TimelineView::HtmlStyles ()
{
  dtd = QString 
("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\""
 " \"http://www.w3.org/TR/html4/loose.dtd\">");
  statusBackgroundColor = "f0f0f0";
  headPattern = QString ("<head><title>Tweet List</title><meta http-equiv="
             "\"Content-Type\" content=\"text/html;charset=utf-8\" >%1</head>");
  headStyle = QString ("<style type=\"text/css\"> body { background-color:#e0e0e0;} "
                 "p { font-size:10pt; background-color:#%1; "
                    " padding:2px; margin:2x; "
                    " font-family:Times New Roman; } </style>");
  head = headPattern.arg (headStyle.arg(statusBackgroundColor));
  textColor = "000000";
  fontSize = "90%";
  nickStyle = "font-weight:bold;text-decoration:none;";
  titleStyle = "font-size:smaller; color:0f2f0f;";
  titleDateForm = tr("ddd hh:mm:hh");
  imgPattern = QString ("<div style=\"float:left;margin:3px;\">"
                    "<a href=\"chronicon://status/item#%2\" style=\"%3\">"
                    "<img border=\"0\"src=\"%1\" width=\"48\" height=\"48\" "
                      " style=\"vertical-align:text-top;\" />"
                     "</a></div>");
  iconLinkStyle = "text-decoration:none;";
}

void
TimelineView::LoadSettings ()
{
  dtd = Settings().value ("view/DTD", dtd).toString();
  statusBackgroundColor = Settings()
                  .value ("view/status_background_color",statusBackgroundColor)
                   .toString();
  headPattern = Settings().value ("view/headpattern",headPattern).toString();
  headStyle = Settings().value ("view/headstyle", headStyle).toString();
  head = headPattern.arg (headStyle.arg(statusBackgroundColor));
  textColor = Settings().value("view/textcolor", textColor).toString();
  fontSize = Settings().value("view/fontsize",fontSize).toString();
  nickStyle = Settings().value("view/nickstyle",nickStyle).toString();
  paraHeadPat = QString ( "<div style=\"width:100%;"
                        "float:left;"
                        "font-size:%1;background-color:#%2;"
                         "color:%3\">");
  paraHeadPat = Settings().value ("view/status_head",paraHeadPat).toString();
  titleStyle = Settings().value ("view/titlestyle",titleStyle).toString();
  titleDateForm = Settings().value ("view/titledateform",titleDateForm)
                            .toString();
  imgPattern = Settings().value ("view/imgpattern",imgPattern).toString();
  iconLinkStyle = Settings().value ("view/iconlinkstyle", iconLinkStyle).toString();
  maxParagraphs = Settings().value ("view/maxitems",maxParagraphs).toInt();
  notifyDelay = Settings().value ("timers/notifydelay",notifyDelay).toInt ();
  Settings().setValue ("view/DTD",dtd);
  Settings().setValue ("view/headpattern",headPattern);
  Settings().setValue ("view/status_background_color",statusBackgroundColor);
  Settings().setValue ("view/textcolor",textColor);
  Settings().setValue ("view/fontsize",fontSize);
  Settings().setValue ("view/nickstyle",nickStyle);
  Settings().setValue ("view/headstyle",headStyle);
  Settings().setValue ("view/status_head",paraHeadPat);
  Settings().setValue ("view/titlestyle",titleStyle);
  Settings().setValue ("view/titleDateForm",titleDateForm);
  Settings().setValue ("view/imgpattern",imgPattern);
  Settings().setValue ("view/maxitems",maxParagraphs);
  Settings().setValue ("view/iconlinkstyle",iconLinkStyle);
  Settings().setValue ("timers/notifydelay",notifyDelay);
}

void
TimelineView::SetView (QWebView *pv)
{
  view = pv;
  if (view) {
    connect (view, SIGNAL (linkClicked (const QUrl&)),
             this, SLOT (LinkClicked (const QUrl&)));
    mypage = new MiniPage (parentWidget);
    view->setPage (mypage);
    mypage->setLinkDelegationPolicy (QWebPage::DelegateAllLinks);
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
TimelineView::ClearList ()
{
  paragraphs.clear();
  Show ();
}


/** \LinkClicked - Chronicon has internal links, the syntax is
 *                chronicon://status/item#statusid
 *            and chronicon://search/q#searchterm
 */

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
      qDebug () << "bad URL " << url;
    }
  }
}

void
TimelineView::CustomLink (const QUrl & url)
{
  QString frag = url.fragment();
  QString path = url.path();
  QString host = url.host();
  if (host == "status" && path == "/item") {
     PagePartMap::iterator index = paragraphs.find (frag);
     if (index != paragraphs.end()) {
       QString html;
       FormatParagraph (html, index->second);
       emit ItemDialog (frag, index->second, html);
     }
  } else if (host == "search" && path == "/q") {
    QString urlString ("http://search.twitter.com/search.xml?q=%1");
    QUrl url (urlString.arg(frag.mid(1,-1)));
    QDesktopServices::openUrl (url);
  }
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
  if (doNotify && notifyDelay > 0) {
     PopupNotify (id,block);
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
  text.prepend ("my new POST: ");
  block.SetValue ("text", text);
  if (doNotify && notifyDelay > 0) {
     PopupNotify (id,block);
  }
  paragraphs[id] = block;
}

void
TimelineView::PopupNotify (QString id, StatusBlock & block)
{
  if (paragraphs.find(id) != paragraphs.end()) {
    return;
  }
  QString msg (tr("From: "));
  msg.append (block.UserValue ("screen_name"));
  msg.append (" at ");
  msg.append (block.Value ("created_at"));
  msg.append ("\n");
  msg.append (block.Value ("text"));
  NotifyNotification * note 
           = notify_notification_new (chronicon::ChroniconName,
               msg.toLocal8Bit(),
               NULL, NULL);
  if (note)  {
    notify_notification_set_timeout (note, notifyDelay);
    if (!notify_notification_show (note, NULL)) {
      qDebug () << "cannot send notification";
    }
    g_object_unref (note);
  } else {
    qDebug () << " cannot allocate notification";
  }
  
}


void
TimelineView::FormatParagraph (QString & html, const StatusBlock & para)
{
  QString bckCol = statusBackgroundColor;
  QString txtCol = textColor;
  html = paraHeadPat.arg(fontSize).arg (bckCol).arg(txtCol);
  html.append (imgPattern.arg(para.UserValue("profile_image_url"))
                         .arg(para.Id())
                         .arg(iconLinkStyle));
  QString urlPattern ("&nbsp;<a style=\"font-weight:bold;font-size:%1;\" "
                       "href=\"http://twitter.com/%2\">%2</a> ");
  html.append (urlPattern.arg(fontSize).arg(para.UserValue("screen_name")));
  html.append (tr(" "));
  html.append (FormatTextBlock (para.Value("text")));
  QDateTime now = QDateTime::currentDateTime().toUTC();
  QDateTime date = QDateTime::fromString 
                              (para.Value("created_at"),
                               "ddd MMM dd HH:mm:ss +0000 yyyy");
  date.setTimeSpec (Qt::UTC);
  int ago = date.secsTo (now);
  QString from = para.Value("source");
  if (from.length() < 1) {
    from = tr("unknown");
  }
  html.append (QString("&nbsp;<span style=\"font-size:%3;font-style:italic;\">"
                          "%1 from %2</span>")
                      .arg( Ago(ago)).arg(from).arg(fontSize));
  html.append ("</div><hr width=\"70%\">");
}

QString 
TimelineView::FormatTextBlock (const QString & text)
{
  QString subHttp = LinkMangle::Anchorize (text + QString(" "), 
                             QRegExp ("(https?://)(\\S*)"), 
                             chronicon::LinkMangle::HttpAnchor);
  QString subAt = LinkMangle::Anchorize (subHttp, 
                             QRegExp ("@(\\S*)"),
                             chronicon::LinkMangle::TwitAtAnchor);
  QString subHash = LinkMangle::Anchorize (subAt, 
                             QRegExp ("#(\\S*)"),
                             chronicon::LinkMangle::TwitHashAnchor);
  QString span  ("<span style=\"font-size:%2;\">%1</span>");
  return span.arg(subHash).arg(fontSize);
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
TimelineView::FlushParagraphs ()
{
  maxParagraphs = Settings().value ("view/maxitems",maxParagraphs).toInt();
  if (maxParagraphs > paragraphs.size()) {
    return;
  }
  PagePartMap::iterator  index;
  int toomany = paragraphs.size() - maxParagraphs;
  QStringList oldEntries;
  for (index = paragraphs.begin(); 
       toomany > 0 && index != paragraphs.end(); 
       toomany--, index++) {
    oldEntries << index->first;
  }
  QStringList::iterator sindex;
  for (sindex = oldEntries.begin(); sindex != oldEntries.end(); sindex++) {
    paragraphs.erase (*sindex);
  }
}

void
TimelineView::Show ()
{
  if (view == 0) {
    return;
  }

  FlushParagraphs ();
  QString html (dtd);
  html.append ("\n<html>\n");
  html.append (head);
  html.append ("\n<body>\n");

  QString headlinePattern = tr(("<h3 style=\"%2\">As of %1:</h3>"));
  QString date = QDateTime::currentDateTime().toString(titleDateForm);
  html.append (headlinePattern.arg(date).arg(titleStyle));
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
