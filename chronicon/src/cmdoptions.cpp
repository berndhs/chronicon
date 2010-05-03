#include "cmdoptions.h"
#include <iostream>
#include <QObject>

//
//  Copyright (C) 2009 - Bernd H Stramm 
//
// This program is distributed under the terms of 
// the GNU General Public License version 3 
//
// This software is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty 
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//

using namespace std;

namespace deliberate {

QString CmdOptions::trHelp (QObject::tr("help"));
QString CmdOptions::trH    (QObject::tr("h"));
QString CmdOptions::trVersion (QObject::tr("version"));
QString CmdOptions::trV       (QObject::tr("v"));

CmdOptions::CmdOptions (QString pgmname)
   :mPgm("")
{
  bad = false;
  mPgm = pgmname;
  AddSoloOption (trHelp,
                 trH,
                 QObject::tr("Give help message and exit"));
  AddSoloOption (trVersion,
                 trV,
                 QObject::tr("Give version and exit"));
	;
}

QStringList
CmdOptions::Arguments ()
{
  return mArgs;
}

void
CmdOptions::Usage ()
{
  cout << mPgm.toStdString() << QObject::tr(":").toStdString() << endl;
  OptValuesType::iterator it;
  string tab = QObject::tr("\t\t").toStdString();
  for (it = mOptValues.begin(); it != mOptValues.end(); it++) {
    Option *opt = it->second;
    cout << QObject::tr("--").toStdString() << opt->longName.toStdString()
         << QObject::tr(" ( -").toStdString() << opt->shortName.toStdString() 
         << QObject::tr(") ").toStdString(); 
    if (opt->theType != Opt_Solo) {
      cout << QObject::tr(" value  ").toStdString();
    }
    cout << tab << opt->theMsg.toStdString()    
         << endl;
  }
}

bool
CmdOptions::Parse (int argc, char *argv[])
{
  try {
    int a(1);
    bool keepParsing (true);
    QString word;
    while (a < argc) {
      word = QString (argv[a]);
      if (word == "--") {
        keepParsing = false;
      } else if (keepParsing && word.startsWith("--")) {
        ParseLong (word, argv, a, argc+1);
      } else if (keepParsing && word.startsWith("-")) {
        ParseShort (word, argv, a, argc+1);
      } else {
        mArgs += word;
      }
      a++;
    }
  } catch (...) {
    bad = true;
    return false;
  }
  bad = false;
  return true;  
}

void
CmdOptions::ParseLong (QString wd, char *argv[], int & pos, int posmax)
{
  int len = wd.length();
  if (len < 3) {
    throw -1;
  }
  QString name = wd.right(len-2);
  if (mOptValues.count(name) < 1) { // invalid option
    throw -1;
  }
  Option * opt = mOptValues[name];
  if (opt == 0) { // corrupt option map
    throw -2;
  }
  opt->seenIt = true;
  if (opt->theType == Opt_Solo) {
    opt->theValue.setValue(true);
    return;
  } else {
    pos++;
    if (pos > posmax) {
      throw -1;
    }
    QString value(argv[pos]);
    switch (opt->theType) {
    case Opt_Int:
      opt->theValue.setValue (value.toInt());
      break;
    case Opt_QStr:
      opt->theValue.setValue (value);
      break;
    default: // corrupt option map
      throw -2;
    }
  }
}


void
CmdOptions::ParseShort (QString wd, char* argv[], int & pos, int posmax)
{
  int len = wd.length();
  if (len < 2) {
    throw -1;
  }
  QString name = wd.right(len-1);
  if (mLongNameOf.count(name) < 1) {
    throw -1;
  }
  QString lngWd ("--");
  lngWd.append(mLongNameOf[name]);
  ParseLong (lngWd, argv, pos, posmax);
}

CmdOptions::Option *
CmdOptions::AddOption (const QString longName, 
                      const QString shortName, 
                      const QString msg)
{
  if (shortName.length() > 1) {
    throw -3;
  }
  Option * thisopt = new Option;
  if (thisopt == 0) {
    throw -2;
  }
  thisopt->seenIt = false;
  thisopt->theMsg = msg;
  thisopt->longName = longName;
  thisopt->shortName = shortName;
  mLongNameOf[shortName] = longName;
  mOptValues[longName] = thisopt;
  return thisopt;
}

void
CmdOptions::AddStrOption (const QString longName,
                          const QString shortName,
                          const QString msg)
{
  Option * thisopt = AddOption (longName, shortName, msg);
  thisopt->theType = Opt_QStr;
  thisopt->theValue.setValue(QString(""));
}

void
CmdOptions::AddIntOption (const QString longName,
                          const QString shortName,
                          const QString msg)
{
  Option * thisopt = AddOption (longName, shortName, msg);
  thisopt->theType = Opt_Int;
  thisopt->theValue.setValue(0);
}

void
CmdOptions::AddSoloOption (const QString longName,
                          const QString shortName,
                          const QString msg)
{
  Option * thisopt = AddOption (longName, shortName, msg);
  thisopt->theType = Opt_Solo;
}

bool
CmdOptions::SeenOpt (const QString name)
{
  if (mOptValues.count(name) > 0) {
    return mOptValues[name]->seenIt;
  } 
  return false;
}


bool
CmdOptions::SetStringOpt (const QString longName, QString & opt)
{
  bool seenOpt = SeenOpt (longName);
  if (seenOpt) {
    opt = mOptValues[longName]->theValue.toString();
    return true;
  }
  return false;
}

bool
CmdOptions::SetIntOpt (const QString longName, int & opt)
{
  bool seenOpt = SeenOpt(longName);
  if (seenOpt) {
    opt = mOptValues[longName]->theValue.toInt();
    return true;
  }
  return false;
}

bool
CmdOptions::SetSoloOpt (const QString longName, bool & seenIt)
{
  seenIt = SeenOpt(longName);
  return seenIt;
}

bool
CmdOptions::WantHelp ()
{
  return SeenOpt(trHelp);
}

bool
CmdOptions::WantVersion ()
{
  return SeenOpt(trVersion);
}

} // namespace
