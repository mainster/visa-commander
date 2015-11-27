#ifndef HWREG_H
#define HWREG_H

#include <stdint.h>
#include <QString>
#include <QObject>
#include <QDebug>


// ------------------------------------------------------------------------
namespace Qt {
enum instCtrl {
   obj_ptr_only,
   obj_new_if_noexist,
};
Q_DECLARE_FLAGS(instCtrls, instCtrl)
}
// ------------------------------------------------------------------------
namespace Dvms {

enum Dvm_M2_Mode {
   DVM2_AD2T,
   DVM2_AC_off = 0x00,
   DVM2_AC_on  = 0x01,
};
Q_DECLARE_FLAGS(Dvm_M2_Modes, Dvm_M2_Mode)

enum Dvm_Mx_Input {
   Input_single = 0x00,
   Input_differential = 0x01
};
Q_DECLARE_FLAGS(Dvm_Mx_Inputs, Dvm_Mx_Input)

enum Dvm_Mx_Rng {
   Range20V    = 0x01,
   Range2V     = 0x02,
   Range200mV  = 0x03,
   RangeOff    = 0x00
};
Q_DECLARE_FLAGS(Dvm_Mx_Rngs, Dvm_Mx_Rng)

enum Dvm_Mode {
   Mode_DC = 0x00,
   Mode_AC = 0x01
};
Q_DECLARE_FLAGS(Dvm_Modes, Dvm_Mode)

enum Dvm_Period {
   Periode_100ms  = 0x00,
   Periode_233ms  = 0x01
};
Q_DECLARE_FLAGS(Dvm_Periods, Dvm_Period)

}
// ------------------------------------------------------------------------
namespace FuGens {

enum noise_typ {
   typ_binary_distributed     = 0x0,
   typ_uniform_distributed    = 0x1,
   typ_gaussioan_distributed  = 0x2,
};
Q_DECLARE_FLAGS(noise_typs, noise_typ)

enum wave_form {
   form_no        = 0x0,
   form_rect      = 0x1,
   form_pos_saw   = 0x2,
   form_neg_saw   = 0x3,
   form_triangle  = 0x4,
   form_sinusod   = 0x5,
   form_dc_out    = 0x6,
   form_arbitrary = 0x7,
   form_pseudo_rand_noise = 0x8,
   form_puls_gen  = 0x9,
};
Q_DECLARE_FLAGS(wave_forms, wave_form)

enum amp_rng {
   rng_0308  = 0x00,
   rng_11321 = 0x01,
   rng_27152 = 0x02,
   rng_1     = 0x03
};
Q_DECLARE_FLAGS(amp_rngs, amp_rng)
}
// ------------------------------------------------------------------------

/* ======================================================================== */
/*                   Polymorphic super class                                */
/* ======================================================================== */
class HwReg {

public:

   HwReg() {;}

   /** Polymorphie funktioniert nur, wenn die beteiligten Klassen über eine
    * gemeinsame Basisklasse "verwandt" sind. Was aber, wenn es in dieser
    * Basisklasse keine "natürliche" Definition der fraglichen Methode gibt?
    *
    * Das = 0 kennzeichnet die Methode als rein virtuell. Das heißt,
    * Polymorphie funktioniert wie bekannt, aber der Versuch, eine Instanz der
    * Klasse Figur zu erzeugen, wird vom Compiler abgewiesen. Es können nur
    * Instanzen solcher Klassen erzeugt werden, welche diese „leere“ Definition
    * mit Leben erfüllen. Eine zusätzliche Sicherheit für den Programmierer.
    */
   //   virtual void setAddr(uint8_t addrInit) = 0;
   virtual quint8 getAddr() = 0;
   virtual void AboutRegister() {
      qDebug() << QString("Address: ").toStdString().c_str() <<
                  QString("0x%1").arg(address, 2, 16, QLatin1Char('0'))
                  .toStdString().c_str() <<
                  QString(" %1byte ").arg(size()).toStdString().c_str() <<
                  QString(QString::number(address)).toStdString().c_str() <<
                  QString(description).toStdString().c_str();
   }
   virtual uint16_t size() { return sizeof(quint8); }
   virtual QByteArray getDataByteArray() { return QByteArray(""); }
   virtual quint64 getData() { return 0x00; }

public slots:

private:
   //   metrics_t metrics;
   static const quint8 address;
   static const QString description;
};

