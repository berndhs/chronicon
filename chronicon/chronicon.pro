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

unix:{
  CONFIG += link_pkgconfig
  PKGCONFIG += libnotify
  DEFINES += USE_NOTIFY=1
}

win32: {
  DEFINES += USE_NOTIFY=0
  INCLUDEPATH += C:\Qt\qca-2.0.1-mingw\include\QtCrypto
}

DEFINES += DELIBERATE_DEBUG=1

QT += core gui xml network webkit  crypto

MAKEFILE = MakeChron

UI_DIR = tmp/ui
OBJECTS_DIR = tmp/obj
MOC_DIR = tmp/moc
RCC_DIR = tmp/rcc

RESOURCES += chronicon.qrc

INCLUDEPATH += src 
INCLUDEPATH += qoa-src
unix: {
INCLUDEPATH += /usr/include/QtCrypto
}
INCLUDEPATH += temp/ui

unix:{
  LIBS += -lqca
}
HEADERS = src/chronicon.h \
          src/chronicon-types.h \
          src/chronicon-global.h \
          src/chron-textedit.h \
          src/delib-debug.h \
          src/cmdoptions.h \
          src/chron-network-reply.h \
          src/bitly-network-reply.h \
          src/network-if.h \
          src/timeline-view.h \
          src/status-block.h \
          src/deliberate.h \
          src/item-dialog.h \
          src/direct-dialog.h \
          src/shortener.h \
          src/login-dialog.h \
          src/link-mangle.h \
          src/switch-dialog.h \
          src/follow-dialog.h \
          src/ch-menu.h \
          src/weblogin.h \
          src/webauth.h \
          src/minipage.h \
          src/config-edit.h \
          src/helpview.h \
          src/version.h \
         qoa-src/interface.h \
         qoa-src/interface_p.h \
         qoa-src/qoauth_global.h \
         qoa-src/qoauth_namespace.h \


SOURCES = src/main.cpp \
          src/version.cpp \
          src/chronicon.cpp \
          src/chronicon-types.cpp \
          src/chron-textedit.cpp  \
          src/delib-debug.cpp \
          src/cmdoptions.cpp \
          src/chron-network-reply.cpp \
          src/bitly-network-reply.cpp \
          src/network-if.cpp \
          src/timeline-view.cpp \
          src/status-block.cpp \
          src/deliberate.cpp \
          src/item-dialog.cpp \
          src/direct-dialog.cpp \
          src/switch-dialog.cpp \
          src/follow-dialog.cpp \
          src/shortener.cpp \
          src/login-dialog.cpp \
          src/link-mangle.cpp \
          src/ch-menu.cpp \
          src/weblogin.cpp \
          src/webauth.cpp \
          src/minipage.cpp \
          src/config-edit.cpp \
          src/helpview.cpp \
          qoa-src/interface.cpp \


FORMS = ui/chronicon.ui \
        ui/DebugLog.ui \
        ui/enterpass.ui \
        ui/weblogin.ui \
        ui/itemdetail.ui \
        ui/helpwin.ui \
        ui/config-edit.ui \
        ui/directmsg.ui \
        ui/pick-timeline.ui \
        ui/follow.ui \



