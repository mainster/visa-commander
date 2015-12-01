#ifndef VISAREG_H
#define VISAREG_H

#include <QObject>
#include <stdint.h>
#include <sstream>
#include <QDataStream>
#include <QFlags>

#include "hwreg.h"
#include "globals.h"
#include "visareg.h"

class Visa;
class Driver;
class IOEdit;
class Calc;


class VisaReg : public QObject {

   Q_OBJECT

private:

public:
   explicit VisaReg(QObject *parent = 0);
   static VisaReg *getInstance(/*QObject *parent*/) {
      if(inst == 0)
         inst = new VisaReg(/*parent*/);
      return inst;
   }
   static VisaReg *getObjectPtr() {
      return inst;
   }
   ~VisaReg();
   /* ======================================================================== */
   /*                    Start of register declaration                         */
   /* ======================================================================== */
   //   static VisaReg* getInstance() {
   //      if (instance == 0)
   //         instance = new VisaReg();
   //      return instance;
   //   }

#define QFOLDINGSTART {
   /* h00..h01   Versions
 *
 * 16-Bit register
 * BIT-adressierbar
 */
   typedef union {
      struct {
         uint16_t FPGA_Vers:12, hwVers:4;
      } ;
      uint16_t word;
   } h0001_vers;

   /* h02..h03    txed_bytes
 *
 * 16-Bit register
 * byte-adressierbar
 */
   typedef union {
      struct {
         uint8_t msb;
         uint8_t lsb;
      } ;
      uint16_t txed;
   } h0203_txed;

   /* h04     v1
 *
 * 8-Bit register
 * BIT-adressierbar
 */
   typedef union {
      struct {
         uint8_t SCOPE_state:3, trig_pres:1, OV_Vpl:1;
         uint8_t OV_Vmi:1, newADC:1, Authoriz:1;
      } ;
      uint8_t byte;
   } h04_state;

   /* h05   Trig_dist */
   typedef uint8_t h05_Trig_dist;

   /* h06..h09
 *
 * 32-Bit register
 * BIT-adressierbar
 */
//   typedef union {
//      struct {
//         uint32_t act_RAM_addr:16, ADDR_after_trig:16;
//      } ;
//      uint32_t dword;
//   } h0609_RAM_Addr;

   /* h0a..h0d  trig_ev distance
 *
 * 32-Bit register
 * byte-adressierbar
 */
//   typedef union {
//      struct {
//         uint8_t byte1;
//         uint8_t byte2;
//         uint8_t byte3;
//         uint8_t byte4;
//      } ;
//      uint32_t dist;
//   } h0a0d_trig_ev;

   /** h0e, h0f */

   /* h10..h17   ADC PWR
 *
 * 64-Bit register
 * BIT-adressierbar
 */
//   typedef union {
//      struct {
//         uint64_t OV_i0:1, OV_u0:1, ADC_Vplus:14;
//         uint64_t OV_i1:1, OV_u1:1, ADC_Iplus:14;
//         uint64_t OV_i2:1, OV_u2:1, ADC_Vminus:14;
//         uint64_t OV_i3:1, OV_u3:1, ADC_Iminus:14;
//      } ; /*cont;*/
//      uint64_t lword;
//   } h1017_adcPwr;

   /* h18..h1f   DVM, Bias
 *
 * 64-Bit register
 * BIT-adressierbar
 */
//   typedef union {
//      struct {
//         uint64_t OV_i4:1, OV_u4:1, ADC_Vbias:14;
//         uint64_t OV_i5:1, OV_u5:1, ADC_avg:14;
//         uint64_t OV_i6:1, OV_u6:1, ADC_gnd:14;
//         uint64_t OV_i7:1, OV_u7:1, ADC_sel_gnd:14;
//      } ; /*cont;*/
//      uint64_t lword;
//   } h181f_dvm;

   /* h20..23     ACDC24
 *
 * 32-Bit register
 * BIT-adressierbar
 */
   typedef union {
      struct {
         uint32_t ACDC_val:24;
         uint32_t reserved:8;
      } ;
      uint32_t dword;
   } h2023_24bit;

   /* h24..27     ACDC24 square
 *
 * 32-Bit register
 * BIT-adressierbar
 */
   typedef union {
      struct {
         uint32_t IV_i8:1, IV_u8:1;
         uint32_t ACDC_val:30;
      } square;
      uint32_t dword;
   } h2427_24bit;

   /* Enum for 0x2b
 */

   enum V_LOG_LIM {
      Current_lim_on  = 0x00,
      Current_lim_off = 0x01
   };
   /* h2b   VLOGIC
 *
 * 8-Bit register
 * BIT-adressierbar
 */

   typedef union {
      struct {
         uint8_t VLOGIC:1, reserved:6, Vlog_min:1;
      } ;
      uint8_t byte;
   } h2b_logic;

   /* h2c..h2f    minmaxScope
 *
 * 32-Bit register
 * byte-adressierbar
 */
   typedef union {
      struct {
         uint8_t ymin_ch1;
         uint8_t ymax_ch1;
         uint8_t ymin_ch2;
         uint8_t ymax_ch2;
      } ;
      uint32_t dword;
   } h2c2f_scope_ys;

