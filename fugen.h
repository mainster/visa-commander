#ifndef FUGEN_H
#define FUGEN_H

#include <QDockWidget>
#include <QWidget>
#include <QTimer>
#include <QTime>
#include <qmath.h>
#include <stdint.h>
#include <QProcess>

#include "hwreg.h"
#include "globals.h"
#include "eventhdl.h"


namespace Ui {
class FuGen;
}

class FuGen;
class VisaReg;
class Visa;
class IOEdit;
class EventHdl;

/*!
 \brief Class FuGen provides a configuration and setpoint interface to the DDS
 implemented on the VisaScope.

 Technical Data:
      * Direct-Digital-Synthesis
      * Clock-frequency       60MHz
      * Amplitude-Resolution  10bit
      * Frequency-Range       0.1Hz .. 5Mhz
      * Amplitude-Adjustment  50mVpp .. 10Vpp
      * DC-Offset             +-4V max/min
*/
class FuGen : public QDockWidget {

   Q_OBJECT
public:
   QDataStream &operator<<(QDataStream &dstream/*, const ioeditL&cioed*/) {
      dstream << "hallo";
      return dstream;
   }
public:
   enum add_type {
      type_increment,
      type_replace,
   };
   Q_DECLARE_FLAGS(add_types, add_type)

   struct tuple_t {
      double      float_ ;
      uint32_t    reg32_ ;
      uint16_t    reg16_ ;
      int         int___ ;
   };
   typedef struct tuple_t tuple_t;

   /* ======================================================================= */
   /*                  Fu-Gen config structure                                */
   /* ======================================================================= */
   struct genCfg_t {
      tuple_t Freq;
      tuple_t Amp;
      tuple_t Offs;
      tuple_t Duty;
      FuGens::wave_forms   waveForm;
      FuGens::noise_typs   noiseModel;
      FuGens::amp_rngs     ampRng;

      // TODO: Calculate real calibrated scale factors


      /*!
       \brief Convert the LCD setpoint values to the corresponding integer
       values and write it to the stream generation vector.

       \param dAmp
       \param dFreq
       \param dOffs
       \param dDuty
       \param type
       \return int
      */
      ///< --------------- convert Amplitude -------------------------------
      long convLCD2amp(double dVamp, add_type type = FuGen::type_increment);

      ///< --------------- convert frequency -------------------------------
      long convLCD2freq(double dFreq, add_type type = FuGen::type_increment);

      ///< --------------- convert Offset ----------------------------------
      long convLCD2offs(double dVoffs, add_type type = FuGen::type_increment);

      ///< --------------- convert Duty-Cycle ------------------------------
      long convLCD2duty(double dDuty, add_type type = FuGen::type_increment);


      /*!
       \brief Check range of freq, offset and ampl values, truncate if
       necessary. Also deploy this values to int or float representations

      */
      void rngCheckToFloat() {
      }

#define QFOLDINGSTART {
//      void AmpSet( double val, add_type type = FuGen::type_increment) {
//         FuGens::amp_rngs rng = calcAmp_rng();

//         if (type == FuGen::type_increment)
//            Freq.int_ += (int) val / HZ_PER_DIG;
//         else
//            Freq.int_ = (uint) val / HZ_PER_DIG;

//         rngCheckToFloat();
//      }
//      void AmpSet( double val, add_type type = FuGen::type_increment) {
//         if (type == FuGen::type_increment)
//            Amp.int_ += (int) val / V_AMP_PER_DIG;
//         else
//            Amp.int_ = (int) val / V_AMP_PER_DIG;

//         rngCheckToFloat();
//      }
//      void FreqSet( double val, add_type type = FuGen::type_increment) {
//         if (type == FuGen::type_increment)
//            Freq.int_ += (int) val;
//         else
//            Freq.int_ = (uint) abs( val );

//         rngCheckToFloat();
//      }
//      void AmpSet( int val, add_type type = FuGen::type_increment) {
//         if (type == FuGen::type_increment)
//            Amp.int_ += (int) val;
//         else
//            Amp.int_ = (uint) abs( val );

//         rngCheckToFloat();
//      }
//      void OffsSet( int val, add_type type = FuGen::type_increment) {
//         if (type == FuGen::type_increment)
//            Offs.int_ += (int) val;
//         else
//            Offs.int_ = (uint) abs( val );

//         rngCheckToFloat();
//      }
//      void OffsSet( double val, add_type type = FuGen::type_increment) {
//         if (type == FuGen::type_increment)
//            Offs.int_ += (int) val / V_OFFS_PER_DIG;
//         else
//            Offs.int_ = (int) val / V_OFFS_PER_DIG;

//         rngCheckToFloat();
//      }
//      void DutySet( int val, add_type type = FuGen::type_increment) {
//         if (type == FuGen::type_increment)
//            Duty.int_ += (int) val;
//         else
//            Duty.int_ = (uint) abs( val );

//         rngCheckToFloat();
//      }
//      void DutySet( double val, add_type type = FuGen::type_increment) {
//         if (type == FuGen::type_increment)
//            Duty.int_ += (int) val / V_OFFS_PER_DIG;
//         else
//            Duty.int_ = (int) val / V_OFFS_PER_DIG;

//         rngCheckToFloat();
//      }



//      void rngCheckToFloat() {
//         if (Vplus.int_.set >= V_MAX_INT)
//            Vplus.int_.set = V_MAX_INT;
//         if (Vplus.int_.set < 0)
//            Vplus.int_.set = 0;

//         if (Vmin.int_.set >= V_MIN_INT)  //!!!!!
//            Vmin.int_.set = V_MIN_INT;
//         if (Vmin.int_.set < 0)
//            Vmin.int_.set = 0;

//         if (Iplus.int_.set >= I_MAX_INT)
//            Iplus.int_.set = I_MAX_INT;
//         if (Iplus.int_.set < 0)
//            Iplus.int_.set = 0;

//         if (Imin.int_.set >= I_MIN_INT)
//            Imin.int_.set = I_MIN_INT;
//         if (Imin.int_.set < 0)
//            Imin.int_.set = 0;


//         qDebug() << (double) Vplus.int_.set * V_PER_DIG;
//         qDebug() << Vplus.int_.set * V_PER_DIG;
//         Vplus.float_.set = (double) Vplus.int_.set * V_PER_DIG;
//         Vmin.float_.set  = (double) Vmin.int_.set * V_PER_DIG;
//         Iplus.float_.set = (double) Iplus.int_.set * A_PER_DIG;
//         Imin.float_.set  = (double) Imin.int_.set * A_PER_DIG;
//      }
#define QFOLDINGEND }

//      QString getsFloat(tuple_t ret) {
//         return QString("%1").arg(ret.float_);
//      }
//      QString getsIntRead(tuple_t ret) {
//         return QString("%1").arg(ret.int_);
//      }

   };
   typedef struct genCfg_t genCfg_t;
   genCfg_t *genCfg;

