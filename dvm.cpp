
#include "dvm.h"
#include "ui_dvm.h"
#include "visareg.h"
#include "QObject"
#include "globals.h"
#include "hwreg.h"
#include "calc.h"

#include <QFlag>
#include <QFlags>


class HwReg;
class VisaReg;
class Visa;
class Calc;


Dvm* Dvm::instDc = 0; 
Dvm* Dvm::instAcDc = 0; 

/* ======================================================================== */
/*                     class constructor                                    */
/* ======================================================================== */
Dvm::Dvm(const DvmTypes type, QWidget *parent) :
   QDockWidget(parent),
   ui(new Ui::Dvm) {
   ui->setupUi(this);

   waitForDvmUi = true;
   refTim = new QTimer();
   visa = Visa::getObjectPtr();
   calc = Calc::getInstance();

   refTim->start(500);

   vr       = VisaReg::getInstance();
   driver   = Driver::getInstance();
   //   ioedit   = IOEdit::getInstance();

   QSETTINGS;

   QString cfgType;
   if (type == dvmType_DC) {
      cfgType = tr("DvmDc");
      setWindowTitle(DVM_DC_TITLE);
      dvmConf = &dvmCfg_DC;
   }
   else if (type == dvmType_ACDC) {
      cfgType = tr("DvmAcDc");
      setWindowTitle(DVM_ACDC_TITLE);
      dvmConf = &dvmCfg_ACDC;
   }
   else qWarning();

   dvmHasType = type;

   /** Push Dvm range and input selection register union into the config vec */
   hwRegs.push_back( vr->regDvmRngInp );

   /* ---------------------------------------------------------------- */
   /*                      Connect callbacks                           */
   /* ---------------------------------------------------------------- */
   connectSignalSlots();
   /* ---------------------------------------------------------------- */

   dvmConf->enabled = false;
   dvmConf->inpSel  = Dvms::Input_differential;
   dvmConf->mode    = Dvms::Mode_DC;
   dvmConf->selRng  = Dvms::Range2V;
   dvmConf->periode = Dvms::Periode_100ms;
   dvmConf->ovRng   = false;

   ui->ledDvmGn->setColor(QColor(Qt::green));
   ui->ledDvmRd->setColor(QColor(Qt::red));
   ui->ledDvmGn->turnOff();
   ui->ledDvmRd->turnOff();

   deployConfigs( dvmConf );

   restoreGeometry(config.value( cfgType + "/geometry", "").toByteArray());

   return;

   //   dvmCfg_DC.enabled = false;
   //   dvmCfg_DC.inpSel  = Dvm::Input_single;
   //   dvmCfg_DC.mode    = Dvm::Mode_DC;
   //   dvmCfg_DC.selRng  = Dvm::Range2V;

   //   dvmCfg_ACDC.enabled = false;
   //   dvmCfg_ACDC.inpSel  = Dvm::Input_differential;
   //   dvmCfg_ACDC.mode    = Dvm::Mode_DC;
   //   dvmCfg_ACDC.selRng  = Dvm::Range2V;
   //   dvmCfg_ACDC.periode = Dvm::Periode_100ms;

   //   deployConfigs( dvmCfg_DC, dvmCfg_ACDC );

   return;



   ui->slRng->setValue( config.value( cfgType + "/slRng", 0).toInt());

   //   dvmCfg_DC.inpSel = (Dvm::Dvm_Mx_Input)
   //         config.value( cfgType + "/inpSel", Dvm::Input_single).toInt();
   //   updateDvmConfig();
   ui->rbDC->setChecked( config.value( cfgType + "/rbDC", false).toBool());
   ui->rbVdb->setChecked( config.value( cfgType + "/rbVdb", false).toBool());
   ui->rbAC->setChecked( config.value( cfgType + "/rbAC", true).toBool());
}
/* ======================================================================== */
/*                     class destructor                                     */
/* ======================================================================== */
Dvm::~Dvm() {
   QSETTINGS;

   QString cfgType;
   if (dvmHasType == dvmType_DC) {
      cfgType = tr("DvmDc");
   }
   else
      if (dvmHasType == dvmType_ACDC) {
         cfgType = tr("DvmAcDc");
      }
      else
         qWarning();

   config.setValue(cfgType + "/geometry", saveGeometry());
   config.setValue(cfgType + "/slRng", ui->slRng->value());
   //   config.setValue(cfgType + "/inpSel", dvmCfg_DC.inpSel);

   config.setValue(cfgType + "/rbDC", ui->rbDC->isChecked());
   config.setValue(cfgType + "/rbAC", ui->rbAC->isChecked());
   config.setValue(cfgType + "/rbVdb", ui->rbVdb->isChecked());
   delete ui;
}

