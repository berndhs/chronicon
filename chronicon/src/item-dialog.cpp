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

#include "item-dialog.h"
#include "deliberate.h"
#include "link-mangle.h"
#include <QDesktopServices>
#include <QWebPage>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

using namespace deliberate;

namespace chronicon {

ItemDialog::ItemDialog (QWidget *parent)
:QDialog(parent),
 network(0),
 actionMenu (this)
{
  setupUi (this);
  HtmlStyles ();
  SetupMenus ();

  connect (cancelButton, SIGNAL (clicked()), this, SLOT (reject()));
  connect (actionButton, SIGNAL (clicked()), this, SLOT (ActionMenu()));
  connect (itemView, SIGNAL (linkClicked (const QUrl&)),
             this, SLOT (LinkClicked (const QUrl&)));
  QWebPage *myPage = itemView->page();
  if (myPage) {
    myPage->setLinkDelegationPolicy (QWebPage::DelegateAllLinks);
  }
}

void
ItemDialog::SetupMenus ()
{
  actionMenu.addAction (tr("E-Mail"), this, SLOT (Mailto()));
  actionMenu.addAction (tr("ReTweet"), this, SLOT (ReTweet()));
  actionMenu.addAction (tr("ReTweet-Plus"), this, SLOT (AddMessage()));
  actionMenu.addAction (tr("Save Message"), this, SLOT (Save()));
  actionMenu.addAction (tr("Delete My Message"), this, SLOT (Delete()));
  actionMenu.addAction (tr("Send Direct Message"), this, SLOT (Direct()));
}

void
ItemDialog::ActionMenu ()
{
  actionMenu.SetPos (mapToGlobal(actionButton->pos()));
  QTimer::singleShot (50, &actionMenu, SLOT (Popup()));
}

void
ItemDialog::HtmlStyles ()
{
  dtd = QString 
("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\""
 " \"http://www.w3.org/TR/html4/loose.dtd\">");
  statusBackgroundColor = "f0f0f0";
  headPattern = QString ("<head><title>Tweet List</title><meta http-equiv="
             "\"Content-Type\" content=\"text/html;charset=utf-8\" >%1</head>");
  headStyle = QString ("<style type=\"text/css\"> body { background-color:#e0e0e0;} "
                 "p { font-size:small; background-color:%1; "
                    " padding:2px; margin:2x; "
                    " font-family:Times New Roman; } </style>");
  head = headPattern.arg (headStyle.arg(statusBackgroundColor));
  textColor = "000000";
  fontSize = "90%";
  nickStyle = "font-weight:bold;text-decoration:none;";
  titleStyle = "font-size:smaller; color:0f2f0f;";
  titleDateForm = tr("ddd hh:mm:hh");
  imgPattern = QString ("<div style=\"float:left;margin:3px;\">"
                    "<a href=\"chronicon://status/item#%2\" style=\"%3\">"
                    "<img border=\"0\"src=\"%1\" width=\"48\" height=\"48\" "
                      " style=\"vertical-align:text-top;\" />"
                     "</a></div>");
  iconLinkStyle = "text-decoration:none;";
}

void
ItemDialog::LoadSettings ()
{
  dtd = Settings().value ("view/DTD", dtd).toString();
  statusBackgroundColor = Settings()
                  .value ("view/status_background_color",statusBackgroundColor)
                   .toString();
  headPattern = Settings().value ("view/headpattern",headPattern).toString();
  headStyle = Settings().value ("view/headstyle", headStyle).toString();
  head = headPattern.arg (headStyle.arg(statusBackgroundColor));
  textColor = Settings().value("view/textcolor", textColor).toString();
  fontSize = Settings().value("view/fontsize",fontSize).toString();
  nickStyle = Settings().value("view/nickstyle",nickStyle).toString();
  paraHeadPat = QString ( "<div style=\"width:100%;"
                        "float:left;"
                        "font-size:%1;background-color:#%2;"
                         "color:%3\">");
  paraHeadPat = Settings().value ("view/status_head",paraHeadPat).toString();
  titleStyle = Settings().value ("view/titlestyle",titleStyle).toString();
  titleDateForm = Settings().value ("view/titledateform",titleDateForm)
                            .toString();
  imgPattern = Settings().value ("view/imgpattern",imgPattern).toString();
  iconLinkStyle = Settings().value ("view/iconlinkstyle", iconLinkStyle).toString();
  Settings().setValue ("view/DTD",dtd);
  Settings().setValue ("view/headpattern",headPattern);
  Settings().setValue ("view/status_background_color",statusBackgroundColor);
  Settings().setValue ("view/textcolor",textColor);
  Settings().setValue ("view/fontsize",fontSize);
  Settings().setValue ("view/nickstyle",nickStyle);
  Settings().setValue ("view/headstyle",headStyle);
  Settings().setValue ("view/status_head",paraHeadPat);
  Settings().setValue ("view/titlestyle",titleStyle);
  Settings().setValue ("view/titleDateForm",titleDateForm);
  Settings().setValue ("view/imgpattern",imgPattern);
  Settings().setValue ("view/iconlinkstyle",iconLinkStyle);
}


void
ItemDialog::Exec  (QString id, StatusBlock  block, QString itemHtml)
{
  itemId = id;
  itemBlock = block;
  QString html (dtd);
  html.append ("<html>");
  html.append (itemHead);
  html.append ("<body>");
  html.append (itemHtml);
  html.append (UserInfoHtml());
  html.append ("</body>");
  html.append ("</html>");
  itemView->setHtml (html);
qDebug () << __FILE__ << __LINE__ << block;
  exec ();
}

void
ItemDialog::PlainText (QString & plain, const StatusBlock & block)
{
  plain = tr("Twitter Message from ");
  plain.append (block.UserValue ("name"));
  plain.append (tr(" a.k.a "));
  plain.append (block.UserValue ("screen_name"));
  plain.append ("\n");
  plain.append (tr("sent on "));
  plain.append (block.Value("created_at"));
  plain.append ("\n\n");
  plain.append (block.Value ("text"));
}

QString
ItemDialog::UserInfoHtml ()
{
  QString html;
  html.append ("<div style=\"font-size:small;\">");
  html.append (tr("<p>Message Id %1</p>").arg(itemBlock.Value("id")));
  html.append (tr("<p>User Info:"));
  html.append ("<ul>");
  html.append (tr("<li>User %1 (%2) id %3</li>")
                   .arg(itemBlock.UserValue("screen_name"))
                   .arg(itemBlock.UserValue("name"))
                   .arg(itemBlock.UserValue("id")));
  html.append (tr("<li>Time Zone: %1</li>")
                 .arg(itemBlock.UserValue("time_zone")));

  QString site = itemBlock.UserValue("url");
  QString siteHttp = LinkMangle::Anchorize (site + QString(" "), 
                             QRegExp ("(https?://)(\\S*)"), 
                             chronicon::LinkMangle::HttpAnchor);
  html.append (tr("<li>Site: %1</li>").arg(siteHttp));
  html.append (
     tr("<li>Bio: <span style=\"font-style:italic;\">%1</span></li>")
       .arg(itemBlock.UserValue("description")));
  html.append ("</ul></p>");
  html.append("</div>");
  return html;
}

void
ItemDialog::AddMessage ()
{
  QString newmsg;
  newmsg.append ("RT ");
  newmsg.append ("@");
  newmsg.append (itemBlock.UserValue("screen_name"));
  newmsg.append (": ");
  newmsg.append (itemBlock.Value("text"));
  emit SendMessage (newmsg,itemBlock.Id());
  accept ();
}

void
ItemDialog::Mailto ()
{
  QString subject = tr ("Twitter Message from ");
  subject.append (itemBlock.UserValue("screen_name"));
  QString plainText;
  PlainText (plainText, itemBlock);
  QUrl url(QString("mailto:?subject=")
                   + subject
                   + "&body=" + plainText);
  QDesktopServices::openUrl (url);
  accept ();
}

void
ItemDialog::Log ()
{
  qDebug () << "Log item " << itemId;
  QDomElement  elt;
  itemBlock.Domify (elt);
  QDomDocument doc;
  QString emptyDoc ("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
  emptyDoc.append ("<chronicon_status_list version=\"1.0\">\n");
  emptyDoc.append ("</chronicon_status_list>\n");
  doc.setContent (emptyDoc);
  QDomElement root = doc.documentElement ();
  root.appendChild (elt);
  accept ();
}

void
ItemDialog::Save ()
{
  QString plain;
  PlainText (plain, itemBlock);
  QString filename (itemBlock.UserValue ("screen_name"));
  filename.append ("_");
  filename.append (itemId);
  filename.append (".txt");
  QString defaultDir = Settings().value ("defaultDir",QString(".")).toString();
  defaultDir.append (QDir::separator());
  defaultDir.append (filename);
  QString fullName = QFileDialog::getSaveFileName (this,
             tr ("Save Message Text"),
             defaultDir,
             tr ("Text Files (*.txt) ;;All Files (* *.*)")
             );
  if (fullName.length() > 0) {
    QFile file (fullName);
    bool opened = file.open (QFile::WriteOnly) ;
    if (opened) {
      int bytes = file.write (plain.toLocal8Bit());
      if (bytes > 0) {
        QFileInfo info (file);
        Settings().setValue ("files/savedir",info.absolutePath());
      }
    } else {
      QMessageBox box;
      QString failMsg (tr("Could not open %1 to write"));
      box.setText (failMsg.arg(fullName));
      box.exec ();
    }
  }
  accept ();
}

void
ItemDialog::ReTweet ()
{
  if (network) {
    network->ReTweet (itemId);
    accept ();
  } else {
    qDebug () << __FILE__ << __LINE__ << " no network for ReTweet!";
    reject ();
  }
}

void
ItemDialog::Delete ()
{
  if (network) {
    network->PushDelete (itemId);
    accept ();
  } else {
    qDebug () << __FILE__ << __LINE__ << " not network for Delete!";
    reject ();
  }
}

void
ItemDialog::Direct ()
{
  reject ();
  QString screen_name = itemBlock.UserValue ("screen_name");
  emit MakeDirect (screen_name);
}

void
ItemDialog::LinkClicked (const QUrl & url)
{
  if (url.isValid()) {
    QDesktopServices::openUrl (url);
  }
}


} // namespace
