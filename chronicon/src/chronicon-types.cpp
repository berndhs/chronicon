
#include "chronicon-types.h"

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

namespace chronicon {

QString
timelineName (TimelineKind kind)
{
  QString name;
  switch (kind) {
  case R_Public:
    name = "public_timeline";
    break;
  case R_Private:
    name = "home_timeline";
    break;
  case R_ThisUser:
    name = "user_timeline";
    break;
  case R_OtherUser:
    name = "user_timeline";
    break;
  case R_Mentions:
    name = "mentions";
    break;
  case R_OwnRetweets:
    name = "retweeted_by_me";
    break;
  case R_FriendRetweets:
    name = "retweeted_to_me";
    break;
  default:
    name = "public_timeline";
    break;
  }
  return name;
}

} // namespace

