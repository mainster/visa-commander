#ifndef POWER_H
#define POWER_H

#include <QWidget>
#include <QTimer>
#include <QTime>

#include "mqtimer.h"

namespace Ui {
class Power;
}

class VisaReg;
class IOEdit;
class Visa;

//const long Power::V_PER_DIG = 0.0030940594059405;
//const long Power::supply_t::A_PER_DIG = 0.0002442002442002;

class Power : public QWidget {

   Q_OBJECT

public:

   enum add_type {
      type_increment,
      type_replace,
   };
   Q_DECLARE_FLAGS(add_types, add_type)

   typedef struct {
      double set, read;
   } _float_;

   typedef struct {
      int set, read;
   } _int_;

   typedef struct {
      _float_  float_ ;
      _int_    int_ ;
   } tuple_t;

   /* ======================================================================= */
   /*                  Power supply config structure                          */
   /* ======================================================================= */
   typedef struct {
      tuple_t Vplus;
      tuple_t Vmin;
      tuple_t Iplus;
      tuple_t Imin;
      bool LimPlus;
      bool LimMin;

      void VplusSet( int val, add_type type = Power::type_increment) {
         if (type == Power::type_increment)
            Vplus.int_.set += (int) val;
         else
            Vplus.int_.set = (uint) abs( val );

         rngCheckToFloat();
      }
      void VplusSet( double val, add_type type = Power::type_increment) {
         if (type == Power::type_increment)
            Vplus.int_.set += (int) val / V_PER_DIG;
         else
            Vplus.int_.set = (int) val / V_PER_DIG;

         rngCheckToFloat();
      }
      void VminSet( int val, add_type type = Power::type_increment) {
         if (type == Power::type_increment)
            Vmin.int_.set += (int) val;
         else
            Vmin.int_.set = (int) val;

         rngCheckToFloat();
      }
      void VminSet( double val, add_type type = Power::type_increment) {
         if (type == Power::type_increment)
            Vmin.int_.set += (int) val / V_PER_DIG;
         else
            Vmin.int_.set = (int) val / V_PER_DIG;

         rngCheckToFloat();
      }

      void IplusSet( int val, add_type type = Power::type_increment) {
         if (type == Power::type_increment)
            Iplus.int_.set += (int) val;
         else
            Iplus.int_.set = (uint) abs( val );

         rngCheckToFloat();
      }
      void IplusSet( double val, add_type type = Power::type_increment) {
         if (type == Power::type_increment)
            Iplus.int_.set += ((int) val / A_PER_DIG);
         else
            Iplus.int_.set = (int) val / A_PER_DIG;

         rngCheckToFloat();
      }
      void IminSet( int val, add_type type = Power::type_increment) {
         if (type == Power::type_increment)
            Imin.int_.set += (int) val;
         else
            Imin.int_.set = (int) val;

         rngCheckToFloat();
      }
      void IminSet( double val, add_type type = Power::type_increment) {
         if (type == Power::type_increment)
            Imin.int_.set += (int) val / A_PER_DIG;
         else
            Imin.int_.set = (int) val / A_PER_DIG;

         rngCheckToFloat();
      }

      void rngCheckToFloat() {
         if (Vplus.int_.set >= V_MAX_INT)
            Vplus.int_.set = V_MAX_INT;
         if (Vplus.int_.set < 0)
            Vplus.int_.set = 0;

         if (Vmin.int_.set >= Power::V_MIN_INT)  //!!!!!
            Vmin.int_.set = V_MIN_INT;
         if (Vmin.int_.set < 0)
            Vmin.int_.set = 0;

         if (Iplus.int_.set >= I_MAX_INT)
            Iplus.int_.set = I_MAX_INT;
         if (Iplus.int_.set < 0)
            Iplus.int_.set = 0;

         if (Imin.int_.set >= I_MIN_INT)
            Imin.int_.set = I_MIN_INT;
         if (Imin.int_.set < 0)
            Imin.int_.set = 0;


         qDebug() << (double) Vplus.int_.set * V_PER_DIG;
         qDebug() << Vplus.int_.set * V_PER_DIG;
         Vplus.float_.set = (double) Vplus.int_.set * V_PER_DIG;
         Vmin.float_.set  = (double) Vmin.int_.set * V_PER_DIG;
         Iplus.float_.set = (double) Iplus.int_.set * A_PER_DIG;
         Imin.float_.set  = (double) Imin.int_.set * A_PER_DIG;
      }

      QString getsFloatRead(tuple_t ret) {
         return QString("%1").arg(ret.float_.read);
      }
      QString getsFloatSetpoint(tuple_t ret) {
         return QString("%1").arg(ret.float_.set);
      }
      QString getsIntRead(tuple_t ret) {
         return QString("%1").arg(ret.int_.read);
      }
      QString getsIntSetpoint(tuple_t ret) {
         return QString("%1").arg(ret.int_.set);
      }

   }  supply_t;

   supply_t *supply;
   explicit Power(QWidget *parent = 0);
   static Power *getInstance(QWidget *parent/* = 0*/) {
      if(inst == 0)
         inst = new Power(parent);
      return inst;
   }
   static Power *getObjectPtr() {
      return inst;
   }

   ~Power();

public slots:
   void onCyclic();
   void onBtnLockClicked();
   void onBtnEnPosClicked();
   void onBtnEnNegClicked();
   void wheelEvent(QWheelEvent *event);
   void onAppFocusChanged(QWidget* old, QWidget* now);
   void onfocGuard();
protected:

private:
   static const double
   INTERVAL_LCD   = 100e-3;

   static const double
   V_PER_DIG      = 0.00310136,
   A_PER_DIG      = 6.10352e-05,
   V_MAX_FLOAT    = 12.5,
   V_MIN_FLOAT    = 12.5,
   I_MAX_FLOAT    = 0.15,
   I_MIN_FLOAT    = 0.15;

   static const int
   V_MAX_INT      = 4095,
   V_MIN_INT      = 4096,
   MAX_INT        = 0x0fff,
   I_MAX_INT      = 4095,
   I_MIN_INT      = 4096;

   Ui::Power   *ui;
   static Power   *inst;
   QTimer      *timCycl;
   int         mouseWheelCnt;
   VisaReg     *vr;
   IOEdit      *ioeditL;
   Visa        *visa;
   MQTimer_t   *autoLock, *focGuard;
   QStringList focusOkObj;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Power::add_types)

#endif // POWER_H
