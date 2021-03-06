#ifndef MINIPAGE_H
#define MINIPAGE_H


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

#include <QWebPage>
#include <QWidget>
#include <QString>
#include <QNetworkRequest>
#include <QUrl>
#include <QLabel>
#include <QLineEdit>
#include <set>

namespace chronicon {

/** \brief Supplies a user agent string to the web server.
  *
  * Supplying the User Agent string is useful when you want the
  * web server to present a mobile-formatted web page.
  */

class MiniPage : public QWebPage {

Q_OBJECT 

public:

   MiniPage (QWidget * parent);
   
   QString userAgentForUrl ( const QUrl & url ) const;

   QString UserAgent ();
   void    SetUserAgent (const QString & uas);

public slots:
private:


   QString agentString;

};

}


#endif
