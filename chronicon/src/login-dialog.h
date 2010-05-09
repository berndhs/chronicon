#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

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
#include "ui_enterpass.h"

namespace chronicon {

class NetworkIF;

class LoginDialog : public QDialog {
Q_OBJECT

public:

  LoginDialog (QWidget *parent, NetworkIF * netIF);

  /** Exec() - return 1 for log in, 0 for nothing done, -1 for log out
  */
  int Exec (QString oldUser);
  
  QString User ();
  QString Pass ();

public slots:

  void AuthOK ();
  void AuthBad ();

private slots:

  void Login ();
  void Logout ();
  void Cancel ();

private:

  NetworkIF  * network;
  void SaveText ();

  QString    user;
  QString    pass;

  Ui_EnterPass  ui;

};

} // namespace

#endif
