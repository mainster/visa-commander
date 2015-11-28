#ifndef CALC_H
#define CALC_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QDebug>
#include <QFlag>
#include <QFlags>
#include <stdint.h>
#include <qmath.h>
#include <QString>

#include "ioedit.h"
#include "visa.h"
#include "visareg.h"


class Calc : public QObject {

   Q_OBJECT

public:

   enum tolType {
      unspecified = 0x00,
      absolut,
      relativ,
      none
   };

   /**
    * typedef: Container base to hold all necessary informations that are
    * needed for calibration purposes. ( hardware_xxxx.dat file, EEPROM )
    */
   typedef struct {
      float    Hd;      //< Hardware value from hardware_xxx.dat  
      uint16_t  C;      //< Calibration value from ATtiny EEPROM  
      QString  note;    //< Type of hw value + tolerance, as string  
      tolType  type;    //< Enum represents absolut, relative or none  
      float    T;       //< Scale factor for calibration  
      float    H;       //< Corrected hw value for later calculation  
   } hw_calib_t;
   Q_DECLARE_FLAGS(tolTypes, tolType)

   static Calc *getInstance(QWidget *visaregPtr = 0) {
      if(inst == 0)
         inst = new Calc(visaregPtr);
      return inst;
   }

   static Calc *getObjectPtr() {
      return inst;
   }

   explicit Calc(QWidget *parent = 0/*, VisaReg *otherparent = 0*/);

   ~Calc();

   /** Calibration and correction vector
    * for the later processed calculation task from integral data (uint) to
    * the physical expression
    */
   QVector<hw_calib_t> cCalib;  

   QStringList hwNotes;  

   /** ATtiny EEPROM storage
    * This vector is used to store the retreived calibration data from
    * ATtiny EEPROM read out
    */
   QVector<uint16_t> *eepromRx;  

signals:

public slots:

   void onCalibInfoAvailable();
   int mathAdjustment(QVector<hw_calib_t> &hwCal,
                        QVector<uint16_t> &Ctol);
   int loadHardwareDat(QString path);
   void loadHardwareDat();
   int physCalcADC();

protected:
   friend class VisaReg;
private slots:

private:
   static Calc * inst;  
   Visa        * visa;
   VisaReg     * visareg;
   IOEdit      *ioeditL, *ioeditR;


//   QLocale     german;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Calc::tolTypes)


#endif // CALC_H
