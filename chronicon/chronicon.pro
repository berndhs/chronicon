#
# qmake pro file for chronicon gui
#
#/****************************************************************
# * This file is distributed under the following license:
# *
# * Copyright (C) 2010, Bernd Stramm
# *
# *  This program is free software; you can redistribute it and/or
# *  modify it under the terms of the GNU General Public License
# *  as published by the Free Software Foundation; either version 2
# *  of the License, or (at your option) any later version.
# *
# *  This program is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *  GNU General Public License for more details.
# *
# *  You should have received a copy of the GNU General Public License
# *  along with this program; if not, write to the Free Software
# *  Foundation, Inc., 51 Franklin Street, Fifth Floor, 
# *  Boston, MA  02110-1301, USA.
# ****************************************************************/
#

TEMPLATE = app


CONFIG += qt app 

DEFINES += DELIBERATE_DEBUG=1

QT += core gui xml network qoauth crypto webkit

MAKEFILE = MakeChron

UI_DIR = temp/ui

RESOURCES +=

INCLUDEPATH += src 
INCLUDEPATH += ../qoauth/include
INCLUDEPATH += /usr/include/QtCrypto
INCLUDEPATH += temp/ui

LIBS += -L../qoauth/lib -lqoauth

HEADERS = src/chronicon.h \
          src/chron-textedit.h \
          src/delib-debug.h \
          src/cmdoptions.h \
          src/chron-network-reply.h \
          src/network-if.h


SOURCES = src/main.cpp \
          src/chronicon.cpp \
          src/chron-textedit.cpp  \
          src/delib-debug.cpp \
          src/cmdoptions.cpp \
          src/chron-network-reply.cpp \
          src/network-if.cpp

FORMS = ui/chronicon.ui \
        ui/DebugLog.ui

