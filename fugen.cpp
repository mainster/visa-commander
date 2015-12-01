#include <QTime>
#include <QTimer>
#include <QInputDialog>
#include <QDebug>
#include <QIcon>
#include <typeinfo>

#include "ui_fugen.h"
#include "fugen.h"

#include "globals.h"
#include "ioedit.h"
#include "visareg.h"
#include "visa.h"

class FuGen;
class VisaReg;

//#define  WHEELEVENT_OBJECT_NAME   1

FuGen    *FuGen::inst   = 0x00;
VisaReg  *FuGen::vr     = 0x00;

#define  DELTA_NORM  120

/* ======================================================================== */
/*                     class constructor                                    */
/* ======================================================================== */
FuGen::FuGen(QWidget *parent) :
   QDockWidget(parent),
   ui(new Ui::FuGen) {
   ui->setupUi(this);
   this->setAttribute(Qt::WA_DeleteOnClose);

   DVAMP_PER_TICK = (VAMP_MAX - VAMP_MIN)/ VAMP_MAX_TICKS,
         DFOUT_PER_TICK = (FOUT_MAX - FOUT_MIN)/ FOUT_MAX_TICKS;

   timCycl = new QTimer();
   timCycl->setInterval( INTERVAL_LCD * 1e3 );
   timCycl->start();

   genCfg = new FuGen::genCfg_t();

   vr       = VisaReg::getObjectPtr();

   ioeditL  = IOEdit::getObjectPtr();
   visa     = Visa::getObjectPtr();

   initUiElements();

   connect( timCycl,          SIGNAL(timeout()),
            this,             SLOT(onCyclic()));
   connect( ui->btnOnOff,     SIGNAL(clicked()),
            this,             SLOT(onBtnOnOffClicked()));
   connect( ui->cbWaveform,   SIGNAL(currentIndexChanged(int)),
            this,             SLOT(onConfigChangeTriggered(int)));
   connect( ui->cbNoiseType,  SIGNAL(currentIndexChanged(int)),
            this,             SLOT(onConfigChangeTriggered(int)));
   connect( this,             SIGNAL(destroyed(QObject*)),
            visa,             SLOT(onChildWidgetDestroyed(QObject*)));

//   vr->hwRegs.push_back( vr->regFgenOut );
//   vr->hwRegs.push_back( vr->regFgenFreq );

}

FuGen::~FuGen() {
   delete ui;
}

void FuGen::initUiElements() {
   ui->cbWaveform->addItem(QIcon("://images/waveformSine.png"), "Sine",
                           QVariant::fromValue(FuGens::form_sinusod));
   ui->cbWaveform->addItem(QIcon("://images/waveformRect.png"), "Rect",
                           QVariant::fromValue(FuGens::form_rect));

   ui->cbNoiseType->addItem("Binary", QVariant::fromValue(
                               FuGens::typ_binary_distributed));
   ui->cbNoiseType->addItem("Gaussian", QVariant::fromValue(
                               FuGens::typ_binary_distributed));
   ui->cbNoiseType->addItem("Uniform", QVariant::fromValue(
                               FuGens::typ_binary_distributed));
}
void FuGen::onBtnOnOffClicked() {

   ioeditL= IOEdit::getObjectPtr();

   ioeditL->putInfoLine("Output off");

}
void FuGen::onConfigChangeTriggered(int idx) {
   Q_UNUSED(idx);
   QVariant varWav  = ui->cbWaveform->currentData();
   QVariant varNoise = ui->cbNoiseType->currentData();

   if ( varWav.canConvert<FuGens::wave_forms>() )
      genCfg->waveForm = qvariant_cast<FuGens::wave_forms>(varWav);
   else return;

   if ( varWav.canConvert<FuGens::wave_form>() )
      genCfg->waveForm = qvariant_cast<FuGens::wave_form>(varWav);
   else return;

   if ( varNoise.canConvert<FuGens::noise_typs>() )
      genCfg->noiseModel = qvariant_cast<FuGens::noise_typs>(varNoise);
   else return;

   if ( varNoise.canConvert<FuGens::noise_typ>() )
      genCfg->noiseModel = qvariant_cast<FuGens::noise_typ>(varNoise);
   else return;

//   visa->gs.changedFuGenCfg = true;

}
void FuGen::loadSineIntoFPGA() {
   visa->writeRam(&sinTbl, 0xc0);
}