void Dvm::connectSignalSlots() {
   connect( ui->conGnd,    SIGNAL(clicked()),
            this,          SLOT(onBtnConnectorsClicked()));
   connect( ui->conMinus,  SIGNAL(clicked()),
            this,          SLOT(onBtnConnectorsClicked()));
   connect( ui->conPlus,   SIGNAL(clicked()),
            this,          SLOT(onBtnConnectorsClicked()));

   connect( ui->rbAC,      SIGNAL(clicked()),
            this,          SLOT(onRbxxClicked()));
   connect( ui->rbDC,      SIGNAL(clicked()),
            this,          SLOT(onRbxxClicked()));
   connect( ui->rbVdb,           SIGNAL(clicked()),
            this,                SLOT(onRbxxClicked()));

   connect( ui->btnDvmA,         SIGNAL(clicked()),
            this,                SLOT(onBtnDvmAClicked()));
   connect( ui->btnDvmB,         SIGNAL(clicked()),
            this,                SLOT(onBtnDvmBClicked()));
   connect( ui->btnReadRegs,     SIGNAL(clicked()),
            this,                SLOT(onBtnReadRegsClicked()));

   connect( refTim,              SIGNAL(timeout()),
            this,                SLOT(onLcdRefresh()));

   connect( ui->leLcdRefrshPeri, SIGNAL(returnPressed()),
            this,                SLOT(onLcdRefreshPeriReturnPress()));
   connect( ui->rbDvmContSample, SIGNAL(clicked()),
            this,                SLOT(onLcdRefreshPeriReturnPress()));
   connect( ui->slRng,           SIGNAL(valueChanged(int)),
            this,                SLOT(onSlRngChanged(int)));
   connect( ui->btnCalcAdcValue, SIGNAL(clicked()),
            calc,                SLOT(physCalcADC()));
   connect( ui->btnGetChilds,    SIGNAL(clicked()),
            this,                SLOT(childsToDbg()));

   connect( ui->btnCfgRec,       SIGNAL(clicked()),
            this,                SLOT(onBtnCfgRecClicked()));
   connect( ui->btnOnOff,        SIGNAL(clicked(bool)),
            this,                SLOT(onBtnOnOffClicked(bool)));

}
/*!
 \brief

*/
void Dvm::onBtnCfgRecClicked() {
   QString tmp = HEAD_FPGA_COMM +
         tr("05 0001 8A 02 0001 62 01 0005 72 00 00 00 00 00 00 02"
            "BE 15 00 00 47 E0");
   QByteArray retRaw;
   tmp.remove(" ");
   retRaw.append( visa->convert( tmp ), tmp.length()/2 );
   //   IOEdit *ioedit = IOEdit::getObjectPtr();
   Driver *driver = Driver::getObjectPtr();
   driver->rxBuffReq.expectedByteCount = 0x47;
   driver->writeData(retRaw);

   //   ioedit->putTxData( tmp );
}
/*!
 \brief

*/
void Dvm::onRbxxClicked() {

}
/*!
 \brief

*/
void Dvm::onLcdRefreshPeriReturnPress() {
   refTim->stop();
   refTim->setInterval( ui->leLcdRefrshPeri->text().toInt() );
   if (ui->rbDvmContSample->isChecked()) {
      refTim->start();
      visa->setVisaLED_VLogic(Visa::led_flash_gn, true, 5, vr);
   }
   else
      visa->setVisaLED_VLogic(Visa::led_flash_rd, true, 5, vr);

}
/*!
 \brief

*/
void Dvm::onLcdRefresh() {
   if (! waitForDvmUi) {
      if (! ui->rbDvmContSample->isChecked()) {
         refTim->stop();
         visa->setVisaLED_VLogic(Visa::led_flash_rd, true, 5);
      }
//      if (this->isVisible())
//         vr->reqDefaultReg();

      if (ui->btnOnOff->isChecked()) {
         ui->lcd->display( LCDval );
         ui->lcd->setDigitCount(6);
         ui->lcd->setWindowOpacity(0.5);
      }
      else {
         ui->lcd->display( "---" );
      }
   }
   else {
      refTim->stop();
      waitForDvmUi = false;
   }
}
/*!
 \brief

*/
void Dvm::onBtnDvmAClicked() {

   qWarning();
   /*   QVector<HwReg*> hwRegs;
   //   QVector<HwReg*>::iterator it;

   //   hwRegs.push_back( regDvmRngInp );
   //   hwRegs.push_back( regSetPwr );
   //   hwRegs.push_back( regLedVlogic );

   //   for ( it = hwRegs.begin(); it != hwRegs.end(); *it++) {
   //      (*it)->AboutRegister();
   //   }
   //   for ( it = hwRegs.begin(); it != hwRegs.end(); *it++) {
   //      qout() << "Union size:" << (*it)->size();
   //   }

   //   regDvmRngInp->h8a8b_rng.M1_rng   = Reg_8a8b::Range20V;
   //   regDvmRngInp->h8a8b_rng.M2_rng   = Reg_8a8b::Range20V;
   //   regDvmRngInp->h8a8b_rng.M1_inpt  = Reg_8a8b::Input_single;
   //   regDvmRngInp->h8a8b_rng.M2_inpt  = Reg_8a8b::Input_single;

   //   regSetPwr->h8087_pows.ILim_plus = 250;
   //   regSetPwr->h8087_pows.ILim_minus = 0;
   //   regSetPwr->h8087_pows.Vplus_set = 250;
   //   regSetPwr->h8087_pows.Vplus_set = 250;

   //   regLedVlogic->hbebf_led.LED_Freq    = Reg_bebf::freq_1dot78_Hz;
   //   regLedVlogic->hbebf_led.LED_color   = Reg_bebf::highGrn_lowOff;
   //   regLedVlogic->hbebf_led.LED_duty    = Reg_bebf::duty_125;
   //   regLedVlogic->hbebf_led.Vlocic_lvl  = Reg_bebf::V_LEVEL_5V;
   //   regLedVlogic->hbebf_led.Vlocic_enb  = Reg_bebf::Vlogic_5V_or_3V;

   //   QByteArray ret = vr->writeToReg(hwRegs, VisaReg::RetFormBIN);
*/

}
/*!
 \brief

*/
void Dvm::onBtnDvmBClicked() {
   vr->reqVisaInfoFromMicro();

}
/*!
 \brief

*/
void Dvm::onBtnReadRegsClicked() {
   vr->testReadReg();
}

