#ifndef SWITCH_DIALOG_H
#define SWITCH_DIALOG_H

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

#include "ui_pick-timeline.h"
#include <QDialog>
#include "chronicon-types.h"
#include <map>

namespace chronicon {

/** \brief User can choose to change to view a different timeline.
  *
  * Private, Public, own posts, other users's posts.
  * This dialog only determins the users choice, it doesnt 
  * actually switch anything.
  */

class SwitchDialog : public QDialog, public Ui_PickTimelineDialog {
Q_OBJECT

public:

  SwitchDialog (QWidget * parent);

  int     Choice ();
  QString OtherUser ();

public slots:

  int Exec ();

private slots:

  void Choose ();
  void Cancel ();

signals:

  void TimelineSwitch (int timeline, QString user);

private:

  int        timelineChoice;
  QString    userChoice;

  std::map <QAbstractButton*, int>  choiceMap;

};

} // namespace

#endif