/**
 * Encoding Output amplitude (page 57..)
 * Determining Amp_Rng is now very easy: Check both conditions, starting with
 * Amp_Rng=3 and decrease Amp_Rng as long as both conditions are fulﬁlled. The
 * aim ist to ﬁnd the smalles possible value of KX without violating the
 * range-conditions.
 *
 * Note: Using Amp_Rng = 3 fulﬁlls both equations under all circumstances, but
 * in case of small output-amplitudes and offset-values, you will suffer from
 * too much noise and offset-error introduced by the output-ampliﬁer;
 * therefore, chooseKX as small as possibe without violating the two
 * conditions.
 */
/**
 * Return "calced" amp_rng which depends on physical target ampl voltage
 */
FuGens::amp_rngs FuGen::calcAmp_rng() {
   // genCfg->Amp.float_;
   /**
    * Sollte für kleine amplituden berechnet werden, range check dann
    * nicht vergessen (S.57)
    */
   return FuGens::rng_1;
}
/**
 * Return current scale factor Kx which corresponds to amp_rng
 */
double FuGen::Kx(FuGens::amp_rngs fRng) {
   VisaReg *vr = VisaReg::getObjectPtr();

   if (fRng == FuGens::rng_0308)
      return (double) vr->H[33] * vr->H[34];
   if (fRng == FuGens::rng_11321)
      return (double) vr->H[33];
   if (fRng == FuGens::rng_27152)
      return (double) vr->H[34];
   if (fRng == FuGens::rng_1)
      return 1.0;
   qWarning();
   return 0.0;
}

void FuGen::onCyclic() {
   /** Refresh lcd views */
   ui->lcdFreq->display( QString::number(genCfg->Freq.float_) + "Hz" );
   ui->lcdAmp ->display( QString::number(genCfg->Amp.float_) + "V");
   ui->lcdOffs->display( QString::number(genCfg->Offs.float_) + "V");
   ui->lcdDuty->display( QString::number(genCfg->Duty.float_) + "%");

   /** If the fuGen-config-hasChanged flag is set ... */
//   if (visa->gs.fuGenCfgAltered)
//      fillTxReg();
}
/**
 * Change LCD values by wheel scrolling
 * event->delta() = +- 120
 *
 * (tested with logitech mx...)
 *
 * 1) Normieren auf: event->delta() = +- 1 (double)
 * 2) Für jedes LCD einen "MAX_TICKS" festlegen, modifier mit berechnen
 * 3) CTRL        + WheelTick: x4
 *    CTRL+Shift  + WheelTick: x10
 *
 *    => Bei x10 soll V_Amp-range 0.1 ... 8Vpp
 *       in 15ticks durchlaufen werden
 *    => Bei  x1 soll V_Amp-range 0.1 ... 8Vpp
 *       in 150ticks durchlaufen werden
 *    => MAX_TICKS_V_AMP = 150
 *    => DELTA_TICK_V_AMP = 8V / 150 = 53.33 mV/Tick
 */
