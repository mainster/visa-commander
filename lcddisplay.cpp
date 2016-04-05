#include "lcddisplay.h"
#include "fugen.h"

#define  DELTA_NORM  120


int LcdDisplay::ali = 0;

LcdDisplay::LcdDisplay(QString lableStr, QWidget *parent) :
   QLCDNumber(parent) {

   this->setObjectName(tr("lcd%1").arg(lableStr));
   mSelected = Lcd::type_none;

   mlable = new QLabel(lableStr, this);
   mlable->setVisible(true);

   al.append(Qt::AlignLeft);
   al.append(Qt::AlignCenter);
   al.append(Qt::AlignHCenter);
   al.append(Qt::AlignJustify);
   al.append(Qt::AlignRight);
   al.append(Qt::AlignAbsolute);

   QGridLayout *gl = new QGridLayout(this);
   gl->addWidget(mlable,0,0);
   this->setLayout(gl);

   /** LCD attributes */
   setSizePolicy(QSizePolicy::Expanding,
                 QSizePolicy::Expanding);
   setMinimumSize(100, 28);
   setMaximumHeight(35);
   QFont lcFont = QFont("Monospace");
   lcFont.setPointSize(10);
   setFont(lcFont);

   setFrameShadow(Raised);
   setLineWidth(2);
   setSmallDecimalPoint(true);
   setDigitCount(6);
   setSegmentStyle(Flat);
   display(0);

   setFocusPolicy(Qt::ClickFocus);
   setEnabled(true);
   installEventFilter(this);
   show();
}

///////////////////////////////////////////////////////////////////////////////
////                      EVENT PROPAGATION
/** ////////////////////////////////////////////////////////////////////////////
////         Call base class implementation of wheelEvent().
//////////////////////////////////////////////////////////////////////////////
*/
void LcdDisplay::wheelEvent(QWheelEvent *ev) {
   //   if (ali + 1 < al.length())
   //      ali++;
   //   else
   //      ali=0;
   //   mlable->setAlignment(al[ali]);
   QLCDNumber::wheelEvent(ev);
}
void LcdDisplay::keyPressEvent(QKeyEvent *ev) {
   QStringList alNum, kMult;

   /** Set lcd value */
   if ((ev->key() == Qt::Key_Enter) ||
       (ev->key() == Qt::Key_Return)) {

   }

   for (int i = 0; i <= 9; i++)
      alNum << QString::number(i);

   kMult << "u" << "m" << "k";

   if (alNum.contains(ev->text())) {
      /**
       * Alpha numeric key pressed ( 0..9)
       */
      FuGen * fu = FuGen::getObjectPtr();
      this->display(QString::number( this->value() * 10 + ev->text().toInt()));
   }
   else {
      if (kMult.contains(ev->text())) {
         /**
          * Multiplier key (kilo, milli) pressed
          */

      }

   }

   QLCDNumber::keyPressEvent(ev);
}
bool LcdDisplay::eventFilter(QObject *obj, QEvent *ev) {
   if ((ev->type() == QEvent::Show) ||
       (ev->type() == QEvent::Hide)) {
      return QObject::eventFilter(obj, ev);
   }

   if ((ev->type() == QEvent::MouseButtonPress)) {
      Q_INFO << "objName:" << objectName()
            << "accessName:" << accessibleName();
   }

   /** Check the event type */
   if (ev->type() == QEvent::KeyPress) {
      QKeyEvent *kev = static_cast<QKeyEvent *>( ev );

      /** Return and print out error msg if static cast returns 0 */
      if (! kev) {
         Q_INFO << "ev->type() == QEvent::KeyPress but QKeyEvent cast failed";
         return QObject::eventFilter(obj, ev);
      }
      else {
         emit keyPressEvent(kev);
      }
      return true;
   }
   return QObject::eventFilter(obj, ev);
}
void LcdDisplay::focusInEvent(QFocusEvent *ev) {
   ev->accept();
   setSelected(Lcd::type_select);
}
void LcdDisplay::focusOutEvent(QFocusEvent *ev) {
   ev->accept();

//   if (this->isVisible())
      setSelected(Lcd::type_none);
}
void LcdDisplay::setSelected(Lcd::type_selections selectType) {
   if (selectType == Lcd::type_select) {
      setStyleSheet("QLCDNumber { "
                    "   background-color: rgb(167, 167, 167); "
                    "   border: 1px solid green; }"
                    "QLabel { "
                    "   font: bold; }");
   }

   if (selectType == Lcd::type_altered) {
      setStyleSheet("QLCDNumber { "
                    "   background-color: rgb(167, 167, 167); "
                    "   border: 2px solid green; }"
                    "QLabel { "
                    "   font: bold; }");
   }

   if (selectType == Lcd::type_confirmed) {
      setStyleSheet("QLCDNumber { "
                    "   background-color: rgb(167, 167, 167); "
                    "   border: 3px solid green; }"
                    "QLabel { "
                    "   font: bold; }");
   }

   if (selectType == Lcd::type_none) {
      setStyleSheet("QLCDNumber { "
                    "   background-color: rgb(167, 167, 167); }");
   }

   setStyle(QApplication::style());
   mSelected = selectType;
}
bool LcdDisplay::isSelected() const {
   if(! (mSelected == Lcd::type_none))
      return true;
   else
      return false;
}
