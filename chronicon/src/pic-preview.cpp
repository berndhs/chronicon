
#include "pic-preview.h"

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

#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>

namespace chronicon {

PicPreview::PicPreview (QWidget * parent)
:QDialog (parent)
{
  setupUi (this);
  picArea->setScaledContents (true);

  connect (cancelButton, SIGNAL (clicked()), this, SLOT (Close ()));
  connect (backButton, SIGNAL (clicked()), this, SLOT (GetFilename()));
  connect (sendButton, SIGNAL (clicked()), this, SLOT (Send ()));
  hide ();
}


void
PicPreview::Exec ()
{
  imageName->clear ();
  messageEdit->clear ();
  show ();
  GetFilename ();
}

void
PicPreview::GetFilename ()
{
  QString tryfile = QDesktopServices::storageLocation
                           (QDesktopServices::PicturesLocation)
                    + QDir::separator() + QString ("image.jpg");
  fullName = QFileDialog::getOpenFileName (parentWidget(),
                     tr("Image to Upload"),
                     tryfile,
                     tr("Images ( *.gif *.png *.jpg )"));
  if (fullName.length () < 1) {
    Close ();
    return;
  }
  QFileInfo info (fullName);
  QString shortname = info.fileName(); // without the path
  message = shortname;
  messageEdit->setPlainText (shortname);
  imageName->setText (fullName);
  QPixmap newpix (fullName);
  QSize availSize = picArea->size();
  picpix = newpix.scaled (availSize, Qt::KeepAspectRatio);
  picArea->setPixmap (picpix);
}

void
PicPreview::Send ()
{
  message = messageEdit->toPlainText();
  if (fullName.length() > 0) {
    emit SendPic (fullName, message);
  } 
  Close ();
}

void
PicPreview::Close ()
{
  picpix.fill (Qt::white);
  picArea->setPixmap (picpix);
  hide ();
}



} // namespace

