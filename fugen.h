#ifndef FUGEN_H
#define FUGEN_H

#include <QDockWidget>
#include <QWidget>
#include <QTimer>
#include <QTime>
#include <qmath.h>
#include <stdint.h>

#include "visareg.h"
#include "visa.h"
#include "hwreg.h"
#include "mqtimer.h"


namespace Ui {
class FuGen;
}

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
   enum add_type {
      type_increment,
      type_replace,
   };
   Q_DECLARE_FLAGS(add_types, add_type)

   typedef struct {
      double   float_ ;
      int      int_ ;
      uint     reg_ ;
   } tuple_t;

   /* ======================================================================= */
   /*                  Fu-Gen config structure                                */
   /* ======================================================================= */

   typedef struct {
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
      ///< -----------------------------------------------------------------
      int convLCD2amp(double dAmp, add_type type = FuGen::type_increment) {
         FuGens::amp_rngs rng = FuGens::calcAmp_rng();

         if (type == FuGen::type_increment)
            Amp.float_  += dAmp;
         else Amp.float_ = dAmp;

         Amp.int_    = (uint16_t)((double) 4095 *
                               dAmp/((double)vr->H[31] * Kx( rng )));

         /** Range check according to page 56, equ. (12.3) */
         if (! ((Amp.int_ > 0) && (Amp.int_ < LIM_12BIT_UINT)))
            Amp.int_ = -1;

         return 0;
      }
      ///< --------------- convert Offset ----------------------------------
      ///< -----------------------------------------------------------------
      int convLCD2offs(double dOffs, add_type type = FuGen::type_increment) {
         FuGens::amp_rngs rng = calcAmp_rng();

         if (type == FuGen::type_increment)
            Offs.float_  += dOffs;
         else Offs.float_ = dOffs;

         Offs.int_   = (uint16_t) (2047 * ((double) 1.0 + vr->H[32] *
                                   Offs.float_/Kx( rng )) + vr->H[30]);

         /** Range check according to page 56, equ. (12.4) */
         if (! ((Offs.int_ > 410) && (Offs.int_ < 3685)))
            Offs.int_ = -1;

         return 0;
      }
      ///< --------------- convert Duty-Cycle ------------------------------
      ///< -----------------------------------------------------------------
      int convLCD2duty(double dDuty, add_type type = FuGen::type_increment) {
         FuGens::amp_rngs rng = calcAmp_rng();

         if (type == FuGen::type_increment)
            Duty.float_  += dDuty;
         else Duty.float_ = dDuty;

         if (dDuty < 100.0)
            Duty.int_   = (uint16_t) ((double)fcDuty * dDuty);
         else
            Duty.int_   = 0xffff;

         /** Range check according to page 56, equ. (12.3)
         if (! ((Amp.int_ > 0) && (Amp.int_ < LIM_12BIT_UINT)))
            Amp.int_ = -1;*/

         return 0;
      }
      ///< --------------- convert frequency -------------------------------
      ///< -----------------------------------------------------------------
      int convLCD2freq(double dFreq, add_type type = FuGen::type_increment) {
         FuGens::amp_rngs rng = calcAmp_rng();

         if (type == FuGen::type_increment)
            Freq.float_  += dFreq;
         else Freq.float_ = dFreq;

         if (dFrq < 5.0e6)
            Freq.int_   = (uint16_t) ((double)fcFrq * dFreq);
         else {
            Freq.int_   = 0xffff;
            Freq.float_ = 5e6;
         }

         /** Range check according to page 56, equ. (12.3)
         if (! ((Amp.int_ > 0) && (Amp.int_ < LIM_12BIT_UINT)))
            Amp.int_ = -1;*/

         return 0;
      }

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

   }  genCfg_t;


   genCfg_t *genCfg;
   explicit FuGen(QWidget *parent = 0);
   static FuGen *getInstance() {
      if(inst == 0)
         inst = new FuGen();
      return inst;
   }
   static FuGen *getObjectPtr() {
      return inst;
   }
   ~FuGen();

   void initUiElements();
   FuGens::amp_rngs calcAmp_rng();
   double Kx(FuGens::amp_rngs fRng);
   uint16_t calcAmplRegVal(double physAmp);
   uint16_t calcOffsRegVal(double physOffs);
public slots:
   void onCyclic();
   void wheelEvent(QWheelEvent *event);
   void fillTxReg();

   void onBtnOnOffClicked();
   void onConfigChangeTriggered(int idx = -1);
   void loadSineIntoFPGA();
private:
   Ui::FuGen *ui;
   static FuGen *inst;
   QTimer      *timCycl;
   int         mouseWheelCnt;
   static VisaReg     *vr;
   IOEdit      *ioedit ;
   Visa        *visa;

   static const QByteArray sinTbl;

   static const double
   INTERVAL_LCD      = 0.1,         //< s
   AMP_MIN           = 0.05,        //< Vpp
   AMP_MAX           = 8.0,         //< Vpp
   HZ_PER_DIG        = 0.01397,     //< Hz

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
   Vout_max =   8.0,
   Vout_min =  -8.0,
   Vpp_max  =  10.0,
   Vos_max  =   4.0,
   Vos_min  =  -4.0,
   fcDuty   = 655.35,
   fcFreq   = (0xffffffff/60e6),         //< 2^32/fclk = 71.58....

   V_PER_DIG         = 6.10352e-05, //< V
   V_AMP_PER_DIG     = 0.0,         //< V
   V_OFFS_PER_DIG    = 0.0,         //< V
   V_AMP_MAX_FLOAT   = 12.5l,       //< V
   V_OFFS_MAX_FLOAT  = 12.5l;       //< V
};

//Q_DECLARE_METATYPE()

#endif // FUGEN_H
