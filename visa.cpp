#include <QSettings>
#include <QDebug>
#include <QtCore/QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>
#include <QTableView>
#include <QList>
#include <QAction>

#include "visa.h"
#include "ui_visa.h"

#include "driver.h"
#include "ioedit.h"
#include "globals.h"
#include "visareg.h"
#include "register.h"
#include "calc.h"
#include "dvm.h"
#include "filebackup.h"
#include "power.h"
#include "fugen.h"

#define hardware_xxx_dat_rew QString("/opt/Visatronic/visascope-3.1/share/" \
   "hardware_values/rewSort_hardware_137000.dat")
#define hardware_xxx_dat QString("/opt/Visatronic/visascope-3.1/share/" \
   "hardware_values/hardware_137000.dat")

#define GEOM   tr("/widgetGeometry")
#define STAT   tr("/windowState")

const int Visa::MAX_CONSOLE_CHARS = 8000;

QTableWidget * Visa::pTw = 0;

/**
 * Initialize structs that holds visa scope sequences,
 * html codes, visascope info
 */
const Visa::visaSeqences Visa::vseq =  {
   "00000046706700",                                           //< STARTSEQ
   "040001620100057200000000000002be15000047e0",               //< STDBY
   "04000162",                                                 //< STDBYSML
   "040001620100057200000000000002be15020047e0",               //< STDBY_POWERON
   "03000162500002be15020047e0",                               //< STDBY_SCOPE Fenster offen aber kein trigger
   "0200026000d81000ec",                                       //< STDBY_SCOPE Fenster offen mit auto trigger
   "060009700800f205074cf20520000e500000000001c0001770000000000000086201470000ea000e3d000162410002be15020047e0",   //< SCOPEON
   "0500018a010001620100057200000000000002be15000047e0",       //< VDCON
   "0500018a000001620100057200000000000002be15000047e0",       //< VDCOFF
   "0500018b420001620100057200000000000002be15000047e0",       //< VADCON
   "0500018b000001620100057200000000000002be15000047e0",       //< VADCOFF
   "05000f3004830a657fff031100369d030000000001620100057200000000000002be15000047e0",   //< FUGENON
   "05000f30000007ff00000000000000000000000001620100057200000000000002be15000047e0",   //< FUGENOFF
   "05000880065800B400000C830001620100057200000000000002BE30020047E0",  //< POSPOWEREN
};
const Visa::HTMLcode Visa::html = {
   "<font color=\"Red\"> %1 </font>",
   "<font color=\"Gray\">",
   "<font color=\"Blue\">",
   "<b style=\"color: #000000; background-color: #c0c0c0\">@@@REPLACE</b>",
   //    "</font><br><",
   "</font><br>",
};
Visa::infoFromVisascope Visa::visaInfo = {
   "uc fw ???",
   "fpga fw ???",
   "hw rev ???",
   "Vcom ???",
   TESTBYTES,
   -2,
   0,
   -1.33,
   -1,
   false,
};

Visa* Visa::inst = 0;
Reg_bebf* Visa::regLedVlogic = 0;

/* ======================================================================== */
/*                            NOTES                                         */
/* ======================================================================== */
/**
 * Bei der original Software läuft ein 10..12ms Heartbeat im Standby Betrieb
 * (nur Serverfenster).
 * Bei jedem heartbeat wird ein kurzer stream übertragen indem mit der Endung
 * ... 00 47 E0 die unteren 71 (0x47 = 71) angefordert werden.
 *
 * TxStream (standby)
 * ------------------
 * 00000046706700  04    Startseq, 4 Blöcke
 * 0001 62 01            ScopeX:  trig_enb, free_run, autostart, trigger, start 0
 * 0005 72 0000000000    Ch1,Ch2: Gnd-position, filter aus, DC kopplung
 * 0002 BE 1500          0x15 LED (freq, duty, color) 0x00 Vlogic
 * 00 47 E0              Kennung für lese anforderung für 0x00...0x46
 *                       (general, scope, pws, dvms, fu-gen)
 *
 * ======================================================================== */