/* ======================================================================== */
/*                   Registers 0x06 ... 0x09                                */
/* ======================================================================== */
class Reg_0609 : public HwReg {

public:

   quint8 getAddr() {  return address;  }
   void AboutRegister() {
      qDebug() << QString("Address: ").toStdString().c_str() <<
                  QString("0x%1").arg(address, 2, 16, QLatin1Char('0'))
                  .toStdString().c_str() <<
                  QString(" %1byte ").arg(size()).toStdString().c_str() <<
                  QString(description).toStdString().c_str();
   }
   uint16_t size() { return sizeof(h0609_RAM_Addr); }
   QByteArray getDataByteArray() {
      return QByteArray::number((uint16_t) h0609_RAM_Addr.dword);
   }
   quint64 getData() { return h0609_RAM_Addr.dword; }

   /* h06..h09
   *
   * 32-Bit register
   * BIT-adressierbar
   */
   typedef union {
      struct {
         uint32_t act_RAM_addr:16, ADDR_after_trig:16;
      } ;
      uint32_t dword;
   } h0609_RAM_Addr_t;

   h0609_RAM_Addr_t h0609_RAM_Addr;
private:
   static const quint8 address;
   static const QString description;
};

/* ======================================================================== */
/*                   Registers 0x0a ... 0x0d                                */
/* ======================================================================== */
class Reg_0a0d : public HwReg {

public:
   quint8 getAddr() {  return address;  }
   void AboutRegister() {
      qDebug() << QString("Address: ").toStdString().c_str() <<
                  QString("0x%1").arg(address, 2, 16, QLatin1Char('0'))
                  .toStdString().c_str() <<
                  QString(" %1byte ").arg(size()).toStdString().c_str() <<
                  QString(description).toStdString().c_str();
   }
   uint16_t size() { return sizeof(h0a0d_trig_ev); }
   QByteArray getDataByteArray() {
      // TODO: Check if cast is correct, maybe should be quint8??!
      return QByteArray::number((uint16_t) h0a0d_trig_ev.dist);
   }
   quint64 getData() { return h0a0d_trig_ev.dist; }

   /* h0a..h0d  trig_ev distance
   *
   * 32-Bit register
   * byte-adressierbar
   */
   typedef union {
      struct {
         uint8_t byte1;
         uint8_t byte2;
         uint8_t byte3;
         uint8_t byte4;
      } ;
      uint32_t dist;
   } h0a0d_trig_ev_t;

   h0a0d_trig_ev_t h0a0d_trig_ev;
private:
   static const quint8 address;
   static const QString description;
};

/* ======================================================================== */
/*                   Registers 0x10 ... 0x17     ADC values, Vplus, Vminu   */
/* ======================================================================== */
class Reg_1017 : public HwReg {

public:

   quint8 getAddr() {  return address;  }
   void AboutRegister() {
      qDebug() << QString("Address: ").toStdString().c_str() <<
                  QString("0x%1").arg(address, 2, 16, QLatin1Char('0'))
                  .toStdString().c_str() <<
                  QString(" %1byte ").arg(size()).toStdString().c_str() <<
                  QString(description).toStdString().c_str();
   }
   uint16_t size() { return sizeof(h1017_adcPwr); }
   //   QByteArray getDataByteArray() {
   //      return QByteArray::number((quint8) h1017_adcPwr.lword);
   //   }
   quint64 getData() { return h1017_adcPwr.lword; }

   /* h10..h17   ADC PWR
    * 64-Bit register
    * BIT-adressierbar
    */
   typedef union {
      /**
       * @brief
       *
       */
      /**
       * @brief
       *
       */
      /*!
       \brief

      */
      struct {
         uint64_t OV_i0:1, OV_u0:1, ADC_Vplus:14;
         uint64_t OV_i1:1, OV_u1:1, ADC_Iplus:14;
         uint64_t OV_i2:1, OV_u2:1, ADC_Vminus:14;
         uint64_t OV_i3:1, OV_u3:1, ADC_Iminus:14;
      } ;
      uint64_t lword;
      /**
    * @brief
    *
    */
      /**
    * @brief
    *
    */
      /*!
    \brief

   */
   } h1017_adcPwr_t;

