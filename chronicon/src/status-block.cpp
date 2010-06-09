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
#include <QVariant>
#include <QVariantMap>
#include <QDateTime>
#include <QTextDocument>


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
  ParseFromStatus (statusValues, dom);
  ident = statusValues.Value("id");
}

StatusBlock::StatusBlock ()
{}

void
StatusBlock::SetContent (const QDomElement & dom)
{
  statusValues.Clear ();
  userValues.Clear ();
  ParseFromStatus (statusValues, dom);
  ident = statusValues.Value ("id");
}

void
StatusBlock::SetFromUser (const QDomElement & dom)
{
  statusValues.Clear ();
  userValues.Clear ();
  ParseFromUser (userValues, dom);
  ident = statusValues.Value ("id");
}

void
StatusBlock::SetSearchContent (const QVariantMap & data)
{
  QVariantMap::const_iterator mit;
  QString  tag;
  for (mit = data.begin(); mit != data.end(); mit++) {
    tag = mit.key();

    if (tag == "from_user") {
       userValues.SetValue ("screen_name", mit.value().toString());
    } else if (tag == "from_user_id") {
       userValues.SetValue ("id", QString::number (mit.value().toULongLong()));
    } else if (tag == "id") {
       qulonglong val = mit.value().toULongLong();
       QString stval = QString::number (val);
       statusValues.SetValue ("id", stval);
       ident = stval;
    } else if (tag == "text") {
       statusValues.SetValue ("text", mit.value().toString());
    } else if (tag == "profile_image_url") {
       userValues.SetValue (tag, mit.value().toString());
    } else if (tag == "created_at") {
       QString searchData = mit.value().toString();
       QDateTime t = QDateTime::fromString (searchData,
                              "ddd, dd MMM yyyy HH:mm:ss +0000");
       statusValues.SetValue (tag,t.toString 
                              ("ddd MMM dd HH:mm:ss +0000 yyyy"));
    } else if (tag == "source") {
       QTextDocument tmp;
       tmp.setHtml (mit.value().toString());
       statusValues.SetValue (tag, tmp.toPlainText());
    } else {
       statusValues.SetValue (tag, mit.value().toString());
    }
  }
}

void
StatusBlock::SetValue (const QString & key, const QString & value)
{
  statusValues.SetValue (key, value);
  if (key == "id") {
    ident = value;
  }
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
StatusBlock::ParseFromStatus (StringBlock & block, const QDomElement & dom)
{
  QDomElement child;
  for (child = dom.firstChildElement (); !child.isNull();
       child = child.nextSiblingElement())
  {
    QString tag = child.tagName();
    if (tag == "user") {
      ParseFromUser (userValues, child);
    } else {
      block.SetValue (tag,child.text());
    }
  }
}

void
StatusBlock::ParseFromUser (StringBlock & block, const QDomElement & dom)
{
  QDomElement child;
  for (child = dom.firstChildElement (); !child.isNull();
       child = child.nextSiblingElement())
  {
    QString tag = child.tagName();
    if (tag == "status") {
      ParseFromStatus (statusValues, child);
    } else {
      block.SetValue (tag,child.text());
    }
  }
}

void
StatusBlock::Domify (QDomElement & dom)
{
  QDomDocument doc("chronicon_doc");
  QDomElement top = doc.createElement (QString("status"));
  std::map <QString,QString>::iterator index;
  for (index = statusValues.values.begin();
       index != statusValues.values.end();
       index++) {
    QDomElement child = doc.createElement (index->first);
    QDomText    text = doc.createTextNode (index->second);
    child.appendChild (text);
    top.appendChild (child);
  }
  QDomElement userPart = doc.createElement (QString("user"));
  for (index = userValues.values.begin();
       index != userValues.values.end();
       index++ ) {
    QDomElement userChild = doc.createElement (index->first);
    QDomText   text = doc.createTextNode (index->second);
    userChild.appendChild (text);
    userPart.appendChild (userChild);
  }
  top.appendChild (userPart);
  dom = top;
}



UserBlock::UserBlock (const QDomElement & dom)
{
  ParseContent (userValues, dom);
  ident = userValues.Value("id");
}


void
UserBlock::SetValue (const QString & key, const QString & value)
{
  userValues.SetValue (key, value);
}


QString
UserBlock::Value (const QString & key) const
{
  if (!userValues.HasValue(key)) {
    return QString ();
  } else {
    return userValues.Value (key);
  }
}

void
UserBlock::ParseContent (StringBlock & block, const QDomElement & dom)
{
  QDomElement child;
  for (child = dom.firstChildElement (); !child.isNull();
       child = child.nextSiblingElement())
  {
    QString tag = child.tagName();
    if (tag == "status") {
      // ignore for now, don't recurse
    } else {
      block.SetValue (tag,child.text());
    }
  }
}

void
UserBlock::Domify (QDomElement & dom)
{
  QDomDocument doc("chronicon_doc");
  QDomElement top = doc.createElement (QString("status"));
  std::map <QString,QString>::iterator index;
  for (index = userValues.values.begin();
       index != userValues.values.end();
       index++) {
    QDomElement child = doc.createElement (index->first);
    QDomText    text = doc.createTextNode (index->second);
    child.appendChild (text);
    top.appendChild (child);
  }
  QDomElement userPart = doc.createElement (QString("user"));
  for (index = userValues.values.begin();
       index != userValues.values.end();
       index++ ) {
    QDomElement userChild = doc.createElement (index->first);
    QDomText   text = doc.createTextNode (index->second);
    userChild.appendChild (text);
    userPart.appendChild (userChild);
  }
  top.appendChild (userPart);
  dom = top;
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


QDebug &
operator << (QDebug & out, const UserBlock & data)
{
  out << " UserBlock (";
  out << " userValues ( " << data.userValues << " ) "; 
  out << " ) ";
  return out;
}


} // namespace
