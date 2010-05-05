#ifndef DELIBERATE_H
#define DELIBERATE_H
//
//  Copyright (C) 2010 - Bernd H Stramm 
//
// This program is distributed under the terms of 
// the GNU General Public License version 3 
//
// This software is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty 
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//
//

#include <qapplication.h>
#include <stdio.h>
#include <QTextStream>
#include <QSettings>

#define DELIBERATE_QT_NUM (((DELIBERATE_QTM1)*10000)+((DELIBERATE_QTM2)*100)+(DELIBERATE_QTP))
#if DELIBERATE_QT_NUM > 40600
#define DELIBERATE_HAVE_WEBELT 1
#else
#define DELIBERATE_HAVE_WEBELT 0
#endif

namespace deliberate {


QTextStream  & StdOut();

void SetSettings (QSettings & settings);

QSettings & Settings ();

bool IsMaemo ();

bool IsFingerInterface ();

}


#endif
