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

#include "switch-dialog.h"
#include <QDebug>


namespace chronicon {

SwitchDialog::SwitchDialog (QWidget *parent)
:QDialog(parent),
 timelineChoice (R_None),
 textChoice ("")
{
  setupUi (this);

  connect (chooseButton, SIGNAL (clicked()), this, SLOT (Choose()));
  connect (cancelButton, SIGNAL (clicked()), this, SLOT (Cancel()));
  choiceMap[privateButton] = R_Private;
  choiceMap[publicButton] =  R_Public;
  choiceMap[ownButton]    =  R_ThisUser;
  choiceMap[otherUserButton] = R_OtherUser;
  choiceMap[mentionsButton]  = R_Mentions;
  choiceMap[retweetMineButton] = R_OwnRetweets;
  choiceMap[retweetFriendsButton] = R_FriendRetweets;
  choiceMap[searchButton] = R_SearchResults;

}

void
SwitchDialog::Cancel ()
{
  timelineChoice = R_None;
  textChoice = QString();
  reject ();
}

void
SwitchDialog::Choose ()
{
  QAbstractButton * picked = buttonChoice-> checkedButton();
  textChoice = QString();
  if (choiceMap.find (picked) != choiceMap.end()) {
    timelineChoice = choiceMap[picked];
  } else {
    timelineChoice = R_None;
  }
  if (timelineChoice == R_OtherUser) {
     textChoice = otherName->text();
  } if (timelineChoice == R_SearchResults) {
     textChoice = searchNeedle->text();
  }
  emit TimelineSwitch (timelineChoice, textChoice);
  done (timelineChoice);
}

int
SwitchDialog::Exec ()
{
  textChoice = QString();
  publicButton->setChecked (true);
  return exec ();
}

int
SwitchDialog::Choice ()
{
  return timelineChoice;
}

QString
SwitchDialog::OtherUser ()
{
  return textChoice;
}



} // namespace

