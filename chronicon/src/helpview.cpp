
#include "helpview.h"
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
#include "delib-debug.h"
#include <QFile>
#include <QString>

namespace deliberate {


 HelpView::HelpView (QWidget *parent)
 :QDialog(parent)
 {
   setupUi (this);
   ConnectButtons ();
   hide();
 }

 HelpView::~HelpView ()
 {
 
 }
 
 void
 HelpView::update ()
 {
   box->update ();
   QWidget::update ();
 }

 void 
 HelpView::ConnectButtons ()
 {
   connect (closeButton, SIGNAL (clicked()), this, SLOT (DoClose()));
   connect (backButton, SIGNAL (clicked()), this, SLOT (DoBack()));
   connect (forwardButton, SIGNAL (clicked()), this, SLOT (DoForward()));
 }

 void
 HelpView::DoClose ()
 {
   hide();
 }
 
 void
 HelpView::DoBack ()
 {
   box->back();
 }
 
 void
 HelpView::DoForward ()
 {
   box->forward();
 }
 
 void
 HelpView::Show (QString urlString)
 {
   QUrl url(urlString);
   box->load (url);
   show ();
 }
 
} // namespace