   ////////////////////////////////
   //       h30...h7f
   ////////////////////////////////
   /* Enum -- */
   /* h80..h87   PWR_set
 *
 * 64-Bit register
 * BIT-adressierbar
 */
//   typedef union {
//      struct {
//         uint64_t reserved1:4, Vplus_set:12;
//         uint64_t reserved2:4, ILim_plus:12;
//         uint64_t reserved3:4, Vminus_set:12;
//         uint64_t reserved4:4, ILim_minus:12;
//      } ;
//      uint64_t lword;
//   } h8087_pows;


   enum Dvm_Mx_Input {
      DVM1_single_input = 0x00,
      DVM1_differential_input = 0x01
   };

   enum h8a8b_Mx_Rng {
      Range20V = 0x01,
      Range2V = 0x02,
      Range200mV = 0x03
   };
   /* h8a..h8b   DMM Range
 *
 * 16-Bit register
 * BIT-adressierbar
 */
//   typedef union {
//      struct {
//         uint16_t M1_rng:2, reserved1:1, M1_inpt:1, reserved2:4;
//         uint16_t M2_rng:2, reserved3:1, M2_inpt:1, reserved4:2, AD2_on:1, AD2_T:1;
//      } ;
//      uint16_t word;
//   } h8a8b_rng;

   //////////////////////////////////////
   //       h90...h9f   is for PA
   //////////////////////////////////////


   enum LED_duty {
      duty_0   = 0x00,
      duty_063 = 0x01,
      duty_125 = 0x02,
      duty_250 = 0x03,
      duty_500 = 0x04,
      duty_1   = 0x05
   };

   enum LED_color {
      highRed_lowOff = 0x00,
      highGrn_lowOff = 0x01,
      highRed_lowGrn = 0x02,
      highGrn_lowRed = 0x03
   };

   enum LED_freq {
      freq_7dot14_Hz = 0x00,
      freq_3dot6_Hz  = 0x01,
      freq_1dot78_Hz = 0x02,
      freq_0dot83_Hz = 0x03,
      freq_57_Hz     = 0x04
   };

   enum V_LOGIC_ENB {
      Vlogic_off  = 0x00,
      Vlogic_5V_or_3V = 0x01,
   };

   enum V_LOGIC_LVL {
      V_LEVEL_5V = 0x00,
      V_LEVEL_3V3 = 0x01
   };
   /* hbe..hbf   LED
 *
 * 16-Bit register
 * BIT-adressierbar
 */
   typedef union {
      struct {
         uint16_t LED_color:2, LED_duty:3, LED_Freq:3;
         uint16_t Vlocic_level:1, Vlocic_enb:1, reserved:6;
      } ;
      uint16_t word;
   } hbebf_led;

   /* ======================================================================== */
   /*                    End of register declaration                           */
   /* ======================================================================== */

   /**
 * HW_ro_reg structure mirrors every single BIT of all addressable
 * read only registers! Lets call it a read only-register image
 * structure "HW_ro_img"
 */

//   typedef struct {
//      h0001_vers     h0001;
//      h0203_txed     h0203;
//      h04_state      h04;
//      h05_Trig_dist  h05;
//      h0609_RAM_Addr h0609;
//      h0a0d_trig_ev  h0a0d;
//      h1017_adcPwr   h1017;
//      h181f_dvm      h181f;
//      h2023_24bit    h2023;
//      h2427_24bit    h2427;
//      h2b_logic      h2b;
//      h2c2f_scope_ys h2c2f;
//   } HW_ro_img_t;


//   typedef struct {
//      h8087_pows     h8047;
//      h8a8b_rng      h8a8b;
//      hbebf_led      hbebf;
//   } HW_rw_img_t ;

   //   /* Structure of data transfer type A
   // * (write to writable registers) txStream
   // * Holds information about the next stream send to the visascope.
   // */
   //   typedef struct {
   //      QByteArray assembledRaw;

   //      int appendReg () {
   //         return 0;
   //      }
   //   } txStreamA_t;
#define QFOLDINGEND }

#define QFOLDINGSTART {
   Q_DECLARE_FLAGS(LED_dutys,    LED_duty)
   Q_DECLARE_FLAGS(LED_colors,   LED_color)
   Q_DECLARE_FLAGS(LED_freqs,    LED_freq)
   Q_DECLARE_FLAGS(V_LOGIC_ENBs, V_LOGIC_ENB)
   Q_DECLARE_FLAGS(V_LOGIC_LVLs, V_LOGIC_LVL)
   Q_DECLARE_FLAGS(Dvm_Mx_Inputs, Dvm_Mx_Input)
   Q_DECLARE_FLAGS(h8a8b_Mx_Rngs,   h8a8b_Mx_Rng)
#define QFOLDINGEND }

   /** Copy of tolerance modified calibration vector */
   static QVector<double> H;

