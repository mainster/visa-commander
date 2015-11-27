
/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "portdialog.h"
#include "visa.h"
#include "globals.h"

#include <qglobal.h>
#include <QMessageBox>
#include <QSettings>
#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QSerialPort>
#include <QMainWindow>
#include <QDebug>



MainWindow::MainWindow(QWidget *parent) :
   QMainWindow(parent),
   ui(new Ui::MainWindow) {
   ui->setupUi(this);

   visa = Visa::getInstance();

   /**< Init ui action connections */
   initActionsConnections();

#ifdef AUTORUN
   ui->actionVisa->trigger();
#endif

   /**
    * The WA_DeleteOnClose attribute needs to be set to
    * call the destructor on a close event.
    */
   this->setAttribute(Qt::WA_DeleteOnClose);
}
/*!
 \brief

*/
MainWindow::~MainWindow() {
   QSETTINGS;
   config.setValue("MainWindow/geometry", saveGeometry());
   config.setValue("MainWindow/state", saveState());
   //   delete settings;
   delete ui;
}

/* ======================================================================== */
/*                     ui action slots                                      */
/* ======================================================================== */
/*!
 \brief

*/
void MainWindow::initActionsConnections() {
   ui->actionQuit->setEnabled(true);
   ui->actionVisa->setEnabled(true);
   ui->actionVisa->setCheckable(true);
   ui->actionSendSine->setEnabled(true);
   ui->actionRegisters->setEnabled(true);
   ui->actUnderConst->setEnabled(true);

   /**< Test forced crash functionality */
   connect( ui->actionAbout,     SIGNAL(triggered()),
            this,                SLOT(about()));
   connect( ui->actionAboutQt,   SIGNAL(triggered()),
            qApp,                SLOT(aboutQt()));
   connect( ui->actionVisa,      SIGNAL(triggered()),
            this,                SLOT(onActionVisa()));
   connect( ui->actionQuit,      SIGNAL(triggered()),
            this,                SLOT(quit()));
}
/*!
 \brief

 \param in
 \return char
*/
char * MainWindow::convert (QString in) {
   uint16_t i;
   QString str;
   bool ok;
   char * raw;

   int le = in.length()/2;
   raw = (char *) calloc(le, 1);

   ///< We expect that in is a String that encodes a sequence of hex-coded symbols
   /// "0004f408" --> 0x00 0x04 0xf4 0x08
   if (in.length() % 2) {
      qDebug() << "convert: ... mod 2 >0?!";
      return NULL;
   }
   else {
      for (i=0; i < in.length(); i+=2) {
         str = in.at(i);
         str.append( in.at(i+1) );

         *raw = (char) str.toInt(&ok,16);
         raw++;
      }
      return (raw - le);
   }
}
/*!
 \brief

*/
void MainWindow::onActionVisa() {
   QSETTINGS;

   if (ui->actionVisa->isChecked()) {
      visa->restoreGeometry(config.value("Visa/geometry").toByteArray());
      visa->restoreState(config.value("Visa/state").toByteArray());
      visa->show();
   }
   else {
      config.setValue("Visa/geometry", visa->saveGeometry());
      config.setValue("Visa/state", visa->saveState());
      visa->hide();
   }
}
/*!
 \brief

*/
void MainWindow::onBtnClearRxClicked() {
}
/*!
 \brief

*/
void MainWindow::onBtnClearTxClicked() {
}
/*!
 \brief

*/
void MainWindow::about() {
   QMessageBox::
         about(this, tr("About visa-commander"),
               tr("The <b>visa-commander</b> is used and developed as command"
                  "interface to a superb I/O hardware called <b> VisaScope </>."
                  "This qt software is the main part of my effort to gain the"
                  "functionality of a frequency response recorder...MDB"));
}
/* ======================================================================== */
/*                     pretty little helpers                                */
/* ======================================================================== */
/*!
 \brief

*/
void MainWindow::CRASHME() {
   qWarning() << "byebye";
}
/*!
 \brief

*/
void MainWindow::quit() {
//   visa->driver->settings->close();
   QSETTINGS;

   if (visa->isVisible()) {
      config.setValue("Visa/geometry", visa->saveGeometry());
      config.setValue("Visa/state", visa->saveState());
      visa->close();
   }

   this->close();
}

/* ======================================================================== */
/*                         event handlers                                   */
/* ======================================================================== */
void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton)
        Q_INFO << "right button is pressed";
    else
       Q_INFO << "mouse button";
}