/* ======================================================================== */
/*                     class constructor                                    */
/* ======================================================================== */
Visa::Visa(QWidget *parent) :
   QMainWindow(parent),
   ui(new Ui::Visa) {

   ui->setupUi(this);
   /** Initial setup */
   gs.hw_xxx_dat_loaded = false;
   gs.calibrated        = false;
   newParent = parent;

   QSETTINGS;
   le8a8bForm = Visa::form_bin;

   /** Create instances */
   driver   = Driver::getInstance();
   calc     = Calc::getInstance();

   ioeditL  = IOEdit::getInstance(this, location_Left);
   ioeditR  = IOEdit::getInstance(this, location_Right);
//   ioeditL = ui->ioeditL;
//   ioeditR = ui->ioeditR;


   visareg  = new VisaReg(this);

   fugen    = 0x00;
   pwr      = 0x00;
   dvmDc    = 0x00;
   dvmAcDc  = 0x00;

   /**
    * Heartbeat timer - after port connect and calibration, this timer repeatly
    * requests the FPGA for the default register 0x00..0x47
    */
   hbeat       = new MQTimer_t();
   hbeat->tim  = new QTimer();
   hbeat->tim->setInterval(int( T_HEARTBEAT * 1e3 ));    //< 10ms heartbeat
   /**
    * Init the reload register for time division
    */
   hbeat->reload(HEARTBEAT_DIVIDER);

#ifdef NEW_ERA_CTR
   //< Timer instance for serial mode "sniffer"
   sniffTim       = new MQTimer();
   sniffTim->tim  = new QTimer();
   sniffTim->tim->setInterval(int(T_SNIFFER * 1e3));

   //< Init the reload register for time division
   sniffTim->reload(0);
#endif


//   /** Embedd the ioedit's instance widget into visa window */
   ui->splIOEd->addWidget( this->ioeditL );
   ui->splIOEd->addWidget( this->ioeditR );



   /**
    * Custom / User sequence timer. Periodically repeats the transmission
    * of an user / custom entered seq (ui lineedit)
    */
   timUserSeq  = new QTimer();

   /** Create the initial connections for the ui action objects */
   initActionsConnections();

#define QFOLDINGSTART {
   connect( ui->btnLoadSin,   SIGNAL(clicked()),
            this,             SLOT(  onBtnLoadSinClicked()  ));

   connect( hbeat->tim,       SIGNAL(timeout()),
            this,             SLOT(  onTimHbeatTimeout()  ));

   //   connect( timRefresh,       SIGNAL(timeout()),
   //            this,             SLOT(  onTimRefreshTimeout()  ));

   connect( ui->btnResCharCnts, SIGNAL(clicked()),
            driver,           SLOT(  clearRxBuffReqByteCnt()  ));

   connect( ioeditL,           SIGNAL(syncedReqBuffWasted()),
            driver,           SLOT(  flushSyncedReqBuff()  ));

   connect( ioeditL,           SIGNAL(syncedBuffWasted()),
            driver,           SLOT(  flushSyncedBuff()  ));

   connect( driver->serial,   SIGNAL(baudRateChanged(
                                        qint32,QSerialPort::Directions)),
            this,             SLOT(onSerialBaudChanged(
                                      qint32,QSerialPort::Directions)));

   connect(ui->pbDeleteCurrIdx,  SIGNAL(clicked()),
           this,                 SLOT(  onBtnDeleteCurrIdxClicked()  ));

   connect( visareg,          SIGNAL(calibInfoAvailable()),
            calc,             SLOT(  onCalibInfoAvailable()  ));

   connect( ui->btnOutputsOnOff, SIGNAL(clicked()),
            this,                 SLOT(onBtnOutputsOnOffClicked()));

   connect( driver,           SIGNAL(portConnected()),
            this,             SLOT(onDriverPortConnected()));

   connect( driver,           SIGNAL(portDisconnected()),
            this,             SLOT(onDriverPortDisconnected()));
   connect( ui->btnS8a8b,     SIGNAL(clicked()),
            this,             SLOT(onBtn8a8bClicked()));
   connect( ui->le8a8b,       SIGNAL(textChanged(QString)),
            this,             SLOT(onLe8a8bChanged(QString)));
   connect( ui->le8a8b,       SIGNAL(returnPressed()),
            this,             SLOT(onBtn8a8bClicked()));

#define QFOLDINGEND }
   /** should be called once to configure private format variables */
   footerFormatInit();
   //   timRefresh->start();
   refreshGS();

#ifdef AUTORUN
   QTimer::singleShot(100, Qt::PreciseTimer, this, SLOT(autoRunTimerSLOT()));
#endif

   /** RINGBUFFER */
   gs.blockOnReadyRead = false;
   /** RINGBUFFER END*/

   /** Start program heartbeat */
   hbeat->tim->start();


   pTw = ui->tableWidget;


   this->installEventFilter( this );
}
Visa::~Visa() {
   QSETTINGS;
   config.setValue(objectName() + GEOM, saveGeometry());
   config.setValue(objectName() + STAT, saveState());

   delete ui;
}

