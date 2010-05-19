#ifndef LINK_MANGLE_H
#define LINK_MANGLE_H

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

/** Link Mangling functions */

class QString;
class QRegExp;

namespace chronicon {

/** \brief Supply Link transformation functions.
  *
  * This class exists solely to provide a number of text processing
  * functions that locate links in a text, and replace these links
  * with formatted Html or Twitter faux-syntax, so that the resulting
  * text contains links recognized by the viewer module.
  *
  */

class LinkMangle {

public:

/** \brief Anchorize rearranges the text argument to contain links.
  *
  * The text argument is scanned to find parts that match the \regular 
  * expression. Each matched part is given to the \anchorFunc 
  * function, and the result of that function call replaces the original
  * matched portion in the new text.
  */

static QString Anchorize (const QString &text, QRegExp regular, 
                         void (*anchorFunc)(QString&, QString));

static void HttpAnchor (QString & anchor, QString  ref);
static void TwitAtAnchor (QString & anchor , QString  ref);
static void TwitHashAnchor (QString & anchor, QString ref);


};

}

#endif
