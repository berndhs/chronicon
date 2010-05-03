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

#ifndef CHRONICON_H
#define CHRONICON_H

#include <QMainWindow>
#include <QApplication>
#include <QtOAuth>
#include <QTimer>

#include "ui_chronicon.h"

#include "network-if.h"

namespace chronicon {


class Chronicon : public QMainWindow, public Ui_ChroniconWindow {
Q_OBJECT

public:

Chronicon (QWidget * parent=0);

void SetApp (QApplication *a) {pApp = a;}

public slots:

  void quit ();

private slots:

  void startMessage ();
  void finishMessage ();
  void discardMessage ();

  void firstKey (int key);
  void returnKey ();

  void Poll ();

  void DebugCheck ();

private:

  void BigEdit ();
  void SmallEdit ();
  int  normalEditVertical;
  QTimer  pollTimer;
  int     pollPeriod;

  QTimer  debugTimer;

  NetworkIF   network;

  QApplication * pApp;

};

} // namespace


#endif