   h1017_adcPwr_t h1017_adcPwr;

public slots:

private:
   static const quint8 address;
   static const QString description;
};

/* ======================================================================== */
/*                   Registers 0x18 ... 0x1f     ADC values, DVM, Bias      */
/* ======================================================================== */
class Reg_181f : public HwReg {

public:

   quint8 getAddr() {  return address;  }
   void AboutRegister() {
      qDebug() << QString("Address: ").toStdString().c_str() <<
                  QString("0x%1").arg(address, 2, 16, QLatin1Char('0'))
                  .toStdString().c_str() <<
                  QString(" %1byte ").arg(size()).toStdString().c_str() <<
                  QString(description).toStdString().c_str();
   }
   uint16_t size() { return sizeof(h181f_dvm); }

   quint64 getData() { return h181f_dvm.lword; }

   /* h18..h1f   DVM, Bias
 *
 * 64-Bit register
 * BIT-adressierbar
 */
#pragma pack(push, 1)
   /** 15-10-2015    Anordnung per .lword Zugriff stimmt nicht
    * mit den definierten bitfeldern überein.
    * lword = 0x1bd81920007d191c
    * keine hexfolge spiegelt sich im debugger in den Bitfeldern
    * wieder...
    *    (1)   Einführen von pack(push,1) .. pack(pop)
    *          Keine deutliche Änderung
    *    (2)   Anordnung pro WORD in der struct definition gedreht
    *          Jetzt spiegelt sich der hexwert, der über .lword
    *          in das union geschrieben wurde, auch in den Bitfeldern
    *          wieder. Allerdings ist die Serialisierung noch verdreht
    * 0x1bd8 1920 007d 191c   --> ADC_Vbias   0x191c
    *                         --> ADC_avg     0x007d
    *                         --> ADC_gnd     0x1920
    *                         --> ADC_sel_gnd 0x1bd8
    *    (3)   Anordnung MSB <--> LSB in der struct definition vertausch:
    *          Jetzt sieht es so aus wie erwartet:
    * 0x1bdf 1920 007d 191c   --> ADC_Vbias   0x1bdf
    *                         --> ADC_avg     0x1920
    *                         --> ADC_gnd     0x007d
    *                         --> ADC_sel_gnd 0x191c
    */
   typedef union {
      /**
       * @brief
       *
       */
      /**
       * @brief
       *
       */
      /*!
       \brief

      */
      struct {
         //! (1)
         //         uint64_t OV_i4:1, OV_u4:1, ADC_Vbias:14;
         //         uint64_t OV_i5:1, OV_u5:1, ADC_avg:14;
         //         uint64_t OV_i6:1, OV_u6:1, ADC_gnd:14;
         //         uint64_t OV_i7:1, OV_u7:1, ADC_sel_gnd:14;
         //! (2)
         //         uint64_t ADC_Vbias:14,  OV_u4:1, OV_i4:1;
         //         uint64_t ADC_avg:14,    OV_u5:1, OV_i5:1;
         //         uint64_t ADC_gnd:14,    OV_u6:1, OV_i6:1;
         //         uint64_t ADC_sel_gnd:14,OV_u7:1, OV_i7:1;
         //! (3)
         uint64_t ADC_sel_gnd:14,OV_u7:1, OV_i7:1;
         uint64_t ADC_gnd:14,    OV_u6:1, OV_i6:1;
         uint64_t ADC_avg:14,    OV_u5:1, OV_i5:1;
         uint64_t ADC_Vbias:14,  OV_u4:1, OV_i4:1;
      } ; /*cont;*/
      uint64_t lword;
      /**
    * @brief
    *
    */
      /**
    * @brief
    *
    */
      /*!
    \brief

   */
   } h181f_dvm_t;
   h181f_dvm_t h181f_dvm;
#pragma pack(pop)

   QVector<double> getCalc();
   //   operator=(const Reg_181f& right) {
   //      Reg_181f res;
   //      res.metrics = right.metrics;
   //   }

