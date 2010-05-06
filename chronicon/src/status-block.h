#ifndef STATUS_BLOCK_H
#define STATUS_BLOCK_H

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

#include <QDomElement>
#include <QString>
#include <map>
#include <QDebug>

namespace chronicon {

class StringBlock {

public:

  StringBlock () {}

  void SetValue (const QString & key, const QString & value)
                { values[key] = value; }
  QString Value (const QString & key) const ;
  bool HasValue (const QString & key) const
                { return values.find(key) != values.end(); }
  void Clear () { values.clear(); }

  
  friend QDebug & operator << (QDebug & out, const StringBlock & data);

private:

  std::map <QString, QString>   values;

};

class StatusBlock {
public:

  StatusBlock  ();
  StatusBlock (const QDomElement & dom);

  void SetContent (const QDomElement & dom);

  QString   Id () const { return ident; } 
  QString   Value (const QString & key) const;
  QString   UserValue (const QString & key) const;

  bool  HasValue (const QString & key) const { return statusValues.HasValue (key); }
  bool  HasUserValue (const QString & key) const { return userValues.HasValue (key); }

  void  SetValue (const QString & key, const QString & value);
  void  SetUserValue (const QString & key, const QString & value);

  StringBlock &   Status () { return statusValues; }
  StringBlock &   User   () { return userValues; }

  void SetStatus (const StringBlock & status) { statusValues = status; }
  void SetUser   (const StringBlock & user)   { userValues = user; }

private:

  void ParseContent (StringBlock & block, const QDomElement & dom);

  QString   ident;

  StringBlock   statusValues;
  StringBlock   userValues;
 
  friend QDebug & operator << (QDebug & out, const StatusBlock & data);
  
};

QDebug & operator << (QDebug & out, const StringBlock & data);
QDebug & operator << (QDebug & out, const StatusBlock & data);

} // namespace

#endif
