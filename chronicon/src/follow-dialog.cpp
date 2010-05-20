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

#include "follow-dialog.h"
#include "status-block.h"

namespace chronicon {

FollowDialog::FollowDialog (QWidget *parent)
:QDialog(parent),
 fuser (""),
 dofollow (0)
{
  setupUi (this);
  connect (cancelButton, SIGNAL (clicked()), this, SLOT (reject()));
  connect (followButton, SIGNAL (clicked()), this, SLOT (DoFollow()));
  connect (unfollowButton, SIGNAL (clicked()), this, SLOT (UnFollow()));
}

void
FollowDialog::Exec (QString user)
{
  fuser = user;
  dofollow = false;
  nameEdit->setText (fuser);
  userInfo->clear();
  if (fuser.length() < 1) {
    nameEdit->setReadOnly (false);
  }
  exec ();
}

void
FollowDialog::Exec (StringBlock userData)
{
  fuser = userData.Value ("screen_name");
  dofollow = 0;
  nameEdit->setText (fuser);
  userInfo->clear();
  userInfo->append (userData.Value("description"));
  exec ();
}

void
FollowDialog::DoFollow ()
{
qDebug () << __FILE__ << __LINE__ << " yes";
  dofollow = 1;
  fuser = nameEdit->text();
  emit Follow (fuser, 1);
  done (1);
}

void
FollowDialog::UnFollow ()
{
qDebug () << __FILE__ << __LINE__ << " no more ";
  dofollow = -1;
  fuser = nameEdit->text();
  emit Follow (fuser, -1);
  done (-1);
}

} // namespace