void FuGen::wheelEvent ( QWheelEvent * event ) {
   /**
    * query current keyboard modifiers so we can gain the functionality of
    * "page scrolling". This means without CTRL modifier, normal step width is
    * used and with active CTRL key, the step width is multiplied by a factor
    * of 10.
    */
   Qt::KeyboardModifiers mods =
         QApplication::queryKeyboardModifiers();
   
   /**
    * evDelta is a element of [-1, 1]    (Wheel-down, Wheel-Up)
    */
   short evDelta = (short) (event->delta() / DELTA_NORM);

   /**
    * Acceleration factor could be x1, x4, x10
    */
   double accFact = 1.0;
   
   /**
    * Detect and use modifier keys to accelerate setpoint modification.
    * If CTRL key is pressed while proccesing wheelEvent, the division
    * (better multiplication-) factor div is set to 10. If the SHIFT key also
    * is pressed, the div factor is multiplied by 5...
    */
   (mods & Qt::CTRL ) ? accFact = 4.0 : accFact = 1.0;
   (mods & Qt::SHIFT) ? accFact *= 2.5 : accFact *= 1.0;

   uint16_t DIV = 100/accFact;
   /* ---------------------------------------------------------------- */
   /*         Get the name of object under the mouse pointer           */
   /* ---------------------------------------------------------------- */
   QWidget *widget = qApp->widgetAt(QCursor::pos());
   QString widName = widget->objectName();
#ifdef WHEELEVENT_OBJECT_NAME
   ioeditL->putInfoLine( widName + " " + QString::number(mouseWheelCnt));
#endif
   /**
    * Check if the mouse pointer points to one of the lcd widgets from supply
    * window while this wheelEvent takes place.
    * Add current wheel counts to FuGen::'s "config" structure. Event->delta()
    * can be negative or positive.
    */
   if (ui->btnLock->isChecked())
      return;

//   ioeditL->putInfoLine( QString::number( (double) evDelta*accFact*DVAMP_PER_TICK));

   if (widName.contains( ui->lcdFreq->objectName() ))
      genCfg->convLCD2freq( event->delta()/(DIV), FuGen::type_increment );
   else
      if (widName.contains( ui->lcdAmp->objectName() ))
         genCfg->convLCD2amp( (double) evDelta * accFact * DVAMP_PER_TICK,
                              FuGen::type_increment );
      else
         if (widName.contains( ui->lcdOffs->objectName() ))
            genCfg->convLCD2offs( event->delta()/(DIV), FuGen::type_increment );
         else {
            Q_INFO << tr("Wheel event can't be mapped to an lcd widget");
            ioeditL->putInfoLine(
                     tr("Wheel event can't be mapped to a lcd widget") );
         }

   /** refresh hw registers */
   vr->regFgenOut->h3037_fugen.out_ampl   = genCfg->Amp.reg16_;
   vr->regFgenOut->h3037_fugen.out_offs   = genCfg->Offs.reg16_;
   vr->regFgenOut->h3037_fugen.out_duty   = genCfg->Duty.reg16_;
   vr->regFgenOut->h3037_fugen.noise_type = (uint8_t) genCfg->noiseModel;
   vr->regFgenFreq->h383b_fufreq.dword    = genCfg->Freq.reg32_;

   /** This adds changed register to the next TxStream */
   vr->add2TxStream(vr->regFgenOut);
   vr->add2TxStream(vr->regFgenFreq);
}
//void FuGen::fillTxReg() {
//   vr->regFgenOut->h3037_fugen.out_ampl   = genCfg->Amp.reg16_;
//   vr->regFgenOut->h3037_fugen.out_offs   = genCfg->Offs.reg16_;
//   vr->regFgenOut->h3037_fugen.out_duty   = genCfg->Duty.reg16_;
//   vr->regFgenOut->h3037_fugen.noise_type = (uint8_t) genCfg->noiseModel;

//   vr->regFgenFreq->h383b_fufreq.dword    = genCfg->Freq.reg32_;

//   QByteArray ret = vr->writeToReg(vr->hwRegs, VisaReg::RetFormBIN);
//   visa->gs.fuGenCfgAltered = false;
//   ioeditL->putTxData( ret );
//}

/*!
 \brief Convert output amplitude from the lcds physical float representation to
 the corresponding 12-Bit register value

 \fn FuGen::genCfg_t::convLCD2amp
 \param VAmp   Physical representation of the output amplitude in Volts
 \param type   Adding type, if VAmp is passed by a float based LCD value, type
               must always be specified to be of type FuGen::type_increment
 \return int   Integer representation of VAmp, this value also updates the
               fugen config structure.
*/
///< --------------- convert Amplitude -------------------------------
long FuGen::genCfg_t::convLCD2amp(double dVamp, FuGen::add_type type) {
   FuGens::amp_rngs rng = FuGen::calcAmp_rng(); // 0x03

   if (type == FuGen::type_increment)
      Amp.float_  += dVamp;
   else Amp.float_ = dVamp;

   /**
    * Setpoint limit, Vpp range [0, 8Vpp]
    */
   (Amp.float_ > VAMP_MAX)
         ?  Amp.float_ = VAMP_MAX
         :  Amp.float_ = Amp.float_ ;

   (Amp.float_ < 0)
         ?  Amp.float_ = 0
         :  Amp.float_ = Amp.float_ ;

   /**
    * The Register Amplitude(0x30, 0x31) determines the voltage reference on t
    * he DAC and has to be calculated with this formula:
    * Programmers Manual, page 56
    *
    * H(31) | DDS-Generator | Amplitude 3.5Vrms | ca. 0x3e3f | rel | 10
    * Hc(31): 9.63000
    * Kx(0x03) = 1
    *
    * Amp.int_(0.1Vpp) = 4096/9.63 * 0.1V   =    43dec
    * Amp.int_(8.0Vpp) = 4096/9.63 * 8.0V   =  3403dec
    *
    */
   Amp.reg16_ = (uint16_t)((double) 4095 * Amp.float_/
                           ((double)vr->H[31] * Kx(rng)));

   /**
    * Range check according to page 56, equ. (12.3)
    */
   if (! ((Amp.reg16_ > 0) && (Amp.reg16_ < LIM_12BIT_UINT))) {
      Amp.reg16_ = 0;
      Q_INFO << tr("Range check failed [0...LIM_12BIT_UINT]");
      return -1;
   }

   QStringList IODEBUG; QString str;
   IODEBUG << "Amp.float_" << QString::number( Amp.float_ )
           << "Amp.reg16_" << QString::number( Amp.reg16_ );
   str = IODEBUG.join(" ");

   IOEdit * ioeditR = IOEdit::getObjectPtr(location_Right);
   ioeditR->putInfoLine( str );

   return (long) Amp.reg16_;
}