   //   Utype::metrics_t operator =(const Utype::metrics_t &right);
public slots:

private:
   //   HwReg::metrics_t metrics;
   static const quint8 address;
   static const QString description;
};

/* ======================================================================== */
/*                   Registers 0x30 ... 0x37     FUGEN, ampl, offset, duty  */
/* ======================================================================== */
class Reg_3037 : public HwReg {

public:
   quint8 getAddr() {  return address;  }
   void AboutRegister() {
      qDebug() << QString("Address: ").toStdString().c_str() <<
                  QString("0x%1").arg(address, 2, 16, QLatin1Char('0'))
                  .toStdString().c_str() <<
                  QString(" %1byte ").arg(size()).toStdString().c_str() <<
                  QString(description).toStdString().c_str();
   }
   uint16_t size() { return sizeof(h3037_fugen); }

   quint64 getData() { return h3037_fugen.lword; }

   /* h30..h37   FUGEN, ampl, offs, duty, waveform
 *
 * 64-Bit register
 * BIT-adressierbar
 */
#pragma pack(push, 1)
   typedef union {
      struct {
         uint64_t res30:4;
         uint64_t out_ampl:12;
         uint64_t res32:4;
         uint64_t out_offs:12;
         uint64_t out_duty:16;
         uint64_t res36:6;
         uint64_t ampl_rng:2;
         uint64_t res37:2;
         uint64_t noise_type:2;
         uint64_t wavform:4;
      } ;
      uint64_t lword;
   } h3037_fugen_t;
   h3037_fugen_t h3037_fugen;
#pragma pack(pop)

   QVector<double> getCalc();
public slots:

private:
   //   HwReg::metrics_t metrics;
   static const quint8 address;
   static const QString description;
};

/* ======================================================================== */
/*                   Registers 0x38 ... 0x3b     FUGEN, DDS freq            */
/* ======================================================================== */
class Reg_383b : public HwReg {

public:
   quint8 getAddr() {  return address;  }
   void AboutRegister() {
      qDebug() << QString("Address: ").toStdString().c_str() <<
                  QString("0x%1").arg(address, 2, 16, QLatin1Char('0'))
                  .toStdString().c_str() <<
                  QString(" %1byte ").arg(size()).toStdString().c_str() <<
                  QString(description).toStdString().c_str();
   }
   uint16_t size() { return sizeof(h383b_fufreq); }

   quint64 getData() { return h383b_fufreq.dword; }

   /* h38..h3b  DDS output frequency
 *
 * 32-Bit register
 * Word-adressierbar
 */
#pragma pack(push, 1)
   typedef union {
      struct {
         uint32_t freq:32;
      } ;
      uint32_t dword;
   } h383b_fufreq_t;
   h383b_fufreq_t h383b_fufreq;
#pragma pack(pop)

   QVector<double> getCalc();
public slots:

private:
   //   HwReg::metrics_t metrics;
   static const quint8 address;
   static const QString description;
};

/* ======================================================================== */
/*                   Registers 0x80 ... 0x87     Power supplys              */
/* ======================================================================== */
class Reg_8087 : public HwReg {

public:
   quint8 getAddr() {  return address;  }
   void AboutRegister() {
      qDebug() << QString("Address: ").toStdString().c_str() <<
                  QString("0x%1").arg(address, 2, 16, QLatin1Char('0'))
                  .toStdString().c_str() <<
                  QString(" %1byte ").arg(size()).toStdString().c_str() <<
                  QString(description).toStdString().c_str();
   }
   uint16_t size() { return sizeof(h8087_pows); }
   QByteArray getDataByteArray() {
      return QByteArray::number((quint64) h8087_pows.lword);
   }
   quint64 getData() { return h8087_pows.lword; }

