#include "chron-textedit.h"
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

#include <QDebug>

namespace chronicon {

ChronTextEdit::ChronTextEdit (QWidget *parent)
:QTextEdit (parent),
 listening (true),
 firstKey (true)
{}

ChronTextEdit::ChronTextEdit (const QString & text, QWidget *parent)
:QTextEdit (text,parent),
 listening (true)
{}

void
ChronTextEdit::keyPressEvent ( QKeyEvent * event)
{
  int key (0);
  QTextEdit::keyPressEvent (event);
  if (listening && event) {
    key = event->key();
    if (firstKey) {
      emit KeyPressed (key);
      firstKey = false;
    }
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
      emit ReturnPressed ();
    }
  }
}

void 
ChronTextEdit::extractHtml (QString & html, bool clearit)
{
  firstKey = true;
  html = toHtml();
  if (clearit) { clear(); }
}

void
ChronTextEdit::extractPlain (QString & plain, bool clearit)
{
  firstKey = true;
  plain = toPlainText ();
  if (clearit) { clear (); }
}

void
ChronTextEdit::clearText ()
{
  firstKey = true;
  clear ();
}

void
ChronTextEdit::resizeEvent (QResizeEvent * event)
{
  QTextEdit::resizeEvent (event);
}

} // namespace