///< --------------- convert Frequency -------------------------------
long FuGen::genCfg_t::convLCD2freq(double dFreq, FuGen::add_type type) {

   if (type == FuGen::type_increment)
      Freq.float_ += dFreq;
   else Freq.float_ = dFreq;

   /**
    * Range check according to page 54
    */
   (Freq.float_ > FOUT_MAX)
         ?  Freq.float_ = FOUT_MAX
         :  Freq.float_ = Freq.float_ ;

   (Freq.float_ < 0)
         ?  Freq.float_ = 0
         :  Freq.float_ = Freq.float_ ;

   /**
    * Calc integer representation
    *
    * REMINDER:
    * Freq represents a full scaled 32-bit value, be careful
    * with signed datatypes
    */
   Freq.reg32_ = (uint32_t) ((double)FACT_FREQ * Freq.float_);

   QStringList IODEBUG; QString str;
   IODEBUG << "Freq.float_" << QString::number( Freq.float_ )
           << "Freq.reg32_" << QString::number( Freq.reg32_ );
   str = IODEBUG.join(" ");
   IOEdit * ioeditR = IOEdit::getObjectPtr(location_Right);
   ioeditR->putInfoLine( str );

   return (long) Freq.reg32_;
}

///< --------------- convert Offset ----------------------------------
long FuGen::genCfg_t::convLCD2offs(double dVoffs, FuGen::add_type type) {
   FuGens::amp_rngs rng = calcAmp_rng();

   if (type == FuGen::type_increment)
      Offs.float_  += dVoffs;
   else Offs.float_ = dVoffs;

   /**
    * Range check according to page 56, 12.4
    */
   (Offs.float_ > VOFFS_MAX)
         ?  Offs.float_ = VOFFS_MAX
         :  Offs.float_ = Offs.float_ ;

   (Offs.float_ < VOFFS_MIN)
         ?  Offs.float_ = VOFFS_MIN
         :  Offs.float_ = Offs.float_ ;

   /**
    * Calc integer representation
    */
   Offs.reg16_ = (uint16_t) (2047*((double) 1.0 + vr->H[32] *
                             Offs.float_ / Kx(rng)) + vr->H[30]);

   /**
    * Range check according to page 56, equ. (12.4)
    * Not important yet since value of Kx is set fixed to 1
    */
   if (! ((Offs.reg16_ > 410) && (Offs.reg16_ < 3685))) {
      Offs.reg16_ = 0;
      Q_INFO << tr("Range check failed ]410...3685[ ");
      return -1;
   }

   QStringList IODEBUG; QString str;
   IODEBUG << "Offs.float_" << QString::number( Offs.float_ )
           << "Offs.reg32_" << QString::number( Offs.reg16_ );
   str = IODEBUG.join(" ");
   IOEdit * ioeditR = IOEdit::getObjectPtr(location_Right);
   ioeditR->putInfoLine( str );

   return (long) Offs.reg16_;
}