   /* h80..h87   PWR_set
    *
    * 64-Bit register
    * BIT-adressierbar
    */
   /** 26-10-2015    Irgendwo verschieben sich 4 bit zwischen der LCD anzeige
 * Power:: und den stream daten, bisher war die anordnung
 *       uint64_t reserved4:4, ILim_minus:12;
 *       uint64_t reserved3:4, Vminus_set:12;
 *       uint64_t reserved2:4, ILim_plus:12;
 *       uint64_t reserved1:4, Vplus_set:12;
 *
 * Ok jetzt läuft Power:: perfekt!!!
 * unterscheidung zwischen setpoint und tatsächlich gemessenen klemmenwerte
 * kommt aber noch 26-0-10-2015
 */

#pragma pack(push, 1)
   typedef union {
      struct {
         uint64_t ILim_minus:12, reserved4:4;
         uint64_t Vminus_set:12, reserved3:4;
         uint64_t ILim_plus:12, reserved2:4;
         uint64_t Vplus_set:12, reserved1:4;
      } ;
      uint64_t lword;
   } h8087_pows_t;
   h8087_pows_t h8087_pows;
#pragma pack(pop)

private:
   static const quint8 address;
   static const QString description;
};

/* ======================================================================== */
/*                   Registers 0x8a ... 0x8b     DVM range                  */
/* ======================================================================== */
class Reg_8a8b : public HwReg {

public:
   quint8 getAddr() {  return address;  }
   void AboutRegister() {
      qDebug() << QString("Address: ").toStdString().c_str() <<
                  QString("0x%1").arg(address, 2, 16, QLatin1Char('0'))
                  .toStdString().c_str() <<
                  QString(" %1byte ").arg(size()).toStdString().c_str() <<
                  QString(description).toStdString().c_str();
   }
   uint16_t size() { return sizeof(h8a8b_rng); }
   QByteArray getDataByteArray() {
      return QByteArray::number((uint16_t) h8a8b_rng.word);
   }
   quint64 getData() { return h8a8b_rng.word; }

   //   enum h8b_M2_Input {
   //      DVM2_AD2T,
   //      DVM2_AC_off = 0x00,
   //      DVM2_AC_on  = 0x01,
   //   };
   //   Q_DECLARE_FLAGS(h8b_M2_Input2, h8b_M2_Input)

   //   enum Dvm_Mx_Input {
   //      Input_single = 0x00,
   //      Input_differential = 0x01
   //   };
   //   Q_DECLARE_FLAGS(Dvm_Mx_Inputs, Dvm_Mx_Input)

   //   enum Dvm_Mx_Rng {
   //      Range20V    = 0x01,
   //      Range2V     = 0x02,
   //      Range200mV  = 0x03,
   //      RangeOff    = 0x00
   //   };
   //   Q_DECLARE_FLAGS(Dvm_Mx_Rngs, Dvm_Mx_Rng)

   /* h8a..h8b   DMM Range
    *
    * 16-Bit register
    * BIT-adressierbar
    */
#pragma pack(push, 1)
   typedef union {
      struct {
         /** 24-10-2015 Hi- and Low Byte swapped */
         uint16_t M1_rng:2, res1:1, M1_inpt:1, res2:4;
         uint16_t M2_rng:2, res3:1, M2_inpt:1, res4:2, AD2_on:1, AD2_T:1;
      } ;
      uint16_t word;
   } h8a8b_rng_t;
#pragma pack(pop)

   h8a8b_rng_t h8a8b_rng;
private:
   static const quint8 address;
   static const QString description;
};