void Visa::autoRunTimerSLOT( ) {
   this->show();
   //   ui->actionConnect->setChecked(true);
   ui->actionConnect->triggered(true);
   //   ui->actionOpenDvm->setChecked(true);
   ui->actionOpenDvm->trigger();

}
void Visa::onActionRegistersOverview() {
   regFrm = new Register(this);
   regFrm->show();
}
void Visa::initActionsConnections() {
   ui->actionConnect->setEnabled(true);
   ui->actionDisconnect->setEnabled(false);
   ui->actionQuit->setEnabled(true);
   ui->actionConfigure->setEnabled(true);
   ui->actionSendSine->setEnabled(true);
   ui->actionRegisters->setEnabled(true);
   ui->actionSaveConfig->setEnabled(true);
   ui->actionHideFramehead->setEnabled(true);
   ui->actionNewline->setEnabled(true);
   ui->actionHideFramehead->setChecked(false);
   ui->actionNewline->setChecked(true);
   ui->actionOpenDvm->setEnabled(true);

   //   ui->actionErrorHandling->setChecked(true);
   ui->actionErrorHandling->setEnabled(true);
   ui->actionModeSniffer->setChecked(false);

   //<<<<<<<<<<<<<<<<     Driver connections    >>>>>>>>>>>>>>>>>>>>>>>>//
   connect( ui->actionConnect,      SIGNAL(triggered()),
            driver,                 SLOT(  openSerialPort()  ));
   connect( ui->actionDisconnect,   SIGNAL(triggered()),
            driver,                 SLOT(  closeSerialPort()  ));
   connect( ui->actionConfigure,    SIGNAL(triggered()),
            driver,                 SLOT(  onOpenPortDialog()  ));

   //<<<<<<<<<<<<<<     ui action buttons     >>>>>>>>>>>>>>>>>>>>>>>>>//
   connect( ui->actionTestWrite,    SIGNAL(triggered()),
            visareg,                SLOT(  testWriteReg()  ));
   connect( ui->actionTestRead,     SIGNAL(triggered()),
            visareg,                SLOT(  testReadReg()  ));

   connect( ui->actionOpenDvm,      SIGNAL(triggered()),
            this,                   SLOT(  onOpenDvmClicked()  ));
   connect( ui->actionOpenDvm,      SIGNAL(triggered()),
            driver,                 SLOT(  openSerialPort()  ));

   connect( ui->actionRereadDat,    SIGNAL(triggered()),
            calc,                   SLOT(  loadHardwareDat()  ));
   connect( ui->actReadTinyCalib,   SIGNAL(triggered()),
            visareg,                SLOT( reqCalibDataFromAtiny()  ));

   //   connect( ui->actionConsoleColor, SIGNAL(triggered()),
   //            ioedit,                 SLOT(  setColorBack()  ));
   //   connect( ui->actionTextColor,    SIGNAL(triggered()),
   //            ioedit,                 SLOT(  setColorText()  ));

   connect( ui->actionQuit,         SIGNAL(triggered()),
            this,                   SLOT(  closeChildsAlso()  ));
   connect( ui->actionClear,        SIGNAL(triggered()),
            ioeditL,                 SLOT(  clear()  ));
   connect( ui->actionClear,        SIGNAL(triggered()),
            ioeditR,                 SLOT(  clear()  ));
   connect( ui->actionAbout,        SIGNAL(triggered()),
            this,                   SLOT(  about()  ));
//   connect( ui->actionSendSine,     SIGNAL(triggered()),
//            this,                   SLOT(  SinToRam()  ));
   connect( ui->actionRegisters,    SIGNAL(triggered()),
            this,                   SLOT(  onActionRegistersOverview()  ));
   connect( ui->actionSaveConfig,   SIGNAL(triggered()),
            ioeditL,                 SLOT(  savePersistanceSettings()  ));
   connect( ui->actionAboutQt,      SIGNAL(triggered()),
            qApp,                   SLOT(  aboutQt()  ));
   connect( ui->actionHideFramehead,   SIGNAL(toggled(bool)),
            driver,                    SLOT(  setHideFrameheader(bool)  ));
   connect( ui->actionNewline,      SIGNAL(toggled(bool)),
            driver,                 SLOT(  setAppendNewlineChar(bool)  ));
   connect( ui->actionErrorHandling,   SIGNAL(triggered()),
            this,                      SLOT(refreshGS()));

   //<<<<<<<<<<<<<<<<<<<<     Parser     >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
   connect( ui->actionParserEnable, SIGNAL(triggered(bool)),
            ioeditL,                 SLOT(  setParserActive(bool)  ));
   connect( ui->actionFull_Ruler,   SIGNAL(triggered(bool)),
            ioeditL,                 SLOT(  setParserFullRuler(bool)  ));
   connect( ui->actionNo_Ruler,     SIGNAL(triggered(bool)),
            ioeditL,                 SLOT(  setParserNoRuler(bool)  ));

   connect( ui->actionChildsToDbg,  SIGNAL(triggered()),
            this,                   SLOT(childsToDbg()));

   connect( ui->actionModeSniffer,  SIGNAL(triggered(bool)),
            this,                   SLOT(setSnifferTimEnabled(bool)));

   connect( ui->actionSnapShot,     SIGNAL(triggered()),
            this,                   SLOT(onActSnapShotTriggered()));

   connect( ui->actionPower,        SIGNAL(triggered(bool)),
            this,                   SLOT(onActPowerTriggered(bool)));

   connect( ui->actionFuGen,        SIGNAL(triggered(bool)),
            this,                   SLOT(onActFuGenTriggered(bool)));
   connect( ui->actionSaveAllGeometrys,   SIGNAL(triggered()),
            this,                         SLOT(saveAllGeometrys()));
   connect( ui->actionRestoreAllGeometrys,SIGNAL(triggered()),
            this,                         SLOT(restoreAllGeometrys()));

   connect( this,                   SIGNAL(uiCmdLineQuery(QString)),
            this,                   SLOT(onUiCmdLineQuery(QString)));


   connect( ui->actInitialRegStat,  SIGNAL(triggered()),
            visareg,                SLOT(reqInitialRegState()));

   ui->actionParserEnable->setChecked(false);
   ui->actionFull_Ruler->setChecked(false);
   ui->actionNo_Ruler->setChecked(true);
   ui->actionParserEnable->trigger();
   ui->actionFull_Ruler->trigger();
   ui->actionNo_Ruler->trigger();

}
bool Visa::loadPersistance() {
   QSETTINGS;
   config.beginGroup("Tab_UI_settings");  {
      ui->tabCtrls->setCurrentIndex(config.value("tabCtrlsIdx").toInt());

      QStringList list = config.value("StringList/repeatCustomSeq").toString().split(",");
      cbModelRepeatSeq = new QStringListModel(list);
      ui->cbRepeatCustSeq->setModel(cbModelRepeatSeq);

      config.endGroup();
   }
   return true;
}
bool Visa::savePersistance() {
   /**
     * Save all user settings in a configuration file on the filesytem
     */
   QTime time = QTime::currentTime();
   QString timestr =
         QString::number(time.hour()) +
         QString::number(time.minute()) +
         QString::number(time.second());
   //@@@1MDB//qDebug()() << "Writing config at" << time.toString() << "h";
   QFile::copy(CONFIG_PATH, QString(CONFIG_PATH) + "_" + timestr);
   QSETTINGS;

   config.beginGroup("Tab_UI_settings");  {
      config.setValue("tabCtrlsIdx", ui->tabCtrls->currentIndex());

      QStringList list = cbModelRepeatSeq->stringList();
      config.setValue("StringList/repeatCustomSeq", list.join(","));
      config.endGroup();
   }
   return true;
}
/* ======================================================================== */
/*                     timer slots                                          */
/* ======================================================================== */
void Visa::onTimHbeatTimeout() {
   /**
    * If calibration was successfull, periodic requests could be
    * transmitted
    */
   if ( driver->isPortOpen() &&
        gs.hw_xxx_dat_loaded &&
        gs.calibrated && ui->actionPeriodicRequest->isChecked()) {
      visareg->reqDefaultRegs();
   }

   /** Refresh rx byte counter label */
   ui->labRxCtr->setText(QString::number(
                            driver->rxBuffReq.getByteCnt()));

   /** Tick every second */
   if (! hbeat->roundCheck() &&
       ! ui->actionNoStandbyDbgOut->isChecked())
      qDebug() << QTime::currentTime();
}
void Visa::onTim1Timeout() {
   QString tx;
   tx = ui->cbRepeatCustSeq->currentText().toLatin1();
   driver->writeData(tx.toLatin1());
}
/* ======================================================================== */
/*                     (Button) slots                                       */
/* ======================================================================== */
void Visa::on_btnOutputsOnOff_clicked(bool checked) {
   Q_UNUSED(checked);
   QString st;
   st = ui->cbRepeatCustSeq->currentText();
   //   ioeditL->putInfoLines(st);

}
void Visa::onBtnLoadSinClicked() {
   ioeditL->putInfoLine(
            tr("SinToRam() functionality has moved"
               "to class FuGen:: at 27-10-2015!"));
}
void Visa::on_pbRepeatOnOff_clicked() {

   txStr = ui->cbRepeatCustSeq->currentText().toLatin1();
   driver->writeData(txStr.toLatin1());

   if (ui->pbRepeatOnOff->isChecked()) {
      connect(timUserSeq,
              SIGNAL(timeout()), this,
              SLOT(  onTim1Timeout()  ));

      timUserSeq->setInterval(ui->leRepeatPeri->text().toInt());
      timUserSeq->start();
   }
   else {
      timUserSeq->stop();
   }
}
void Visa::clearConsoleCaller() {
   emit clearConsole();
   ioeditL->clear();
   ioeditR->clear();
}
void Visa::onBtnOpenRegView() {
   regFrm = new Register();
   regFrm->show();
}
void Visa::onBtnRefreshRegMap() {
   /**
     * If this slot is executed and the current tabCtrls index is NOT "RAM",
     * deactivate the refresh register map timer
     */
   if (! QString(ui->tabCtrls->tabText(
                    ui->tabCtrls->currentIndex() )).contains("RAM")) {
      reqRefreshTim->stop();
      return;
   }
   /**
     * readVisaInfos only if button is down. This slot is also
     * used by the auto refreshRegister map timer functionality
     */
   if (ui->pbRefreshRegMap->isDown()) {
      /** Read Infos from uC and FPGA */
      //        readVisaInfos();
   }
   //   /** Read register contents of 0x00...0x27 */
   //       readReg_00_27();
   //   /** parse info to visaInfo struct */
   //       parseReg00_27();
   //    refreshRegOverview();

   //    if (! serial->isOpen())
   //        reqRefreshTim->stop();
}
void Visa::onRbxxxContentClicked() {
   /** If radiobutton dec|hex|bin clicked, refresh register overview */
   //    refreshRegOverview();
   bool ok;
   /** If radiobutton  rbAutoRefreshReg clicked, enable/disable auto refresh timer */
   if (ui->rbAutoRefreshReg->isChecked()) {
      int s = ui->leAutoRefPeri->text().toInt(&ok);
      if (!ok)
         s = 2000;
      reqRefreshTim->setInterval( s );
      reqRefreshTim->start();
   }
   else {
      reqRefreshTim->stop();
   }
}
void Visa::onBtnStopSamplingClicked() {
   //    scopeStopMode();
}
void Visa::onBtnDeleteCurrIdxClicked() {
   ui->cbRepeatCustSeq->removeItem(ui->cbRepeatCustSeq->currentIndex());
}
void Visa::onOpenDvmClicked() {
   QSETTINGS;
   /** Get DC dvm instance */
   dvmDc = Dvm::getInstance(dvmType_DC/*, this*/);
   try {
      if (dvmDc->isVisible()) {
         config.setValue("DvmDc/geometry", dvmDc->saveGeometry());
         dvmDc->hide();
      }
      else {
         dvmDc->restoreGeometry(
                  config.value("DvmDc/geometry"," ").toByteArray() );
         dvmDc->show();
      }
   } catch (...) {
      qDebug() << "actionOpenDvm";
   }

   /** Get ACDC dvm instance */
   dvmAcDc = Dvm::getInstance(dvmType_ACDC/*, this*/);
   try {
      if (dvmAcDc->isVisible()) {
         config.setValue("DvmAcDc/geometry", dvmAcDc->saveGeometry());
         dvmAcDc->hide();
      }
      else {
         dvmAcDc->restoreGeometry(
                  config.value("DvmAcDc/geometry"," ").toByteArray() );
         dvmAcDc->show();
      }
   } catch (...) {
      qDebug() << "actionOpenDvm";
   }
}
void Visa::onTransEditReturnPressed() {

}
void Visa::onBtnOutputsOnOffClicked() {

   Reg_8087 *regSetPwr     = Reg_8087::pObj(); Q_UNUSED(regSetPwr);
   Reg_bebf *regLedVlogic  = Reg_bebf::pObj();

   QVector<HwReg*> hwRegs;
   QVector<HwReg*>::iterator it;

   //   hwRegs.push_back( regSetPwr );
   hwRegs.push_back( regLedVlogic );
   for ( it = hwRegs.begin(); it != hwRegs.end(); *it++) {
      (*it)->AboutRegister();
   }
   for ( it = hwRegs.begin(); it != hwRegs.end(); *it++) {
      qout() << "Union size:" << (*it)->size();
   }


   regLedVlogic->hbebf_led.LED_Freq       = Reg_bebf::freq_1dot78_Hz;
   regLedVlogic->hbebf_led.LED_color      = Reg_bebf::highGrn_lowOff;
   regLedVlogic->hbebf_led.LED_duty       = Reg_bebf::duty_125;
   regLedVlogic->hbebf_led.Vlocic_lvl     = Reg_bebf::V_LEVEL_3V3;
   regLedVlogic->hbebf_led.Vlocic_enb     = Reg_bebf::Vlogic_5V_or_3V;

   /*
         regSetPwr->h8087_pows.Vplus_set = 0x0400;
         regSetPwr->h8087_pows.ILim_plus = 0x0200;
         regSetPwr->h8087_pows.Vminus_set = 0x0800;
         regSetPwr->h8087_pows.ILim_minus = 0x0200;

         regDvmRng->h8a8b_rng.M1_rng         = Reg_8a8b::Range20V;
         regDvmRng->h8a8b_rng.M2_rng         = Reg_8a8b::Range20V;
         regDvmRng->h8a8b_rng.M1_inpt        = Reg_8a8b::Input_single;
         regDvmRng->h8a8b_rng.M2_inpt        = Reg_8a8b::Input_single;

         sortForAddrs(hwRegs);
         QByteArray ret = assambleStream(hwRegs, false);
         QByteArray retRaw;
         retRaw.append( convert( ret ), ret.length()/2);

         qDebug() << "\t" << ret;
         qDebug() << "\t" << QString(ret).remove(" ");
         qDebug() << "\t" << retRaw.toHex();

      //   serial.write(ret);
      //   serial.waitForBytesWritten(1);
         serial.write(retRaw);
         serial.waitForBytesWritten(10);
      */
   QByteArray ret = visareg->writeToReg(hwRegs, VisaReg::RetFormBIN);

   //   QTime t;
   //   t.start();
   //   //      int inp = 20;

   //   while( t.elapsed() < 3000 );;

   //   regLedVlogic->hbebf_led.LED_color      = Reg_bebf::highRed_lowOff;
   //   regLedVlogic->hbebf_led.Vlocic_enb     = Reg_bebf::Vlogic_off;

   //   writeToReg(hwRegs);

}
void Visa::onActSnapShotTriggered() {
   //   qDebug() << ioeditL->getCurrentTextBlocks();
   FileBackup::toFilesys( ioeditL->getCurrentTextBlocks(), "test1");
}
void Visa::onBtn8a8bClicked() {
   bool ok = 0;
   Reg_8a8b *regDvmRngInp = Reg_8a8b::pObj();

   QVector<HwReg*> hwRegs;
   hwRegs.push_back( regDvmRngInp );

   QString ln = ui->le8a8b->text();
   quint16 val = ln.toInt(&ok, (int) le8a8bForm);

   if (!ok) {
      Q_INFO << tr("Failed!!!");
      return;
   }

   //   regDvmRngInp->h8a8b_rng.AD2_T = 1;
   //   regDvmRngInp->h8a8b_rng.AD2_on = 1;
   //   regDvmRngInp->h8a8b_rng.M1_inpt = 1;
   //   regDvmRngInp->h8a8b_rng.M1_rng = 0x03;
   //   regDvmRngInp->h8a8b_rng.M2_inpt = 1;
   //   regDvmRngInp->h8a8b_rng.M2_rng = 0x03;

   regDvmRngInp->h8a8b_rng.word = val;

   visareg->writeToReg(hwRegs);
}
void Visa::onLe8a8bChanged(QString str) {
   return;

   while (str.length() < 16) {
      str.prepend("0");
   }
   ui->le8a8b->setText( str );
}
void Visa::onActPowerTriggered(bool onoff) {
   QSETTINGS;

   (pwr == 0x00)
         ?  pwr = Power::getInstance( newParent )
         :  pwr = Power::getObjectPtr();

   if (! onoff)
      config.setValue(pwr->objectName() + GEOM, pwr->saveGeometry());
   else {
      try {
         pwr->restoreGeometry(config.value(pwr->objectName() + GEOM,"").toByteArray());
      }  catch (...) {

      }
   }

   pwr->setVisible(onoff);
}
void Visa::setSnifferTimEnabled(bool onoff) {
   sniffTim->setEnabled(onoff);
}
void Visa::onActFuGenTriggered(bool onoff) {
   QSETTINGS;

   (fugen == 0x00)
         ?  fugen = FuGen::getInstance( newParent )
         :  fugen = FuGen::getObjectPtr();

   if (! onoff)
      config.setValue(fugen->objectName() + GEOM, fugen->saveGeometry());
   else
      fugen->restoreGeometry(config.value(fugen->objectName() + GEOM,"")
                             .toByteArray());

   fugen->setVisible(onoff);
}
/* ======================================================================== */
/*                     Child Events                                         */
/* ======================================================================== */
void Visa::onChildWidgetDestroyed(QObject *died) {
   QProcess process;
   process.start("notify-send",
                 QStringList() << "-t3000"
                 << "Object destroyed" << died->objectName());

   Dvm   *ddvm  = static_cast<Dvm *>(died);
   Power *dpwr  = static_cast<Power *>(died);
   FuGen *dfgen = static_cast<FuGen *>(died);

   if (! (ddvm && dpwr && dfgen)) {
      QMessageBox::warning(this, "Object destroyed",
                           "No static cast possible to identify the died"
                           "object..." + died->objectName());
      return;
   }

   if (ddvm) {
      ui->actionOpenDvm->setChecked( false );
      return;
   }

   if (dpwr) {
      ui->actionPower->setChecked( false );
      return;
   }

   if (dfgen) {
      ui->actionFuGen->setChecked( false );
      return;
   }

}
void Visa::onUiCmdLineQuery(QString cmd) {
   QSETTINGS;
   if (cmd.contains("reqInitialRegState", Qt::CaseInsensitive)) {
      visareg->reqInitialRegState();
      config.setValue(objectName() + "/uiCmdLineQuery", cmd);
   }
}
/* ======================================================================== */
/*                     Serial port driver slots                             */
/* ======================================================================== */
void Visa::onSerialBaudChanged(qint32 baud,
                               QSerialPort::Directions dir) {
   Q_UNUSED(dir);

//   ui->statusBar->showMessage();

   QTextCursor cursor( ui->statusBar->getTeStat()->textCursor() );
   //   ui->teStateFooter->setCursorWidth(50);
   cursor.setPosition(35, QTextCursor::KeepAnchor);
   cursor.setCharFormat(formatLHS);
   cursor.insertText("Baudrate:  ");
   cursor.setCharFormat(formatRHS);
   cursor.insertText(QString::number( baud ));
}
void Visa::onErrorReceived(QString errStr) {
   QMessageBox::critical(this, tr("Error"), errStr);
   ui->statusBar->showMessage(tr("Open error"));
}
void Visa::onDriverPortConnected() {
   PortDialog::Settings p = driver->getSettings();

   ui->actionConnect->setEnabled(false);
   ui->actionDisconnect->setEnabled(true);
   ui->actionConfigure->setEnabled(false);
   ui->statusBar->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                              .arg(p.name)
                              .arg(p.stringBaudRate)
                              .arg(p.stringDataBits)
                              .arg(p.stringParity)
                              .arg(p.stringStopBits)
                              .arg(p.stringFlowControl));
   enableUiElements(true);
   footerUpdate( p );

   /** Load the hardware_xxx.dat file calibration values */
   calc->loadHardwareDat();

   /** Request hardware specific tolerance values from visascopes ATtiny */
   visareg->reqCalibDataFromAtiny();

   setVisaLED_VLogic(Visa::led_flash_gn, true, 5);

   //   QTimer::singleShot(25, Qt::CoarseTimer,
   //                      this, SLOT(AfterConnectedSettings()));
}
void Visa::onDriverPortDisconnected() {
   PortDialog::Settings p = driver->getSettings();

   ui->actionConnect->setEnabled(true);
   ui->actionDisconnect->setEnabled(false);
   ui->actionConfigure->setEnabled(true);
   ui->statusBar->showMessage(tr("Disconnected"));
   enableUiElements(false);

   footerUpdate( p );

   /** Get pointer of valied Dvm instances */
   QVector<Dvm *> dvms;
   dvms.append( Dvm::getObjectPtr(dvmType_DC) );
   dvms.append( Dvm::getObjectPtr(dvmType_ACDC) );

   /** Stop lcd refresh timers Dvm::refTim */
   for (int i=0; i<2; i++)
      if (dvms.at(i) != 0)
         dvms[i]->refTim->stop();

   //   timHbeat->stop();
}
void Visa::footerUpdate(PortDialog::Settings p) {

//   QTextCursor cursor( this->teStat->textCursor());
   QTextCursor cursor( ui->statusBar->getTeStat()->textCursor() );

   cursor.setPosition(0, QTextCursor::KeepAnchor);
   /*
     * Port Mode
     */
   QString sOpenMode;
   switch (driver->getOpenMode()) {
      case (QIODevice::NotOpen):
         sOpenMode = "NotOpen";
         break;

      case (QIODevice::ReadOnly):
         sOpenMode = "ReadOnly";
         break;

      case (QIODevice::WriteOnly):
         sOpenMode = "WriteOnly";
         break;

      case (0x0003):
         sOpenMode = "ReadWrite";
         break;

      case (QIODevice::Append):
         sOpenMode = "Append";
         break;

      case (QIODevice::Truncate):
         sOpenMode = "Truncate";
         break;

      case (QIODevice::Text):
         sOpenMode = "Text";
         break;

      case (QIODevice::Unbuffered):
         sOpenMode = "Unbuffered";
   }
   /**< port name string */
   cursor.setCharFormat(formatLHS);
   cursor.insertText("Port:  ");
   cursor.setCharFormat(formatRHS);
   cursor.insertText( p.name );
   /**< Port open mode */
   cursor.setPosition(18, QTextCursor::KeepAnchor);
   cursor.setCharFormat(formatLHS);
   cursor.insertText("   Open Mode:  ");

   if (sOpenMode.contains("NotOpen")) {
      cursor.setCharFormat(formatRHS_red);
   } else {
      cursor.setCharFormat(formatRHS_gn);
   }

   cursor.insertText(sOpenMode);

   cursor.setPosition(55, QTextCursor::KeepAnchor);
   cursor.setCharFormat(formatLHS);
   cursor.insertText("   Baudrate:  ");
   cursor.setCharFormat(formatRHS);
   cursor.insertText(QString::number( p.baudRate ));
}
/* ======================================================================== */
/*                     pretty little helpers                                */
/* ======================================================================== */
/**
 * Convert a char sequence that encodes hexadecimal bytes to a QByteArray
 * Example call:
 * QByteArray locSTARTSEQ;
 * locSTARTSEQ.append((char *) convert(vseq.STARTSEQ), vseq.STARTSEQ.length()/2);
 * or:
 * seqInfoArr.append( convert(seqInfo), seqInfo.length()/2 );
 */