///< --------------- convert Duty ------------------------------------
long FuGen::genCfg_t::convLCD2duty(double dDuty, FuGen::add_type type) {

   if (type == FuGen::type_increment)
      Duty.float_  += dDuty;
   else Duty.float_ = dDuty;

   /**
    * Range check duty cycle value, 0 ... 100 [%]
    */
   (Duty.float_ > DUTY_MAX_PERC)
         ?  Duty.float_ = DUTY_MAX_PERC
         :  Duty.float_ = Duty.float_ ;

   (Duty.float_ < DUTY_MIN_PERC)
         ?  Duty.float_ = DUTY_MIN_PERC
         :  Duty.float_ = Duty.float_ ;

   /**
    * Calc integer representation
    */
   Duty.reg16_ = (uint16_t) ((double) FACT_DUTY * Duty.float_);

   if (Duty.float_ == DUTY_MAX_PERC)
      Duty.reg16_ = 0xffff;

   qDebug().noquote() << "Duty.float_" << Duty.float_
                      << " Duty.reg16_" << Duty.reg16_;

   return (long) Duty.reg16_;
}


#define QFOLDINGSTART {
const QByteArray FuGen::sinTbl =
      "000001010102020303030404050505060607070708080909090A0A0A0B0B0C0C0C0D0D0E0E0E0F0F"
      "10101011111212121313131414151515161617171718181919191A1A1B1B1B1C1C1C1D1D1E1E1E1F"
      "1F20202021212222222323242424252525262627272728282929292A2A2B2B2B2C2C2D2D2D2E2E2E"
      "2F2F30303031313232323333343434353535363637373738383939393A3A3B3B3B3C3C3D3D3D3E3E"
      "3E3F3F40404041414242424343434444454545464647474748484949494A4A4A4B4B4C4C4C4D4D4E"
      "4E4E4F4F50505051515152525353535454555555565656575758585859595A5A5A5B5B5B5C5C5D5D"
      "5D5E5E5F5F5F60606161616262626363646464656566666667676768686969696A6A6A6B6B6C6C6C"
      "6D6D6E6E6E6F6F6F70707171717272737373747474757576767677777778787979797A7A7B7B7B7C"
      "7C7C7D7D7E7E7E7F7F7F80808181818282838383848484858586868687878788888989898A8A8A8B"
      "8B8C8C8C8D8D8D8E8E8F8F8F9090909191929292939393949495959596969697979898989999999A"
      "9A9B9B9B9C9C9C9D9D9E9E9E9F9F9FA0A0A1A1A1A2A2A2A3A3A4A4A4A5A5A5A6A6A6A7A7A8A8A8A9"
      "A9A9AAAAABABABACACACADADAEAEAEAFAFAFB0B0B0B1B1B2B2B2B3B3B3B4B4B4B5B5B6B6B6B7B7B7"
      "B8B8B9B9B9BABABABBBBBBBCBCBDBDBDBEBEBEBFBFBFC0C0C1C1C1C2C2C2C3C3C3C4C4C5C5C5C6C6"
      "C6C7C7C7C8C8C8C9C9CACACACBCBCBCCCCCCCDCDCECECECFCFCFD0D0D0D1D1D1D2D2D3D3D3D4D4D4"
      "D5D5D5D6D6D6D7D7D8D8D8D9D9D9DADADADBDBDBDCDCDCDDDDDEDEDEDFDFDFE0E0E0E1E1E1E2E2E2"
      "E3E3E4E4E4E5E5E5E6E6E6E7E7E7E8E8E8E9E9E9EAEAEBEBEBECECECEDEDEDEEEEEEEFEFEFF0F0F0"
      "F1F1F1F2F2F2F3F3F4F4F4F5F5F5F6F6F6F7F7F7F8F8F8F9F9F9FAFAFAFBFBFBFCFCFCFDFDFDFEFE"
      "FEFFFFFF00000101010202020303030404040505050606060707070808080909090A0A0A0B0B0B0C"
      "0C0C0D0D0D0E0E0E0F0F0F1010101111111212121313131414141515151616161717171818181819"
      "19191A1A1A1B1B1B1C1C1C1D1D1D1E1E1E1F1F1F2020202121212222222323232424242425252526"
      "26262727272828282929292A2A2A2B2B2B2C2C2C2C2D2D2D2E2E2E2F2F2F30303031313132323232"
      "333333343434353535363636373737373838383939393A3A3A3B3B3B3B3C3C3C3D3D3D3E3E3E3F3F"
      "3F3F4040404141414242424343434344444445454546464646474747484848494949494A4A4A4B4B"
      "4B4C4C4C4C4D4D4D4E4E4E4F4F4F4F50505051515151525252535353545454545555555656565657"
      "5757585858585959595A5A5A5A5B5B5B5C5C5C5C5D5D5D5E5E5E5E5F5F5F60606060616161626262"
      "62636363646464646565656666666667676768686868696969696A6A6A6B6B6B6B6C6C6C6C6D6D6D"
      "6E6E6E6E6F6F6F6F7070707171717172727272737373747474747575757576767676777777787878"
      "78797979797A7A7A7A7B7B7B7C7C7C7C7D7D7D7D7E7E7E7E7F7F7F7F808080808181818182828283"
      "8383838484848485858585868686868787878788888888898989898A8A8A8A8B8B8B8B8C8C8C8C8D"
      "8D8D8D8E8E8E8E8F8F8F8F9090909090919191919292929293939393949494949595959596969696"
      "969797979798989898999999999A9A9A9A9A9B9B9B9B9C9C9C9C9D9D9D9D9D9E9E9E9E9F9F9F9FA0"
      "A0A0A0A0A1A1A1A1A2A2A2A2A2A3A3A3A3A4A4A4A4A5A5A5A5A5A6A6A6A6A7A7A7A7A7A8A8A8A8A8"
      "A9A9A9A9AAAAAAAAAAABABABABABACACACACADADADADADAEAEAEAEAEAFAFAFAFB0B0B0B0B0B1B1B1"
      "B1B1B2B2B2B2B2B3B3B3B3B3B4B4B4B4B4B5B5B5B5B5B6B6B6B6B7B7B7B7B7B8B8B8B8B8B8B9B9B9"
      "B9B9BABABABABABBBBBBBBBBBCBCBCBCBCBDBDBDBDBDBEBEBEBEBEBFBFBFBFBFBFC0C0C0C0C0C1C1"
      "C1C1C1C2C2C2C2C2C2C3C3C3C3C3C4C4C4C4C4C4C5C5C5C5C5C6C6C6C6C6C6C7C7C7C7C7C8C8C8C8"
      "C8C8C9C9C9C9C9C9CACACACACACACBCBCBCBCBCCCCCCCCCCCCCDCDCDCDCDCDCECECECECECECFCFCF"
      "CFCFCFD0D0D0D0D0D0D1D1D1D1D1D1D2D2D2D2D2D2D2D3D3D3D3D3D3D4D4D4D4D4D4D5D5D5D5D5D5"
      "D5D6D6D6D6D6D6D7D7D7D7D7D7D7D8D8D8D8D8D8D8D9D9D9D9D9D9DADADADADADADADBDBDBDBDBDB"
      "DBDCDCDCDCDCDCDCDDDDDDDDDDDDDDDEDEDEDEDEDEDEDEDFDFDFDFDFDFDFE0E0E0E0E0E0E0E0E1E1"
      "E1E1E1E1E1E2E2E2E2E2E2E2E2E3E3E3E3E3E3E3E3E4E4E4E4E4E4E4E4E5E5E5E5E5E5E5E5E6E6E6"
      "E6E6E6E6E6E7E7E7E7E7E7E7E7E7E8E8E8E8E8E8E8E8E9E9E9E9E9E9E9E9E9EAEAEAEAEAEAEAEAEA"
      "EBEBEBEBEBEBEBEBEBECECECECECECECECECECEDEDEDEDEDEDEDEDEDEDEEEEEEEEEEEEEEEEEEEEEF"
      "EFEFEFEFEFEFEFEFEFF0F0F0F0F0F0F0F0F0F0F0F1F1F1F1F1F1F1F1F1F1F1F2F2F2F2F2F2F2F2F2"
      "F2F2F2F3F3F3F3F3F3F3F3F3F3F3F3F4F4F4F4F4F4F4F4F4F4F4F4F5F5F5F5F5F5F5F5F5F5F5F5F5"
      "F6F6F6F6F6F6F6F6F6F6F6F6F6F6F7F7F7F7F7F7F7F7F7F7F7F7F7F7F7F8F8F8F8F8F8F8F8F8F8F8"
      "F8F8F8F8F8F9F9F9F9F9F9F9F9F9F9F9F9F9F9F9F9F9FAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFAFA"
      "FAFBFBFBFBFBFBFBFBFBFBFBFBFBFBFBFBFBFBFBFBFBFCFCFCFCFCFCFCFCFCFCFCFCFCFCFCFCFCFC"
      "FCFCFCFCFCFCFCFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFDFE"
      "FEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFE"
      "FEFEFEFEFEFEFEFEFEFEFEFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
      "FFFFFFFFFFFFFFFF";
#define QFOLDINGEND }
