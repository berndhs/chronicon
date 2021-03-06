#include "deliberate.h"


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
 
namespace deliberate {

QTextStream & StdOut ()
{
  static QTextStream *out(0);
  
  if (out == 0) {
    out = new QTextStream (stdout);
  }
  return *out;
}


static QSettings * mySettings(0);
  
void
SetSettings (QSettings & settings)
{
  if (mySettings) {
    delete mySettings;
  }
  mySettings = &settings;
}

QSettings &
Settings ()
{
  if (mySettings) {
    mySettings = new QSettings;
  }
  return *mySettings;
}

bool
IsMaemo ()
{
  #ifdef Q_WS_MAEMO_5
  return true;
  #else
  return false;
  #endif
}

bool
IsFingerInterface ()
{
  return IsMaemo ();
}

void
Rot1 (QByteArray &data, const QByteArray &key)
{
  int len (key.size());
  for (int c = 0; c<data.size(); c++) {
    int cc = data[c];
    int k = c % len;
    data[c] = (cc ^ key[k]);
  }
  data = data.toBase64();
}

void
Rot2 (QByteArray &data, const QByteArray &key)
{
  data = QByteArray::fromBase64(data);
  int len (key.size());
  for (int c = 0; c<data.size(); c++) {
    int cc = data[c];
    int k = c % len;
    data[c] = (cc ^ key[k]);
  }
}


}
