#include "minipage.h"
#include <QWebFrame>
#include <QWebView>
#include <QFileDialog>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QCursor>


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

namespace chronicon {


MiniPage::MiniPage (QWidget * parent)
{
  setParent (parent);
  // agentString = QString ("Maxwell Smart; Qt WebKit; Safari");
  agentString = "using Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16";
}

QString
MiniPage::userAgentForUrl (const QUrl & url ) const
{
    return agentString;
}

void
MiniPage::SetUserAgent (const QString &uas)
{
  agentString = uas;
}

QString
MiniPage::UserAgent ()
{
  return agentString ;
}

} // namespace
