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

#ifndef CHRON_TEXTEDIT_H
#define CHRON_TEXTEDIT_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QResizeEvent>

namespace chronicon {

class ChronTextEdit : public QTextEdit {

Q_OBJECT

public:

  ChronTextEdit (QWidget *parent=0);
  ChronTextEdit (const QString & text, QWidget *parent=0);

  void SetListen (bool listen) { listening = listen; }
  bool IsListening () { return listening; }

  void extractHtml (QString & html, bool clearit=true);
  void extractPlain (QString & plain, bool clearit=true);
  void clearText ();

signals:

  void KeyPressed (int key);
  void ReturnPressed ();

protected:

  virtual void  keyPressEvent ( QKeyEvent * event );
  virtual void  resizeEvent (QResizeEvent * event );

private:

  bool listening;
  bool firstKey;

};


} // namespace


#endif