   Reg_0609 *regActRamTrig;      //< 32-Bit
   Reg_0a0d *regTrigEvDist;      //< 32-Bit
   Reg_1017 *regAdcPwr;       //< 64-Bit
   Reg_181f *regAdcDvm;       //< 64-Bit
   Reg_3037 *regFgenOut;      //< 64-Bit
   Reg_383b *regFgenFreq;        //< 32-Bit
   Reg_6063 *regScopeXctrl;      //< 32-Bit
   Reg_7073 *regCh1PosFil;       //< 32-Bit
   Reg_7477 *regCh2PosFil;       //< 32-Bit
   Reg_8087 *regSetPwr;       //< 64-Bit
   Reg_8a8b *regDvmRngInp;          //< 16-Bit
   Reg_bebf *regLedVlogic;          //< 16-Bit

//   HW_ro_img_t roRegImg;
//   HW_rw_img_t rwRegImg;

   enum RetFormat {
      RetFormat_ASCII,
      RetFormBIN
   };

   enum HwRegRef {
      HwRegRef_clear,      //< This clears the hwRegs object after write
      HwRegRef_hold
   };

   QVector<HwReg*> hwRegs;

   void sortForAddrs( QVector<HwReg *> &data );
   QByteArray assambleStream( QVector<HwReg *> &regsV,
                              bool subdividedStream = 1 );

   QByteArray writeToReg( QVector<HwReg *> &regObj,
                          RetFormat retFormat = RetFormBIN,
                          HwRegRef sourceObj = HwRegRef_hold
                          );

   bool procVisaInfo(QByteArray &raw);
   QVector<uint16_t> convertCalib(QString in, uint8_t nCharPack);

   /** State enums for methode trxVisa(..) */
   enum trxSt{
      trx_NACKed_Overrun,  /** Not acknowledged overrun error */
      trx_running_req,     /** A request action is still active */
      trx_bad_con_handle,  /** Bad connection handle returned by connect */
      trx_multiple_recvrs, /** More than 1 receiver connected to respReq */
      trx_bad_numOfExpRx,  /** Expected number of bytes couldn't be derived */
      trx_success,         /** Call has been successful */
      trx_que_overrun,     /** new trx req although there are queued requests */
      trx_initial,         /** Initial values, could be used for QFlag vars */
   };
   Q_DECLARE_FLAGS(trxSts, trxSt)

   /** typedef for the vector components of a trxVisa(..) request queue */
   typedef struct {
      QByteArray rawReq;
      QString slotName;
      quint32 nBytes;
      /**
    * @brief
    *
    */
      /*!
    \brief

   */
   } trxQue_t;

   static QVector<double> getH();
   static void setH(const QVector<double> &value);

public slots:
   void init_HwRoImg(bool initVal = true);
   void init_HwRwImg(bool initVal = true);

   /* ======================================================================== */
   /*                     Read response slot declarations                      */
   /* ======================================================================== */
   void onRxRespInfoUcCmpl(QByteArray &rxdata);
   void onRxRespTestReadCmpl(QByteArray &rxdata);
   void onRxRespDefaultReadCmpl(QByteArray &rxdata);
   void onReadReqTimeout();
   void onRxRespDvmCmpl(QByteArray &rxdata);
   void onRxRespCalibData(QByteArray &rxdata);
   /* ======================================================================== */

   int reqVisaInfoFromMicro();
   trxSts trxVisa(QByteArray rawReq = "",
                  QString slotName = "",
                  quint32 nBytes = 0,
                  bool queable = false);

   void testWriteReg();
   void testReadReg();
   void reqDvmADC();
   int reqCalibDataFromAtiny(bool onoff = true);
   char *convert(QString in);
   int reqDefaultRegs();
   void beVerbose(QByteArray &rxdata);

   char *convertVisa(QString in);
   void testWriteReg2();
   int reqDefaultOrigRegs();
   void reqInitialRegState(ushort regCnt = 0x9f);

   void onRxRespInitalRegStateCmpl(QByteArray &rxdata);
   void add2TxStream(HwReg *reg);
signals:
   /** After hardware_xxx.dat file processed successfully AND the EEPROM
    * contents also is available, emit calibInfoAvailable() due to signal
    * that the calibration could be processed now
    */
   void calibInfoAvailable();

private slots:

   //   friend class Calc;

protected:

   /** moved to class Calc
   QVector<uint16_t> *eepromRx; */

private:
   /** Declaration of a copy constructor avoids the compiler to auto generate
    * a public one. Declaration of a private copy constructor makes it
    * impossible to use new() for this class */
   //   VisaReg(const VisaReg&);
   //*/
   QTimer *timReqTimeout;
   QByteArray *txStr, *rxStr;
   QVector<trxQue_t> trxQue;
   static VisaReg *inst;
   Visa     *visa;
   Driver   *driver;
   IOEdit   *ioeditL, *ioeditR;
   Calc     *calc;
   QList<QByteArray> initialRegState;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(VisaReg::Dvm_Mx_Inputs)
Q_DECLARE_OPERATORS_FOR_FLAGS(VisaReg::h8a8b_Mx_Rngs)
Q_DECLARE_OPERATORS_FOR_FLAGS(VisaReg::trxSts)

#endif // VISAREG_H
