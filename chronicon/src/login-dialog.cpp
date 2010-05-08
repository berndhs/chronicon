
#include "login-dialog.h"

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
#include "delib-debug.h"

namespace chronicon {

LoginDialog::LoginDialog (QWidget *parent)
:QDialog (parent)
{ 
  ui.setupUi (this);
  
  connect (ui.loginButton, SIGNAL (clicked()),
           this, SLOT (Login()));
  connect (ui.logoutButton, SIGNAL (clicked()),
           this, SLOT (Logout()));
  connect (ui.cancelButton, SIGNAL (clicked()),
           this, SLOT (Cancel()));
}

int
LoginDialog::Exec (QString oldUser)
{
  user = oldUser;
  ui.userEdit->setText (user);
  pass = "";
  ui.passEdit->setEchoMode (QLineEdit::Password);
  return exec ();
}

void
LoginDialog::Login ()
{
  SaveText ();
  done (1);
}

void
LoginDialog::Logout ()
{
  user = "";
  pass = "";
  done (-1);
}

void
LoginDialog::Cancel ()
{
  done (0);
}

void
LoginDialog::SaveText ()
{
  user = ui.userEdit->text();
  pass = ui.passEdit->text();
}

QString
LoginDialog::User ()
{
  return user;
}

QString
LoginDialog::Pass ()
{
  return pass;
}

} // namespace