/* ======================================================================== */
/*                Tx write Registers implementations                        */
/* ======================================================================== */
double Dvm::getLCDval() const {
   return LCDval;
}

void Dvm::setLCDval(double value) {
   LCDval = value;
   ui->ledDvmRd->turnOn( dvmConf->ovRng );
}

/** Dvm input selection - Single || Differential */
void Dvm::onBtnConnectorsClicked() {
   if (dvmConf->inpSel == Dvms::Input_single) {
      dvmConf->inpSel = Dvms::Input_differential;
      ui->laDvm->setText("DVM 2");
   }
   else if (dvmConf->inpSel == Dvms::Input_differential) {
      dvmConf->inpSel = Dvms::Input_single;
      ui->laDvm->setText("DVM 1");
   }
   else {
      qWarning();
      ui->laDvm->setText("???");
   }

   deployConfigs();
}

/** Select range 0.2V - 2V - 20V */
void Dvm::onSlRngChanged(int val) {
   /** If Dvm instance has on/off button pressed */
   if (ui->btnOnOff->isChecked()) {
      switch (val) {
         case 0: {
            dvmConf->selRng = Dvms::RangeOff;
         }; break;
         case 1: {
            dvmConf->selRng = Dvms::Range200mV;
         }; break;
         case 2: {
            dvmConf->selRng = Dvms::Range2V;
         }; break;
         case 3: {
            dvmConf->selRng = Dvms::Range20V;
         }; break;
         default: qWarning();
      }
   }
   else {
      /** If not, range is always set to "off" */
      dvmConf->selRng = Dvms::RangeOff;
      ui->ledDvmGn->turnOff();
   }

   deployConfigs();
}

/** Button On Off pressed */
void Dvm::onBtnOnOffClicked(bool onoff) {
   dvmConf->enabled = onoff;
   deployConfigs();
}

