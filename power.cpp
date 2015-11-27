#include <QtMath>
#include <QTime>
#include <QTimer>
#include <QInputDialog>
#include <QDebug>

#include "mqtimer.h"
#include "globals.h"
#include "power.h"
#include "ui_power.h"
#include "visareg.h"

#define WHEELEVENT_OBJECT_NAME
#undef WHEELEVENT_OBJECT_NAME

class VisaReg;

/* ======================================================================== */
/*                     class constructor                                    */
/* ======================================================================== */
Power::Power(QWidget *parent) :
   QWidget(parent),
   ui(new Ui::Power) {
   ui->setupUi(this);

   timCycl = new QTimer();
   timCycl->setInterval( INTERVAL_LCD * 1e3 );
   timCycl->start();

   focGuard       = new MQTimer_t();
   focGuard->tim  = new QTimer();
   focGuard->tim->setInterval(250);
   focGuard->reload( 10 );
   focGuard->setEnabled();
   connect( focGuard->tim, SIGNAL(timeout()),
            this,          SLOT(onfocGuard()));

   connect( timCycl,       SIGNAL(timeout()),
            this,          SLOT(onCyclic()));
   connect( ui->btnEnNeg,  SIGNAL(clicked()),
            this,          SLOT(onBtnEnNegClicked()));
   connect( ui->btnEnPos,  SIGNAL(clicked()),
            this,          SLOT(onBtnEnPosClicked()));
   connect( qApp,          SIGNAL(focusChanged(QWidget*,QWidget*)),
            this,          SLOT(onAppFocusChanged(QWidget*,QWidget*)));
   supply = new Power::supply_t();

   vr       = VisaReg::getObjectPtr();
   ioedit   = IOEdit::getObjectPtr();
   visa     = Visa::getObjectPtr();

   vr->hwRegs.push_back( vr->regSetPwr );

   ui->lcdAutoCtr->setVisible(false);
   ui->btnLock->setChecked(true);

   /** Generate list of "focus ok" object names */
   QList<QWidget *> widLst = findChildren<QWidget *>();

   foreach (QWidget * wid, widLst)
      focusOkObj.append( wid->objectName() );
   focusOkObj.append( this->objectName() );

}

Power::~Power() {
   delete ui;
}

