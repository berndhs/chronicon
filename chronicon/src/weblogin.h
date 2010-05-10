#ifndef WEBLOGIN_H
#define WEBLOGIN_H

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
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QByteArray>
#include <QWebPage>
#include "ui_weblogin.h"
#include "minipage.h"


namespace chronicon {

class WebLogin : public QDialog, public Ui_WebLoginDialog {
Q_OBJECT

public:

  WebLogin (QWidget * parent);

  void Start (QString site);
  void Start (const QNetworkRequest & request, 
                    QNetworkAccessManager::Operation operation =
                                    QNetworkAccessManager::GetOperation, 
              const QByteArray & body = QByteArray());

private:

  QUrl       siteUrl;
  MiniPage  *page;

};

} // namespace

#endif