/** Mode select - AC - DC */
void Dvm::onRbModeSel() {
   if (dvmHasType == dvmType_ACDC) {
      if (ui->rbAC->isChecked()) {
         dvmConf->mode = Dvms::Mode_AC;
      }
      else if (ui->rbDC->isChecked()) {
         dvmConf->mode = Dvms::Mode_DC;
      }

      deployConfigs();
   }
}

void Dvm::deployConfigs() {
   deployConfigs(dvmConf);
}

/** Die Klasse zur konfiguration der Dvm's DC + AC/DC ist bis jetzt mal für
 * beide Dvms gültig und somit werden die configs 0x8a (Dvm DC) und 0x8b
 * (Dvm AC/DC) immer gleichzeitig an das FPGA übertragen. Da zwei Instanzen der
 * dieser Klasse laufen, muss in deployConfigs(..) festgestellt werden ob der
 * aktuelle deploy vom DC oder ACDC meter ausgelöst wurde. Der Registerinhalt
 * des inaktiven Dvm's MUSS anschließend vor dem schreiben der aktiv geänderten
 * konfiguration vom FPGA abgeholt und wieder mit geschickt werden.
 */
void Dvm::deployConfigs(dvm_config_t *cfg) {

   /**
    * Request visascope for the register contents of the non-active config
    * dvm
    */

   if (dvmHasType == dvmType_DC) {
      /** Translate Dvm A (DC) config */
      vr->regDvmRngInp->h8a8b_rng.M1_rng = cfg->selRng;
      vr->regDvmRngInp->h8a8b_rng.M1_inpt = cfg->inpSel;
   }
   else if (dvmHasType == dvmType_ACDC) {
      /** Translate Dvm B (AC/DC) config */
      vr->regDvmRngInp->h8a8b_rng.M2_rng = cfg->selRng;
      vr->regDvmRngInp->h8a8b_rng.M2_inpt = cfg->inpSel;

      /** 0 Measurement-time T=100ms || 1 Measurement-time T=233.33ms */
      vr->regDvmRngInp->h8a8b_rng.AD2_T = cfg->periode;

      /** ACDC-measurement switched off (0) || on (1) */
      vr->regDvmRngInp->h8a8b_rng.AD2_on = cfg->mode;
   }

   /** Set up connector / Dvm mode ui's */
   if (cfg->inpSel == Dvms::Input_single) {
      ui->conPlus->setEnabled(true);
      ui->conGnd->setEnabled(false);
      ui->conMinus->setEnabled(false);
      ui->laDvm->setText("DVM 1");
   }
   else if (cfg->inpSel == Dvms::Input_differential) {
      ui->conPlus->setEnabled(false);
      ui->conGnd->setEnabled(true);
      ui->conMinus->setEnabled(true);
      ui->laDvm->setText("DVM 2");
   }

   /** No enabled radio buttons for DC Dvm */
   if (dvmHasType == dvmType_DC) {
      ui->rbDC->setChecked(true);
      ui->rbVdb->setChecked(false);
      ui->rbAC->setChecked(false);

      ui->rbVdb->setEnabled(false);
      ui->rbAC->setEnabled(false);
   }
   else if (dvmHasType == dvmType_ACDC) {
      if (cfg->mode == Dvms::Mode_AC) {
         ui->rbDC->setChecked(false);
         ui->rbAC->setChecked(true);
      }
      else if (cfg->mode == Dvms::Mode_DC) {
         ui->rbDC->setChecked(true);
         ui->rbAC->setChecked(false);
      }
      ui->rbVdb->setChecked(false);
   }

   QByteArray ret = vr->writeToReg(hwRegs, VisaReg::RetFormBIN);
   Q_INFO << ret;
}