void Power::onBtnEnNegClicked() {
   fillTxReg();
}
void Power::onBtnEnPosClicked() {
}
void Power::onBtnLockClicked() {
}
void Power::onCyclic() {
/*   ui->lcdVoltPos->display( supply->Vplus.int_.set);
   ui->lcdAmpPos->display ( supply->Iplus.int_.set);
   ui->lcdVoltNeg->display( supply->Vmin.int_.set);
   ui->lcdAmpNeg->display ( supply->Imin.int_.set);
*/

   ui->lcdVoltPos->display( supply->Vplus.float_.set);
   ui->lcdAmpPos->display ( supply->Iplus.float_.set);
   ui->lcdVoltNeg->display( supply->Vmin.float_.set);
   ui->lcdAmpNeg->display ( supply->Imin.float_.set);

   /** If the power config has changed flag is set ... */
   if (visa->gs.powerCfgAltered)
      fillTxReg();
}
/** Setpoint changed */
void Power::wheelEvent ( QWheelEvent * event ) {
   /**
    * query current keyboard modifiers so we can gain the functionality of
    * "page scrolling". This means without CTRL modifier, normal step width is
    * used and with active CTRL key, the step width is multiplied by a factor
    * of 10.
    */
   Qt::KeyboardModifiers mods =
         QApplication::queryKeyboardModifiers();
   int div = 1;

   if (mods & Qt::ALT) {
      if (ui->lcdVoltPos->mode() == QLCDNumber::Hex) {
         ui->lcdVoltPos->setBinMode();
         ui->lcdAmpPos->setBinMode();
         ui->lcdVoltNeg->setBinMode();
         ui->lcdAmpNeg->setBinMode();

      }
      else if (ui->lcdVoltPos->mode() == QLCDNumber::Dec) {
         ui->lcdVoltPos->setHexMode();
         ui->lcdAmpPos->setHexMode();
         ui->lcdVoltNeg->setHexMode();
         ui->lcdAmpNeg->setHexMode();
      }
      else if (ui->lcdVoltPos->mode() == QLCDNumber::Bin) {
         ui->lcdVoltPos->setDecMode();
         ui->lcdAmpPos->setDecMode();
         ui->lcdVoltNeg->setDecMode();
         ui->lcdAmpNeg->setDecMode();
      }

      return;
   }
   (mods & Qt::CTRL ) ? div = 10 : div = 1;
   (mods & Qt::SHIFT) ? div *= 5 : div *= 1;

   uint16_t DIV = 100/div;
   /* ---------------------------------------------------------------- */
   /*         Get the name of object under the mouse pointer           */
   /* ---------------------------------------------------------------- */
   QWidget *widget = qApp->widgetAt(QCursor::pos());
   QString widName = widget->objectName();
#ifdef WHEELEVENT_OBJECT_NAME
   ioedit->putInfoLine( widName + " " + QString::number(mouseWheelCnt));
#endif
   /**
    * Check if the mouse pointer points to one of the lcd widgets from supply
    * window while this wheelEvent takes place.
    * Add current wheel counts to Power::'s "supply" structure. Event->delta()
    * can be negative or positive.
    */
   if (ui->btnLock->isChecked())
      return;

   ioedit->putInfoLine( QString::number( event->delta()/(DIV) ));

//   V_PER_DIG = ui->lineEdit->text().toInt();

   if (widName.contains( ui->lcdVoltPos->objectName() ))
      supply->VplusSet( event->delta()/(DIV) );
   else
      if (widName.contains( ui->lcdAmpPos->objectName() ))
         supply->IplusSet((int16_t) event->delta()/(DIV));
      else
         if (widName.contains( ui->lcdVoltNeg->objectName() ))
            supply->VminSet((int16_t) event->delta()/(DIV));
         else
            if (widName.contains( ui->lcdAmpNeg->objectName() ))
               supply->IminSet((int16_t) event->delta()/(DIV));
            else {
               Q_INFO << tr("Wheel event can't be mapped to an lcd widget");
               ioedit->putInfoLine(
                        tr("Wheel event can't be mapped to a lcd widget") );
            }

   visa->gs.powerCfgAltered = true;
}
void Power::fillTxReg() {
   /**
    * 00 00 00 46 70 67 00 05 0008 80 0C A0 03 2D 0C 81 0C A8 0001 62 01 00 05 72 00 00 00 00 00 00 02 BE 30 03 00 47 E0
    * 05 0008 80 00 00 00 0A 00 00 0F FF 0001 62 01 00 05 72 00 00 00 00 00 00 02 BE 15 00 00 47 E0
    * 01 0008 80 f9 d0 00 00 00 00 00 00
    * 00 00 00 46 70 67 00
    * 04 00 01 62 01 00 05 72 00 00 00 00 00 00 02 BE 15 00 00 47 E0 (11ms)
    *
    * 1V ... 10V
    * 0x0142 0x0286 0x03C9 0x050C 0x0650 0x0793 0x08D6 0x0A1A 0x0B5D 0x0CA0
    *  322    646     969   1292   1616   1939   2262   2586   2909   3232
    */
   vr->regSetPwr->h8087_pows.Vplus_set = supply->Vplus.int_.set;
   vr->regSetPwr->h8087_pows.ILim_plus = supply->Iplus.int_.set;
   vr->regSetPwr->h8087_pows.Vminus_set = supply->Vmin.int_.set;
   vr->regSetPwr->h8087_pows.ILim_minus = supply->Imin.int_.set;

   QByteArray ret = vr->writeToReg(vr->hwRegs, VisaReg::RetFormBIN);
   visa->gs.powerCfgAltered = false;
   ioedit->putTxData( ret );

}
void Power::onAppFocusChanged(QWidget *old,
                              QWidget *now) {
   if (old == 0 && isAncestorOf(now) == true)
      qDebug("Window not minimized anymore");
   else if (isAncestorOf(old) == true && now == 0)
      qDebug("Window minimized !!!");
}
/**
 * Primary used as onFocusLost() event. After lose focus and a short delay
 * counter, the Power:: ui gots locked to avoid unintended changes of output
 * voltage and currents
 */
void Power::onfocGuard() {
   QWidget *wid; QString widNam; int ctr;

   wid = qApp->widgetAt(QCursor::pos());
   (wid != 0x00) ? widNam = wid->objectName() : "NULL";

   /**
    * If the widget name under the current cursor position isn't found in
    * in the list AND if the lock Power:: ui is not already locked, ...
    */
   if (! (focusOkObj.contains(widNam) || ui->btnLock->isChecked()) ) {
      /** If roundCheck returns zero, countdown is over, lock */
      if (! (ctr = focGuard->roundCheck()))
         ui->btnLock->setChecked( true );
      else
         ui->lcdAutoCtr->display( ctr );

      ui->lcdAutoCtr->setVisible(true);
   }
   else {
      focGuard->reload( 10 );
      ui->lcdAutoCtr->setVisible(false);
      ui->lcdAutoCtr->display( 10 );
   }
}


