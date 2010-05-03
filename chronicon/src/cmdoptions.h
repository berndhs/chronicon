#ifndef CMDOPTIONS_H
#define CMDOPTIONS_H

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

#include <QString>
#include <QVariant>
#include <QStringList>
#include <map>

using namespace std;


namespace deliberate {


class CmdOptions {

public:

enum OptType { Opt_None,
               Opt_Int,
               Opt_QStr,
               Opt_Solo,
               Opt_Bad
             };

CmdOptions (const QString pgmname);

bool Parse (int argc, char*argv[]);
bool Bad ();

QStringList Arguments ();

void Usage ();

bool WantHelp ();
bool WantVersion ();

void AddStrOption  (const QString longName, 
                    const QString shortname, 
                    const QString msg);
void AddIntOption  (const QString longName, 
                    const QString shortname, 
                    const QString msg);
void AddSoloOption (const QString longName, 
                    const QString shortname, 
                    const QString msg);

/** @brief these SetSomething functions set the option variable
  * if it was specified on the command line, otherwise they leave
  * the parameter alone. The bool return says whether or not the 
  * parameter was changed
  */
  
bool SeenOpt (const QString name);
bool SetStringOpt (const QString longName, QString & opt);
bool SetIntOpt    (const QString longName, int & opt);
bool SetSoloOpt   (const QString longName, bool & seenIt);

private:

   class Option {
    
    public:
    
       OptType  theType;
       QVariant theValue;
       QString  theMsg;
       QString  longName;
       QString  shortName;
       bool     seenIt;
   };

   
   void PrintAll ();
   
   void ParseLong (QString wd, char* argv[], int & pos, int posmax);
   void ParseShort (QString wd, char* argv[], int & pos, int posmax);
   
   typedef map <QString, Option*> OptValuesType;
   typedef map <QString, QString> ShortToLongType;
   
   OptValuesType   mOptValues;
   ShortToLongType mLongNameOf;
   
   QStringList   mArgs;
   
   
   QString mPgm;
   
   bool    bad;
   
   bool Seen (QString name);
   Option * AddOption  (const QString longName, 
                    const QString shortname, 
                    const QString msg);
   
   static QString  trHelp;
   static QString  trH;
   static QString  trVersion;
   static QString  trV;
};


}


#endif