/**
 * @brief Dvm::txDvmConfig Periodically called Dvm configuration transmitter
 *
 * 0500018a030001620100057200000000000002be15000047e0 	open V-DC	 (DVM1, 0.2V)
 * 0500018a0b0001620100057200000000000002be15000047e0 	VDC: DVM2
 * 0500018a0a0001620100057200000000000002be15000047e0 	VDC: 0.6V/2V
 * 0500018a090001620100057200000000000002be15000047e0 	VDC: 6V/20V
 * 0500018a000001620100057200000000000002be15000047e0 	VDC: off
 * 0500018b430001620100057200000000000002be15000047e0	open V-ADC (DVM1, 0.2V, DC, 100ms)
 * 0500018b4b0001620100057200000000000002be15000047e0 	VADC: DVM2
 * 0500018b4a0001620100057200000000000002be15000047e0 	VAD C: 0.6V/2V
 * 0500018b490001620100057200000000000002be15000047e0 	VADC: 6V/20V
 * 0500018b000001620100057200000000000002be15000047e0       VAC: off
 * VADC: DC->AC->VdB ist nur sw
 */
void Dvm::txDvmConfig() {
#define SEQ tr("01 0001 %1 %2")

   /** initial disabled */
   qint8 h8x = 0x00; QString seq;

   /** Don't execute while waitForDvmUi not set! */
   if (! waitForDvmUi) {
      if (this->dvmHasType == dvmType_DC) {
         /** greater than zero means, dvm is not disabled */
         if (dvmConf->selRng) {
            h8x += dvmCfg_DC.inpSel;
            h8x += dvmCfg_DC.selRng;
         }
         seq = SEQ.arg(0x8a, 2, 16, QLC).arg(h8x, 2, 16, QLC);
      }
      else if (this->dvmHasType == dvmType_ACDC) {
         /** greater than zero means, dvm is not disabled */
         if (dvmConf->selRng) {
            h8x += dvmCfg_DC.inpSel;
            h8x += dvmCfg_DC.selRng;
            h8x += 0x40;
         }
         seq = SEQ.arg(0x8b, 2, 16, QLC).arg(h8x, 2, 16, QLC);
      }

      seq.remove(" ");

      VisaReg *vr = VisaReg::getObjectPtr();
      QByteArray trxReqRaw;
      trxReqRaw.append( vr->convert(seq), seq.length()/2 );

      VisaReg::trxSts ret = VisaReg::trx_initial;
      int ctr = 5;

      while (ret != VisaReg::trx_success) {
         //         if (ret != VisaReg::trx_running_req) {
         ret = vr->trxVisa(trxReqRaw, tr("onRxRespDefaultReadCmpl(QByteArray&)") );
         //         }
         if (!ctr--)
            break;
      }
   }
   else {
      ///< Called only once
      QTimer::singleShot(25, Qt::CoarseTimer,
                         this, SLOT(dvmTRXConfigUpdate()));
   }
}

void Dvm::txDvmConfigACORDC() {
//   //#define SEQ tr("05 0001 %1 %2 0001 62 01 0005 72 0000000000 0002 be 1500 0047e0")
//#define SEQ tr("01 0001 %1 %2")

//   /** initial disabled */
//   qint8 h8x = 0x00; QString seq;

//   /** Don't execute while waitForDvmUi not set! */
//   if (! waitForDvmUi) {
//      if (this->dvmHasType == dvmType_DC) {
//         /** greater than zero means, dvm is not disabled */
//         if (dvmConf->selRng) {
//            h8x += dvmCfg_DC.inpSel;
//            h8x += dvmCfg_DC.selRng;
//         }
//         seq = SEQ.arg(0x8a, 2, 16, QLC).arg(h8x, 2, 16, QLC);
//      }
//      else if (this->dvmHasType == dvmType_ACDC) {
//         /** greater than zero means, dvm is not disabled */
//         if (dvmConf->selRng) {
//            h8x += dvmCfg_DC.inpSel;
//            h8x += dvmCfg_DC.selRng;
//            h8x += 0x40;
//         }
//         seq = SEQ.arg(0x8b, 2, 16, QLC).arg(h8x, 2, 16, QLC);
//      }

//      seq.remove(" ");

//      VisaReg *vr = VisaReg::getObjectPtr();
//      QByteArray trxReqRaw;
//      trxReqRaw.append( vr->convert(seq), seq.length()/2 );

//      VisaReg::trxSts ret = VisaReg::trx_initial;
//      int ctr = 5;

//      while (ret != VisaReg::trx_success) {
//         //         if (ret != VisaReg::trx_running_req) {
//         ret = vr->trxVisa(trxReqRaw, tr("onRxRespDefaultReadCmpl(QByteArray&)") );
//         //         }
//         if (!ctr--)
//            break;
//      }
//   }
//   else {
//      ///< Called only once
//      QTimer::singleShot(25, Qt::CoarseTimer,
//                         this, SLOT(dvmTRXConfigUpdate()));
//   }
}

