#ifndef FOLLOW_DIALOG_H
#define FOLLOW_DIALOG_H

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
#include <QString>
#include "ui_follow.h"
#include "status-block.h"

namespace chronicon {


class FollowDialog : public QDialog, public Ui_FollowDialog {
Q_OBJECT

public:

  FollowDialog (QWidget *parent);

  QString User () { return fuser; }

  /** \brief ShallFollow 1=change to yes, -1=change to no, 0=unchanged
   */
  int    ShallFollow () { return dofollow; }

public slots:

  void Exec (QString user = QString());
  void Exec (StringBlock  userData);

private slots:

  void DoFollow ();
  void UnFollow ();

signals:
  
  void Follow (QString user, int change);

private:

  QString fuser;
  int    dofollow;


};

} // namespace


#endif
