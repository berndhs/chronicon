#
# qmake pro file for chronicon gui
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
          src/cmdoptions.h


SOURCES = src/main.cpp \
          src/chronicon.cpp \
          src/chron-textedit.cpp  \
          src/delib-debug.cpp \
          src/cmdoptions.cpp

FORMS = ui/chronicon.ui \
        ui/DebugLog.ui