void Dvm::dvmTxConfigUpdate2() {
#define QFOLDINGSTART {
   //   QVector<HwReg*> hwRegs;
   //   QVector<HwReg*>::iterator it;

   //   /** Don't execute while waitForDvmUi not set! */
   //   if (waitForDvmUi) {
   //      hwRegs.push_back( vr->regDvmRngInp );

   //      for ( it = hwRegs.begin(); it != hwRegs.end(); *it++) {
   //         (*it)->AboutRegister();
   //      }
   //      for ( it = hwRegs.begin(); it != hwRegs.end(); *it++) {
   //         qout() << "Union size:" << (*it)->size();
   //      }

   //      if (this->dvmHasType == dvmType_DC) {
   //         vr->regDvmRngInp->h8a8b_rng.M1_rng   =
   //               (Reg_8a8b::h8a8b_Mx_Rngs) dvmCfg_DC.selRng ;
   //         vr->regDvmRngInp->h8a8b_rng.M1_inpt   =
   //               (Reg_8a8b::Dvm_Mx_Inputs) dvmCfg_DC.inpSel ;
   //      }
   //      else if (this->dvmHasType == dvmType_ACDC) {
   //         vr->regDvmRngInp->h8a8b_rng.M2_rng   =
   //               (Reg_8a8b::h8a8b_Mx_Rngs) dvmCfg_DC.selRng ;
   //         vr->regDvmRngInp->h8a8b_rng.M2_inpt   =
   //               (Reg_8a8b::Dvm_Mx_Inputs) dvmCfg_DC.inpSel ;
   //      }

   //      QByteArray ret = vr->writeToReg(hwRegs, VisaReg::RetFormBIN);
   //   }
   //   else {
   //      QTimer::singleShot(25, Qt::CoarseTimer,
   //                         this, SLOT(dvmTxConfigUpdate()));
   //   }

   //   if (dvmCfg_DC. inpSel == Dvm::DVM1_SINGLE_END) {
   //      vr->regDvmRngInp->h8a8b_rng.M1_inpt  = Reg_8a8b::Input_single;
   //      vr->regDvmRngInp->h8a8b_rng.M2_inpt  = Reg_8a8b::Input_single;
   //   }
   //   if (dvmCfg_DC. inpSel == Dvm::DVM2_DIFFERENTIAL) {
   //      vr->regDvmRngInp->h8a8b_rng.M1_inpt  = Reg_8a8b::Input_differential;
   //      vr->regDvmRngInp->h8a8b_rng.M2_inpt  = Reg_8a8b::Input_differential;
   //   }
   //   /* off?? */

   ////   h8a8b_Mx_Rngs selRng;
   //   if (dvmCfg_DC.selRng == (uint8_t) Reg_8a8b::Range20V) {      /** 0x01 */
   //      vr->regDvmRngInp->h8a8b_rng.M1_rng   = Reg_8a8b::Range20V;
   //      vr->regDvmRngInp->h8a8b_rng.M2_rng   = Reg_8a8b::Range20V;
   //   }
   //   if ((dvmCfg_DC.selRng == (uint8_t) Reg_8a8b::Range2V)) {      /** 0x01 */
   //      vr->regDvmRngInp->h8a8b_rng.M1_rng   = Reg_8a8b::Range20V;
   //      vr->regDvmRngInp->h8a8b_rng.M2_rng   = Reg_8a8b::Range20V;
   //   }

#define QFOLDINGEND }

}
/* ======================================================================== */
/* ======================================================================== */


/* ======================================================================== */
/*                     pretty little helpers                                */
/* ======================================================================== */
/*!
 \brief

*/
void Dvm::childsToDbg() {
   Q_INFO << this->children();
}
/*!
 \brief

*/
void Dvm::updateDvmUi() {
   if (dvmConf->inpSel == Dvms::Input_differential) {
      ui->conMinus->setEnabled(true);
      ui->conGnd->setEnabled(true);
      ui->conPlus->setEnabled(false);
   }
   else if (dvmConf->inpSel == Dvms::Input_single) {
      ui->conMinus->setEnabled(false);
      ui->conGnd->setEnabled(false);
      ui->conPlus->setEnabled(true);
   }
   else
      qWarning();
}
