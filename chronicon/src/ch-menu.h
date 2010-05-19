#ifndef CH_MENU_H
#define CH_MENU_H

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

#include <QMenu>

class QEvent;
class QAction;

namespace chronicon {

/** \brief Used for popup menus that can be activated by timer signals.
  *
  * The normal calling sequence is to position the menu first
  * with SetPos ( QPoint whereYouWantIt ), then set up a timer or other signal
  * to call Popup().
 */

class ChMenu : public QMenu {
Q_OBJECT

public:

  ChMenu  (QWidget *parent);
  
  QAction* Exec ();

  void SetPos (QPoint globalPos);

public slots:

  void Popup ();


protected:

  void leaveEvent (QEvent *event);


private:

  QPoint  position;


};

}


#endif