   /* ======================================================================== */
   /*                     class constructor                                    */
   /* ======================================================================== */
   explicit FuGen(QWidget *parent = 0);
   static FuGen *getInstance(QWidget *parent = 0) {
      if(inst == 0)
         inst = new FuGen(parent);
      return inst;
   }
   static FuGen *getObjectPtr() {
      return inst;
   }
   ~FuGen();

   void initUiElements();
   uint16_t calcAmplRegVal(double physAmp);
   uint16_t calcOffsRegVal(double physOffs);
   static double Kx(FuGens::amp_rngs fRng);

   QLCDNumber *getLcdFreqObj() const;
   QLCDNumber *getLcdAmpObj() const;
   QLCDNumber *getLcdOffsObj() const;
   QLCDNumber *getLcdDutyObj() const;
   QList<QLCDNumber *> getLcdObjs() const;

public slots:
   void onCyclic();
   void onBtnOnOffClicked();
   void onConfigChangeTriggered(int idx = -1);
   void loadSineIntoFPGA();
   static FuGens::amp_rngs calcAmp_rng();


protected:
   void wheelEvent(QWheelEvent *event);
   bool eventFilter(QObject *obj, QEvent *ev);

private:
   Ui::FuGen   *ui;
   QTimer      *timCycl;
   IOEdit      *ioeditL, *ioeditR;
   Visa        *visa;
   EventHdl    *evHdl;
//   QLCDNumber  *fgLcdFreq, *fgLcdAmp, *fgLcdOffs, *fgLcdDuty;
   int         mouseWheelCnt;

   double
   DVAMP_PER_TICK,            //< ~53.3 mV/TickV
   DFOUT_PER_TICK;            //< ~100Hz / Tick
   static VisaReg *vr;
   static FuGen *inst;
   static const QByteArray sinTbl;

   static const double
   INTERVAL_LCD      = 0.1,         //< s

   /**
    * Output amplifier limits
    * ---------------------------------------
    * Output amplifier:       Vout_max = +8V
    *                         Vout_min = -8V
    *
    * Max. Output Voltage:    Vpp_max  = 10V
    * swing, all waveforms
    * (except DC-out):
    *
    * Offset Voltage:         Vos_max  = +4V
    *                         Vos_min  = -4V
    *
    * Thus, for a amplitude of Vpp = 10V, the maximum Offset
    * Voltage is Vos = +-3V, since the output voltage swing is
    * limited to Vout = +-8V.
    *
    */
   VAMP_MIN          = 0.05,        //< Vpp
   VAMP_MAX          = 8.0,         //< Vpp
   VAMP_MAX_TICKS    = 150,         //< ~150 ticks @
                                    //< range: 0...8Vpp

   /**
    * DDS output frequency limits
    * --------------------------------------
    * Output frequency: 0.1Hz ... 5MHz
    * delta-f-out: DFOUT_PER_TICK = 100Hz / Tick
    *
    */
   FOUT_MIN          =  0.1,        //< Hz
   FOUT_MAX          =  5.0e6,      //< Hz
   FOUT_MAX_TICKS    =  50e3,

   /**
    * Alternating output frequency
    * --------------------------------------
    * DDS_Increment (32-bit uint) @ 0x38...0x3b
    * DDS_Increment  = 2^32/f_clk * fout
    *                = 71.582788... * fout
    *                = fcFreq * fout
    */
   FACT_FREQ         =  (0xffffffff/60e6),

   /**
    * Duty cycle for rectangular wave form
    * --------------------------------------
    * Duty_Cycle (16-bit uint) @ 0x34...0x35
    * duty_ in % =>  duty_ is an element of [0,100]
    *
    * Duty_Cycle  =  655.35 * duty_
    *             =  fcDuty * duty_
    */
   DUTY_MIN_PERC     =  0.0,
   DUTY_MAX_PERC     =  100.0,
   FACT_DUTY         =  655.35,

   /**
    * Signal generator offset voltage
    * --------------------------------------
    * Voffs = +- 4V but Vp must still stay smaler
    * than Vpp(8V)+Voffs_max(4V)
    */
   VOFFS_MIN         =  -4.0,       //< V
   VOFFS_MAX         =  4.0;        //< V

//   Vout_max =   8.0,
//   Vout_min =  -8.0,
//   Vpp_max  =  10.0;
};

//Q_DECLARE_METATYPE()

#endif // FUGEN_H
