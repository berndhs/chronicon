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

TARGET = chronicon


CONFIG += qt
CONFIG += link_pkgconfig
PKGCONFIG += libnotify

DEFINES += DELIBERATE_DEBUG=1
DEFINES += USE_OAUTH=0

#QT += core gui xml network webkit qoautch crypto
QT += core gui xml network webkit

MAKEFILE = MakeChron

UI_DIR = tmp/ui
OBJECTS_DIR = tmp/obj
MOC_DIR = tmp/moc

RESOURCES += chronicon.qrc

INCLUDEPATH += src 
#INCLUDEPATH += ../qoauth/include
#INCLUDEPATH += /usr/include/QtCrypto
INCLUDEPATH += temp/ui

#LIBS += -L../qoauth/lib -lqoauth
LIBS += 

HEADERS = src/chronicon.h \
          src/chron-textedit.h \
          src/delib-debug.h \
          src/cmdoptions.h \
          src/chron-network-reply.h \
          src/network-if.h \
          src/timeline-view.h \
          src/status-block.h \
          src/deliberate.h \
          src/item-dialog.h \
          src/version.h \


SOURCES = src/main.cpp \
          src/chronicon.cpp \
          src/chron-textedit.cpp  \
          src/delib-debug.cpp \
          src/cmdoptions.cpp \
          src/chron-network-reply.cpp \
          src/network-if.cpp \
          src/timeline-view.cpp \
          src/status-block.cpp \
          src/deliberate.cpp \
          src/item-dialog.cpp \
          src/version.cpp \


FORMS = ui/chronicon.ui \
        ui/DebugLog.ui \
        ui/enterpass.ui \
        ui/itemdetail.ui \



