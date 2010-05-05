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

#include "status-block.h"


namespace chronicon {

QString
StringBlock::Value (const QString & key) const
{
  std::map<QString,QString>::const_iterator index = values.find(key);
  if (index == values.end()) {
    return QString();
  } else {
    return index->second;
  }
}

StatusBlock::StatusBlock (const QDomElement & dom)
{
  ParseContent (statusValues, dom);
  ident = statusValues.Value("id");
}

StatusBlock::StatusBlock ()
{}

void
StatusBlock::SetContent (const QDomElement & dom)
{
  statusValues.Clear ();
  userValues.Clear ();
  ParseContent (statusValues, dom);
  ident = statusValues.Value ("id");
}

void
StatusBlock::SetValue (const QString & key, const QString & value)
{
  statusValues.SetValue (key, value);
}

void
StatusBlock::SetUserValue (const QString & key, const QString & value)
{
  userValues.SetValue (key, value);
}

QString
StatusBlock::Value (const QString & key) const
{
  if (!statusValues.HasValue(key)) {
    return QString ();
  } else {
    return statusValues.Value (key);
  }
}

QString
StatusBlock::UserValue (const QString & key) const
{
  if (!userValues.HasValue(key)) {
    return QString ();
  } else {
    return userValues.Value (key);
  }
}

void
StatusBlock::ParseContent (StringBlock & block, const QDomElement & dom)
{
  QDomElement child;
  for (child = dom.firstChildElement (); !child.isNull();
       child = child.nextSiblingElement())
  {
    QString tag = child.tagName();
    if (tag == "user") {
      ParseContent (userValues, child);
    } else {
      block.SetValue (tag,child.text());
    }
  }
}

QDebug & 
operator << (QDebug & out, const StringBlock & data)
{
  std::map<QString,QString>::const_iterator index;
  out << " StringBlock (";
  for (index = data.values.begin(); index != data.values.end(); index++) {
     out << "( " << index->first << " , " 
         << index->second << " )";
  }
  out << " ) ";
  return out;
}

QDebug &
operator << (QDebug & out, const StatusBlock & data)
{
  out << " StatusBlock (";
  out << " statusValues ( " << data.statusValues << " ) "; 
  out << " userValues ( " << data.userValues << " ) ";
  out << " ) ";
  return out;
}

} // namespace
