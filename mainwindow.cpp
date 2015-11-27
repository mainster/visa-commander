#include <QtCore>
#include <QtGui>
#include <QMessageBox>
#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"


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
void MainWindow::onBtnClearRxClicked() {
}
void MainWindow::onBtnClearTxClicked() {
}
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
void MainWindow::CRASHME() {
   qWarning() << "byebye";
}
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

