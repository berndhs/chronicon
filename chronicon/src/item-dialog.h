#ifndef ITEM_DIALOG_H
#define ITEM_DIALOG_H

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

#include <QDialog>
#include <QWebView>
#include <QString>
#include "ui_itemdetail.h"

#include "network-if.h"
#include "timeline-view.h"
#include "ch-menu.h"

namespace chronicon {

class ItemDialog : public QDialog, public Ui_ItemDialog {
Q_OBJECT

public:

  ItemDialog (QWidget * parent=0);

  void HtmlStyles ();
  void LoadSettings ();
  void SetNetwork (NetworkIF * net) { network = net; }

public slots:

  void Exec (QString id, StatusBlock block, QString itemHtml);
  void LinkClicked (const QUrl & url);

private slots:

  void Mailto   ();
  void Save     ();
  void Log      ();
  void ReTweet  ();
  void AddMessage ();
  void Delete   ();
  void Direct   ();
  void ActionMenu ();
  void Follow ();
  void GetMore ();

signals:

  void SendMessage (QString msg, QString refId);
  void MakeDirect  (QString toName);
  void MaybeFollow (StringBlock userData);
  void GetTimeline (int timeline, QString otherUser);

private:

  void SetupMenus ();

  void PlainText (QString & plain, const StatusBlock & block);

  QString UserInfoHtml ();

  NetworkIF   *network;

  ChMenu      actionMenu;

  QString       itemId;
  StatusBlock   itemBlock;


  QString dtd;
  QString head;
  QString viewBackgroundColor;
  QString statusBackgroundColor;
  QString textColor;
  QString fontSize;
  QString nickStyle;
  QString headPattern;
  QString headStyle;
  QString paraHeadPat;
  QString titleStyle;
  QString titleDateForm;
  QString imgPattern;
  QString iconLinkStyle;
  QString itemHead;



};

} // namespace

#endif
