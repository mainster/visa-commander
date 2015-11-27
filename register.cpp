
#include "register.h"
#include "ui_register.h"
#include "globals.h"

#include <QMessageBox>
#include <QDockWidget>
#include <QLabel>
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <stdint.h>
#include <QTimer>
#include <QTextEdit>
#include <QGridLayout>
#include <QDebug>


/*!
 \brief

 \param parent
*/
Register::Register(QWidget *parent) :
   QDialog(parent),
   ui(new Ui::Register) {
   ui->setupUi(this);
   this->resize(950, 600);

   int i;
   QString htmlStr =
         "Reg_<span style=\" font-weight:600; \""
         ">0x%1 </span><span style=\" color:#909090; "
         "\">(%2): </span>";

   for (i=0; i < MAX_ROWS; i++) {
      labelArrC1[i] = new QLabel(QString(htmlStr).arg(i, 2, 16, QLatin1Char('0'))
                                 .arg(i, 3, 10, QLatin1Char(' ')));
      labelArrC1[i]->setMaximumHeight(15);
      labelArrC1[i]->setMaximumWidth(100);

      leArrC2[i] = new QLineEdit();
      leArrC2[i]->setMaximumHeight(20);
      leArrC2[i]->setMaximumWidth(85);
      leArrC2[i]->setMinimumHeight(20);
      leArrC2[i]->setMinimumWidth(85);
      leArrC2[i]->setReadOnly(false);
      leArrC2[i]->setAlignment(Qt::AlignCenter);
   }
   QGridLayout *layout = new QGridLayout;

   for (i=0; i < MAX_ROWS/2; i++) {
      layout->addWidget(labelArrC1[i], i, 0);
      layout->addWidget(leArrC2[i], i, 1);
   }
   for (i=0; i < MAX_ROWS/2; i++) {
      layout->addWidget(labelArrC1[i + MAX_ROWS/2], i, 3);
      layout->addWidget(leArrC2[i + MAX_ROWS/2], i, 4);
   }
   spacer = new QSpacerItem(20,20);
   layout->addItem(spacer, 1, 2);

   /**
     * Set frame layout and configure window for its geometry, name...
     */
   setLayout(layout);
   setGeometry(1935, 60, 324, 540);
   setWindowTitle("VisaScope Registers 0x00..0x27");
   /** Store reg addresses of interesting regs in a vector */
   vInterest.append(0x06);
   vInterest.append(0x07);

   setBgColorOfInterestings();

   QSETTINGS;
   restoreGeometry(config.value("Register/geometry").toByteArray());

   /**
    * The WA_DeleteOnClose attribute needs to be set to
    * call the destructor on a close event.
    */
   this->setAttribute(Qt::WA_DeleteOnClose);

}

/*!
 \brief

*/
Register::~Register() {
   QSETTINGS;
   config.setValue("Register/geometry", saveGeometry());
   delete ui;
}
/*!
 \brief

*/
void Register::onBtnOneClicked()
{
   QMessageBox *box1;
   box1 = new QMessageBox(0);
   box1->setText("Message box 1");
   box1->show();
}
/*!
 \brief

 \param addr
 \param text
*/
void Register::setRegister(uint8_t addr, QString text) {
   if (addr >= MAX_ROWS)
      return;
   leArrC2[addr]->setText( text );
}
/*!
 \brief

 \param wmax
*/
void Register::setLeWidth(uint16_t wmax) {
   setLeWidth(wmax, 70);
}
/**
 * Fade out all line edit background colors by n steps
 */
/*!
 \brief

 \param steps
*/
void Register::doFadeOut(uint8_t steps) {
   int i; QPalette p;

   for (i = 0; i < MAX_ROWS; i++) {
      /**
         * Check if actual i is one of the interesting register addresses
         * that are stored in qvector vInteres
         */
      if (vInterest.indexOf(i) == -1) {
         p = leArrC2[i]->palette();
         QColor bgcolor = p.color(QPalette::Base);
         QColor bgcolorLight = bgcolor.lighter(steps);
         p.setColor( QPalette::Base, bgcolorLight );
         leArrC2[i]->setPalette( p );
      }
   }
}
/**
 * Apply light blue color background to interesting register addresses
 */
/*!
 \brief

*/
void Register::setBgColorOfInterestings() {
   QPalette p; int i;

   p.setColor( QPalette::Base, QColor(0xcc, 0xff, 0xff) );
   for (i = 0; i < vInterest.size(); i++) {
      leArrC2[ vInterest.at(i) ]->setPalette( p );
   }
}
/*!
 \brief

 \param differs
*/
void Register::setBgColor(QByteArray& differs) {
   QPalette p = leArrC2[0]->palette();
   p.setColor( QPalette::Base, QColor(0xff, 0x1f, 0x1f) );
   int i, idx;

   for (i = 0; i < differs.length(); i++) {
      /**
         * Do not overwrite bgcolor of registered (int vInterest)
         * Interestings reg addresses
         */
      idx = differs[i];

      if (vInterest.indexOf( idx ) == -1) {
         if (idx >= MAX_ROWS) {
            qDebug() << "OUT OF BOUNDS";
            while(1);;
         }
         leArrC2[ idx ]->setPalette( p );
      }
   }
}
/*!
 \brief

 \param wmax
 \param wmin
*/
void Register::setLeWidth(uint16_t wmax, uint16_t wmin = 70) {
   if (wmax < wmin)
      wmin = wmax;

   int i;
   for (i = 0; i < MAX_ROWS; i++) {
      leArrC2[i]->setMaximumWidth(wmax);
      leArrC2[i]->setMinimumWidth(wmin);
   }
}

bool Register::eventFilter(QObject *obj, QEvent *event) {
   if (obj == leArrC2[0]) {
      if (event->type() == QEvent::MouseButtonPress) {
         qDebug() << "here you can call the slot using emit or u can call a function";
         return true;
      }
      else {
         return false;
      }
   } else {

      // pass the event on to the parent class
      return Register::eventFilter(obj, event);
   }
}
