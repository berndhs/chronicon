
#include "link-mangle.h"

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

#include <QString>
#include <QRegExp>

namespace chronicon {

QString
LinkMangle::Anchorize (const QString &text, QRegExp regular, 
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
LinkMangle::HttpAnchor (QString & anchor, QString ref)
{
  anchor =  QString("<a href=\"%1\">%1</a>").arg(ref);
}

void
LinkMangle::TwitAtAnchor (QString & anchor, QString ref)
{
  if (ref.length() == 1) {
    anchor = ref;
    return;
  }
  QString text(ref);
  if (ref.endsWith(':')) {
    ref.chop(1);
  }
  anchor = QString ("@<a href=\"http://twitter.com/%1\">%2</a>")
                 .arg (ref.mid(1)).arg(text.mid(1));
}

void
LinkMangle::TwitHashAnchor (QString & anchor, QString ref)
{
  if (ref.length() == 1) {
    anchor = ref;
    return;
  }
  anchor = QString ("<a href=\"chronicon://search/q#%1\">%1</a>")
                   .arg(ref);
}




} // namespace
