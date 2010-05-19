#ifndef CHRONICON_TYPES_H
#define CHRONICON_TYPES_H

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

  enum TimelineKind {
         R_None = 0,
         R_Public = 1,
         R_Private = 2,
         R_Update = 3,
         R_Destroy = 4,
         R_ThisUser = 5,
         R_OtherUser = 6,
         R_Top
         };

  enum ApiRequestKind {
         A_None = 0,
         A_Timeline,
         A_AuthVerify,
         A_Logout,
         A_Top
  };

  enum ChronNetworkError {
        CHERR_None = 0,
        CHERR_Timeout = 9000,
        CHERR_Internal = 9999
  };
  

} // namespace

#endif
