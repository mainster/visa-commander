
#ifndef DVM_H
#define DVM_H
#include "globals.h"
#include "ioedit.h"
#include "driver.h"
#include "visareg.h"
#include "visa.h"
#include <QDockWidget>
#include <QGroupBox>
#include "calc.h"

namespace Ui {
class Dvm;
}
class Visa;
class Calc;
class Driver;

/*_________________________________
  //////////////////////////////////
////////////////////////////////////*/
/*///////*/ class VisaReg; /*///////*/
//////////////////////////////////////
//////////////////////////////////

/**
 * This enum needs to be defined outside the class definition because we need
 * it to specify wether a DC or ACDC instance of the class Dvm:: is requested
 */
enum DvmType {
   dvmType_DC,
   dvmType_ACDC,
   dvmType_MIX
};
Q_DECLARE_FLAGS(DvmTypes, DvmType)

Q_DECLARE_OPERATORS_FOR_FLAGS(DvmTypes)


class Dvm : public QDockWidget {

   Q_OBJECT

//   enum Dvm_Mx_Input {
//      Input_single       = 0x00,
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

//   enum Dvm_Mode {
//      Mode_DC,
//      Mode_AC
//   };
//   Q_DECLARE_FLAGS(Dvm_Modes, Dvm_Mode)


/* ---------------------------------------------------------------- */

   typedef struct {
      Dvms::Dvm_Mx_Inputs  inpSel;
      Dvms::Dvm_Mx_Rngs   selRng;
      Dvms::Dvm_Modes      mode;
      Dvms::Dvm_Periods    periode;
      bool           enabled;
      bool           ovRng;
   } dvm_config_t;


public:
   DvmTypes       dvmHasType;
   dvm_config_t   dvmCfg_DC, dvmCfg_ACDC;
   dvm_config_t   *dvmConf;
   QTimer         *refTim;
   QVector<HwReg*> hwRegs;

public:
   explicit Dvm(const DvmTypes type, QWidget *parent = 0);
   ~Dvm();

   /** 16-10-2015    Make Dvm multi-instanceable for DvmDC and DvmACDC */
   static Dvm *getInstance(DvmTypes type/*, QWidget *parent = 0*/) {
      if (type == dvmType_DC) {
         if(instDc == 0) {
            instDc = new Dvm( dvmType_DC ); //, 0, parent );
         }
         return instDc;
      }

      if (type == dvmType_ACDC) {
         if(instAcDc == 0)
            instAcDc = new Dvm( dvmType_ACDC ); //, 0, parent );
         return instAcDc;
      }
      return NULL;
   }
   static Dvm *getObjectPtr(DvmTypes type) {
      if (type == dvmType_DC)
         return instDc;
      if (type == dvmType_ACDC)
         return instAcDc;
      return NULL;
   }
   void connectSignalSlots();
   void dvmTxConfigUpdate2();
   double getLCDval() const;
   void setLCDval(double value);
   void txDvmConfigACORDC();

public slots:
   void onBtnConnectorsClicked();
   void onLcdRefresh(void);
   void onBtnDvmAClicked();
   void onBtnDvmBClicked();
   void onRbxxClicked();
   void onBtnReadRegsClicked();
   void onLcdRefreshPeriReturnPress();
   void childsToDbg();
   void updateDvmUi();
   void onBtnCfgRecClicked();
   void onSlRngChanged(int val);
   void txDvmConfig();
   void onBtnOnOffClicked(bool onoff);
   void onRbModeSel();
   void deployConfigs(dvm_config_t *cfg);
   void deployConfigs();

private:
   Ui::Dvm     *ui;
   bool        waitForDvmUi;
   QTimer      *timMisc;
   double      LCDval;
   VisaReg     *vr;
   Visa        *visa;
   Calc        *calc;
   static Dvm  *instDc, *instAcDc/*, *instMix*/;
   Driver      *driver;
};


#endif // DVM_H