/* ======================================================================== */
/*                   Registers 0xbe ... 0xbf     LED, Vlogic                */
/* ======================================================================== */
class Reg_bebf : public HwReg {

public:
   //   Reg_bebf * getInstance(Qt::instCtrl ctrl = Qt::obj_ptr_only ) {
   //      if ((ctrl == Qt::obj_ptr_only) ||
   //          (instance != 0))
   //         return instance;
   //      if (ctrl == Qt::obj_new_if_noexist) {
   //         if (instance == 0)
   //            instance = new Reg_bebf();
   //            return instance;
   //      }
   //      return 0;
   //   }
   /**
    * @brief
    *
    * @return quint8
    */
   /**
    * @brief
    *
    * @return quint8
    */
   /*!
    \brief

    \return quint8
   */
   quint8 getAddr() {  return address;  }
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   void AboutRegister() {
      qDebug() << QString("Address: ").toStdString().c_str() <<
                  QString("0x%1").arg(address, 2, 16, QLatin1Char('0'))
                  .toStdString().c_str() <<
                  QString(" %1byte ").arg(size()).toStdString().c_str() <<
                  QString(description).toStdString().c_str();
   }
   /**
    * @brief
    *
    * @return uint16_t
    */
   /**
    * @brief
    *
    * @return uint16_t
    */
   /*!
    \brief

    \return uint16_t
   */
   uint16_t size() { return sizeof(hbebf_led); }
   /**
    * @brief
    *
    * @return QByteArray
    */
   /**
    * @brief
    *
    * @return QByteArray
    */
   /*!
    \brief

    \return QByteArray
   */
   QByteArray getDataByteArray() {
      return QByteArray::number((uint16_t) hbebf_led.word);
   }
   /**
    * @brief
    *
    * @return quint64
    */
   /**
    * @brief
    *
    * @return quint64
    */
   /*!
    \brief

    \return quint64
   */
   quint64 getData() { return hbebf_led.word; }

   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   enum LED_duty {
      duty_0   = 0x00,
      duty_063 = 0x01,
      duty_125 = 0x02,
      duty_250 = 0x03,
      duty_500 = 0x04,
      duty_1   = 0x05
   };
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   enum LED_color {
      highRed_lowOff = 0x00,
      highGrn_lowOff = 0x01,
      highRed_lowGrn = 0x02,
      highGrn_lowRed = 0x03
   };
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   enum LED_freq {
      freq_7dot14_Hz = 0x00,
      freq_3dot6_Hz  = 0x01,
      freq_1dot78_Hz = 0x02,
      freq_0dot83_Hz = 0x03,
      freq_57_Hz     = 0x04
   };
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   enum V_LOGIC_ENB {
      Vlogic_off  = 0x00,
      Vlogic_5V_or_3V = 0x01,
   };
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   enum V_LOGIC_LVL {
      V_LEVEL_5V = 0x00,
      V_LEVEL_3V3 = 0x01
   };
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   Q_DECLARE_FLAGS(LED_dutys, LED_duty)
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   Q_DECLARE_FLAGS(LED_colors, LED_color)
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   Q_DECLARE_FLAGS(LED_freqs, LED_freq)
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   Q_DECLARE_FLAGS(V_LOGIC_ENBS, V_LOGIC_ENB)
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   Q_DECLARE_FLAGS(V_LOGIC_LVLS, V_LOGIC_LVL)
   /* hbe..hbf   LED
 *
 * 16-Bit register
 * BIT-adressierbar
 */
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   typedef union {
      /**
       * @brief
       *
       */
      /**
       * @brief
       *
       */
      /*!
       \brief

      */
      struct {
         uint16_t Vlocic_lvl:1, Vlocic_enb:1, reserved:6;
         uint16_t LED_color:2, LED_duty:3, LED_Freq:3;
      } ;
      uint16_t word;
      /**
    * @brief
    *
    */
      /**
    * @brief
    *
    */
      /*!
    \brief

   */
   } hbebf_led_t;

   hbebf_led_t hbebf_led;

private:
   static const quint8 address;
   static const QString description;
   //   static Reg_bebf *instance;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Reg_bebf::LED_dutys)
Q_DECLARE_OPERATORS_FOR_FLAGS(Reg_bebf::LED_colors)
Q_DECLARE_OPERATORS_FOR_FLAGS(Reg_bebf::LED_freqs)
Q_DECLARE_OPERATORS_FOR_FLAGS(Reg_bebf::V_LOGIC_ENBS)
Q_DECLARE_OPERATORS_FOR_FLAGS(Reg_bebf::V_LOGIC_LVLS)
Q_DECLARE_OPERATORS_FOR_FLAGS(FuGens::wave_forms)
Q_DECLARE_OPERATORS_FOR_FLAGS(FuGens::noise_typs)
Q_DECLARE_OPERATORS_FOR_FLAGS(FuGens::amp_rngs)
Q_DECLARE_METATYPE(FuGens::wave_forms)
Q_DECLARE_METATYPE(FuGens::wave_form)
Q_DECLARE_METATYPE(FuGens::noise_typs)
Q_DECLARE_METATYPE(FuGens::noise_typ)
Q_DECLARE_METATYPE(FuGens::amp_rngs)
Q_DECLARE_METATYPE(FuGens::amp_rng)


#endif // HWREG_H