char *Visa::convert(QString in) {
   uint16_t i;
   QString str;
   bool ok;
   char * raw;

   int le = in.length()/2;
   raw = (char *) calloc(le, 1);

   if ((in.remove(" ")).length() < in.length()) {
      Q_INFO << tr("Whitespace truncated!\nRemember to truncate whitespaces of"
                   " attribute used to call (..).length/2!!!\n\n");
   }

   /**
    * We expect that in is a String that encodes a sequence of hex-coded
    * symbols "0004f408" --> 0x00 0x04 0xf4 0x08
    */
   if (in.length() % 2) {
      Q_INFO << "in.length() mod(2) > 0?!";
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
/**
 * Public slot that calls the destructor
 */
void Visa::quit() {
   saveAllGeometrys();
   delete this;
}
void Visa::closeChildsAlso() {
   QSETTINGS;

   if ( dvmDc ) {
      if ( dvmDc->dvmHasType == dvmType_DC ) {
         QString cfgType = tr("DvmDc");
         config.setValue(cfgType + GEOM, dvmDc->saveGeometry());
         dvmDc->close();
      }
   }

   if ( dvmAcDc ) {
      if ( dvmAcDc->dvmHasType == dvmType_ACDC ) {
         QString cfgType = tr("DvmAcDc");
         config.setValue(cfgType + GEOM, dvmAcDc->saveGeometry());
         dvmAcDc->close();
      }
   }

   if ( pwr ) {
      config.setValue(pwr->objectName() + GEOM,
                      pwr->saveGeometry());
      pwr->close();
   }

   if ( fugen ) {
      config.setValue(fugen->objectName() + GEOM,
                      fugen->saveGeometry());
      fugen->close();
   }

   //   this->disconnect();
   this->close();
   //   QVector<QAction *> mainwinActs = parentWidget()->parentWidget()
   //         ->actions().toVector();

   //   foreach (QAction *act, mainwinActs) {
   //      if (act->objectName().contains(tr("actionVisa")))
   //         act->setChecked(false);
   //   }

}
void Visa::about() {
   QMessageBox::
         about(this, tr("About visa-commander"),
               tr("The <b>visa-commander</b> is used and developed as command"
                  "interface to a superb I/O hardware called <b> VisaScope </b>."
                  "This qt software is the main part of my effort to gain the"
                  "functionality of a frequency response recorder for electronic"
                  "stimulated systems"
                  "Author: MDB"));
}
void Visa::footerFormatInit() {

   return;
   /** Check if driverSettings parameter is != NULL else exit.
    */
   //   if (driverSettings == NULL)   return;


   //< Refresh state footer text
   /// Move cursor to end of current text instead of jusing methode .append (nonewline)
   //   QTextCursor cursor( ui->teStateFooter->textCursor());
   /*ui->teStateFooter-*/
   this->teStat->setText("________________________________________"
                         "________________________________________");
   /// Save default format setting
   formatDefault.fontWeight();
   formatDefault.foreground();
   /// Save LHS format setting
   formatLHS.setFontWeight(QFont::Normal);
   formatLHS.setForeground(QBrush(QColor(109, 112, 128)));
   /// Save RHS format setting
   formatRHS.setFontWeight(QFont::DemiBold);
   formatRHS.setForeground(QBrush(QColor(109, 112, 128)));
   /// Save RHS format settings if port is closed
   formatRHS_red.setFontWeight(QFont::DemiBold);
   formatRHS_red.setForeground(QBrush(QColor("red")));
   /// Save RHS format settings if port is opened
   formatRHS_gn.setFontWeight(QFont::DemiBold);
   formatRHS_gn.setForeground(QBrush(QColor("green")));
#define FOLDINGSTART {
   //   /*
   //     * Port Mode
   //     */
   //   QString sOpenMode;
   //   switch (driver->getOpenMode()) {
   //      case (QIODevice::NotOpen):
   //         sOpenMode = "NotOpen";
   //         break;

   //      case (QIODevice::ReadOnly):
   //         sOpenMode = "ReadOnly";
   //         break;

   //      case (QIODevice::WriteOnly):
   //         sOpenMode = "WriteOnly";
   //         break;

   //      case (0x0003):
   //         sOpenMode = "ReadWrite";
   //         break;

   //      case (QIODevice::Append):
   //         sOpenMode = "Append";
   //         break;

   //      case (QIODevice::Truncate):
   //         sOpenMode = "Truncate";
   //         break;

   //      case (QIODevice::Text):
   //         sOpenMode = "Text";
   //         break;

   //      case (QIODevice::Unbuffered):
   //         sOpenMode = "Unbuffered";
   //   }

   //   //    //@@@1MDB//qDebug()() << sOpenMode;
   //   /**< Clear and refresh */
   //   ui->teStateFooter->clear();
   //   /**< port name string */
   //   cursor.setCharFormat(formatLHS);
   //   cursor.insertText("Port:  ");
   //   cursor.setCharFormat(formatRHS);
   //   cursor.insertText( driverSettings.name );
   //   /**< Port open mode */
   //   cursor.setCharFormat(formatLHS);
   //   cursor.insertText("  Open Mode:  ");

   //   if (sOpenMode.contains("NotOpen")) {
   //      cursor.setCharFormat(formatRHS_red);
   //   } else {
   //      cursor.setCharFormat(formatRHS_gn);
   //   }

   //   cursor.insertText(sOpenMode);
   /**< Port baudrate */
   //   cursor.setCharFormat(formatLHS);
   //   cursor.insertText("  Baudrate:  ");
   //   cursor.setCharFormat(formatRHS);
   //   cursor.insertText(QString::number( driverSettings.baudRate ));
#define FOLDINGEND }
}
void Visa::footerRefreshVisa() {

   return;

   ///< Refresh info from visa uc - footer text
   ///< Move cursor to end of current text instead of jusing methode .append (nonewline)
   QTextCursor cursor;

//   cursor = ui->teVisaInfo->textCursor();
   cursor = this->teStat->textCursor();
   QTextCharFormat formatDefault,  //< Object to store default format settings
         formatRHS,      //< Object to store format settings of LHS strings
         formatTimeout;  //< Object to store format settings of Timeout

   ///< Save default format setting
   formatDefault.fontWeight();
   formatDefault.foreground();
   ///< Save RHS format setting
   formatRHS.setFontWeight(QFont::DemiBold);
   formatRHS.setForeground(QBrush(QColor(109, 112, 128)));
   ///< Save format settings for timeout indicator
   formatTimeout.setFontWeight(QFont::DemiBold);
   formatTimeout.setForeground(QBrush(QColor(220, 220, 220)));
   formatTimeout.setBackground(QBrush(QColor(240, 0, 0)));

   /**< Clear and refresh */
//   ui->teVisaInfo->clear();
   this->teStat->clear();
   /**< uc firmware */
   cursor.setCharFormat(formatDefault);
   cursor.insertText("uP FW: ");
   cursor.setCharFormat(formatRHS);
   cursor.insertText(driverSettings.name);
   /**< fpga firmware */
   cursor.setCharFormat(formatDefault);
   cursor.insertText("  FPGA FW: ");
   cursor.setCharFormat(formatRHS);
   cursor.insertText( visaInfo.FPGA_FW );
   /**< HW revision */
   cursor.setCharFormat(formatDefault);
   cursor.insertText("  HW rev.: ");
   cursor.setCharFormat(formatRHS);
   cursor.insertText( visaInfo.Hw_rev );
   /**< Serial number */
   cursor.setCharFormat(formatDefault);
   cursor.insertText("  Serial #: ");
   cursor.setCharFormat(formatRHS);
   cursor.insertText( QString::number(visaInfo.SerialNo ));
   /**< VCom */
   cursor.setCharFormat(formatDefault);
   cursor.insertText("  VCom: ");
   cursor.setCharFormat(formatRHS);
   cursor.insertText( visaInfo.Vcom );
   /**< Baudrate */
   cursor.setCharFormat(formatDefault);
   cursor.insertText("  Baud: ");
   cursor.setCharFormat(formatRHS);
   cursor.insertText(QString::number( visaInfo.BaudrateCode ));
   /**< Bias Voltage */
   cursor.setCharFormat(formatDefault);
   cursor.insertText("  V-Bias: ");
   cursor.setCharFormat(formatRHS);
   cursor.insertText(QString::number( visaInfo.BiasVoltage ));
   //    if (! gs.activeRxTimeout) {
   //        cursor.setCharFormat(formatDefault);
   //        cursor.insertText("");
   //        cursor.setCharFormat(formatRHS);
   //        cursor.insertText("");
   //    }
   //    else {
   //        cursor.setCharFormat(formatTimeout);
   //        cursor.insertText("TIMEOUT ");
   //        cursor.setCharFormat(formatTimeout);
   //        cursor.insertText(QString::number(gs.RxTimeoutAmount[1]) % " / " %
   //                QString::number(gs.RxTimeoutAmount[0]));
   //    }

   //    ui->lcdScopeState->display(visa.visaInfo.getScope_State());
   //    ui->labCharCnt->setText(QString::number(getCharCnt()));
}
void Visa::CRASHME() {
   qWarning() << "byebye";
}
void Visa::writeRam(const QByteArray *datIn, quint8 target) {
   QByteArray tx;
   QString tx2;
   QByteArray vseqSTARTSEQ = "00000046706700";

   tx.append( convert(vseqSTARTSEQ), vseqSTARTSEQ.length()/2 );
   tx2 = (QString("01 %1 %2").arg(datIn->length(), 4, 16, QLatin1Char('0'))
          .arg(target, 2, 16, QLatin1Char('0')));
   tx.append( convert(tx2.remove(' ')), tx2.length()/2 );
   tx.append( convert(*datIn), datIn->length()/2 );

   driver->writeData(tx);

}
void Visa::enableUiElements(bool uistate) {
   ui->btnOutputsOnOff->setEnabled( uistate );
   ui->rbFuGenOnOff->setEnabled( uistate );
   ui->slFuAmpl->setEnabled( uistate );
   ui->rbFuGenOnOff->setEnabled( uistate );
   ui->leOffset->setEnabled( uistate );
   ui->rbVLogicOnOff->setEnabled( uistate );
   ui->rbVposOnOff->setEnabled( uistate );
   ui->leAmpl->setEnabled( uistate );
   ui->rbVnegOnOff->setEnabled( uistate );
   ui->btnOutputsOnOff->setEnabled( uistate );
   ui->cbWaveform->setEnabled( uistate );
   ui->leFreq->setEnabled( uistate );
   ui->slFuFreq->setEnabled( uistate );
   ui->slFuOffs->setEnabled( uistate );

}
void Visa::refreshGS() {
   gs.errorHandleractive = ui->actionErrorHandling->isChecked();
}
void Visa::childsToDbg() {
   Q_INFO << this->children();
}
void Visa::setVisaLED_VLogic(Visa::LEDs cLed, bool VlogicEn,
                             quint8 volt, VisaReg* vr) {
   if (regLedVlogic == 0)
      regLedVlogic = Reg_bebf::pObj();

   QVector<HwReg*> hwRegs;
   QVector<HwReg*>::iterator it;

   hwRegs.push_back( regLedVlogic );
   for ( it = hwRegs.begin(); it != hwRegs.end(); *it++)
      (*it)->AboutRegister();

   if (cLed == led_gn)
      regLedVlogic->hbebf_led.LED_color = Reg_bebf::highGrn_lowOff;
   if (cLed == led_flash_gn)
      regLedVlogic->hbebf_led.LED_color = Reg_bebf::highGrn_lowOff;

   if (cLed == led_rd)
      regLedVlogic->hbebf_led.LED_color = Reg_bebf::highRed_lowOff;
   if (cLed == led_flash_rd)
      regLedVlogic->hbebf_led.LED_color = Reg_bebf::highRed_lowOff;

   if (cLed == Visa::led_flash_gnrd)
      regLedVlogic->hbebf_led.LED_color = Reg_bebf::highRed_lowGrn;

   if ((cLed == Visa::led_flash_gn) ||
       (cLed == Visa::led_flash_rd) ||
       (cLed == Visa::led_flash_gnrd))
      regLedVlogic->hbebf_led.LED_Freq  = Reg_bebf::freq_1dot78_Hz;
   else
      regLedVlogic->hbebf_led.LED_Freq  = Reg_bebf::freq_57_Hz;

   regLedVlogic->hbebf_led.LED_duty  = Reg_bebf::duty_125;

   /** Vlogic */
   if (volt == 3)
      regLedVlogic->hbebf_led.Vlocic_lvl     = Reg_bebf::V_LEVEL_3V3;
   else if (volt == 5)
      regLedVlogic->hbebf_led.Vlocic_lvl     = Reg_bebf::V_LEVEL_3V3;
   else
      qWarning();

   if (VlogicEn)
      regLedVlogic->hbebf_led.Vlocic_enb     = Reg_bebf::Vlogic_5V_or_3V;
   else
      regLedVlogic->hbebf_led.Vlocic_enb     = Reg_bebf::Vlogic_off;

   if (vr == 0)
      vr = VisaReg::getInstance();

   QByteArray ret = vr->writeToReg(hwRegs, VisaReg::RetFormBIN);
   driver->serial->flush();
}
/* ======================================================================== */
/*                     class accessors                                      */
/* ======================================================================== */
quint8 Visa::getBaudrate() {
   return visaInfo.BaudrateCode;
}
QString Visa::getFPGA_FW() {
   return visaInfo.FPGA_FW;
}
QString Visa::getHw_rev() {
   return visaInfo.Hw_rev;
}
int Visa::getSerialNo() {
   return visaInfo.SerialNo;
}
QString Visa::getVcom() {
   return visaInfo.Vcom;
}
double Visa::getBiasVoltage() {
   return visaInfo.BiasVoltage;
}
bool Visa::getModeSnifferIsChecked() {
   return ui->actionModeSniffer->isChecked();
}
bool Visa::getParserIsChecked() {
   return ui->actionParserEnable->isChecked();
}
bool Visa::getUiClearConsoleIsChecked() {
   return ui->actionClearConsole->isChecked();
}
void Visa::setUiClearConsoleChecked(bool b) {
   ui->actionClearConsole->setChecked( b );
}
void Visa::setUiSuppressDefaultPutChecked(bool b) {
   ui->actionSuppressDefaultPut->setChecked(b);
}
bool Visa::getUiSuppressDefaultPutIsChecked() {
   return ui->actionSuppressDefaultPut->isChecked();
}
void Visa::setUiPeriodicReqChecked(bool b) {
   ui->actionPeriodicRequest->setChecked(b);
}
bool Visa::getUiPeriodicReqIsChecked() {
   return ui->actionPeriodicRequest->isChecked();
}
/* ======================================================================== */
/*                     Event handler                                      */
/* ======================================================================== */
void Visa::keyPressEvent(QKeyEvent *event) {
   bool ok = false;
   int val; int len;
   QString sval;

   /** If le8a8b has focus and key f got pressed, change le8a8b format */
   if ((event->key() == Qt::Key_Escape) &&
       (ui->le8a8b->hasFocus())) {
      QString s;
      sval = ui->le8a8b->text();
      len = ui->le8a8b->text().length();
      ui->le8a8b->clear();

      /** Check current format */
      switch (le8a8bForm) {
         case Visa::form_dec: {
            val = sval.right(len - 0).toInt(&ok, 10);
            s = QString("0x%1").arg(val, 4, 16, QLC);
            ui->le8a8b->setText(s);
            le8a8bForm = Visa::form_hex;
         }; break;

         case Visa::form_hex: {
            val = sval.right(len - 2).toInt(&ok, 16);
            s = QString("0b%1").arg(val, 16, 2, QLC);
            ui->le8a8b->setText(s);
            le8a8bForm = Visa::form_bin;
         }; break;

         case Visa::form_bin: {
            val = sval.right(len - 2).toInt(&ok, 2);
            s = QString("%1").arg(val, 0, 10, QLC);
            ui->le8a8b->setText(s);
            le8a8bForm = Visa::form_dec;
         }; break;
         default: {
            qWarning();
         }
      }
   }
   /**
    * If Enter pressed event detected and the command line uiCommand has
    * focus, emit a uiCommandQuery signal ...
    */
   if ((event->key() == Qt::Key_Enter) &&
       (ui->uiCmdLine->hasFocus())) {
      emit uiCmdLineQuery( ui->uiCmdLine->text() );
   }
}
void Visa::saveAllGeometrys() {
   QSETTINGS;
   config.setValue(ui->splIOEd->objectName() + STAT,
                   ui->splIOEd->saveState() );
   config.setValue(ui->splCent->objectName() + STAT,
                   ui->splCent->saveState() );

   config.setValue(this->objectName() + GEOM,
                   this->saveGeometry() );
   config.setValue(this->objectName() + STAT,
                   this->saveState() );

}
void Visa::restoreAllGeometrys() {
   QSETTINGS;
   ui->splIOEd->restoreState(
            config.value(ui->splIOEd->objectName() + STAT, "").toByteArray());
   ui->splCent->restoreState(
            config.value(ui->splCent->objectName() + STAT, "").toByteArray());

   restoreGeometry(config.value(objectName() + GEOM, "").toByteArray());
   restoreState(config.value(objectName() + STAT, "").toByteArray());

}
void Visa::showEvent(QShowEvent *) {
   QSETTINGS;
   restoreAllGeometrys();
   ui->uiCmdLine->setText( config.value(objectName()
                                        + "/uiCmdLineQuery","").toString());
}
void Visa::closeEvent(QCloseEvent *) {
   saveAllGeometrys();

   QList<QAction *> mwActs =
         qApp->findChildren<QAction *>( );

   QList<QWidget *> widgets = parentWidget()->findChildren<QWidget *>();
   Q_INFO << widgets;

   foreach (QAction *act, mwActs) {
      if (act->objectName().contains("visa", Qt::CaseInsensitive))
         act->setChecked( false );
   }
}
