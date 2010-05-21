
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

#include "direct-dialog.h"
#include "shortener.h"
#include "delib-debug.h"

namespace chronicon {

/*! \brief Dialog for sending a direct message
  */

DirectDialog::DirectDialog (QWidget *parent)
:QDialog (parent),
 network (0),
 shortener (this)
{
  ui.setupUi (this);
  connect (ui.cancelButton, SIGNAL (clicked()), this, SLOT (reject()));
  connect (ui.sendButton, SIGNAL (clicked()), this, SLOT (SendMessage()));
  connect (&shortener, SIGNAL (DoneShortening (QString)),
             this, SLOT (CatchShort (QString)));
}

void
DirectDialog::SetNetwork (NetworkIF* net)
{
  shortener.SetNetwork (net);
  network = net;
}


void
DirectDialog::WriteMessage (QString toName)
{
  destUserName = toName;
  ui.nameEdit->setText (toName);
  ui.nameEdit->setReadOnly (true);
  ui.messageEdit->clear();
  exec ();
}

void
DirectDialog::SendMessage ()
{
  QString body = ui.messageEdit->toPlainText ();
  bool wait;
  shortener.ShortenHttp (body, wait);
  if (!wait) {
    emit SendDirect (destUserName, body);
    accept ();
  }
}

void
DirectDialog::CatchShort (QString shortMsg)
{
  ui.messageEdit->setPlainText (shortMsg);
  emit SendDirect (destUserName, shortMsg);
  accept ();
}

} // namespace
