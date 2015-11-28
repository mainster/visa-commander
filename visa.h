#ifndef VISA_H
#define VISA_H

#include <QMainWindow>
#include <QStringListModel>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QTableWidget>

#include "globals.h"
#include "portdialog.h"
#include "mqtimer.h"

namespace Ui {
class Visa;
}

class VisaReg;
class Register;
class IOEdit;
class FuGen;
class Calc;
class Power;
class Reg_bebf;
class Driver;
class Dvm;

/** Time division factor for some debug prints */
#define HEARTBEAT_DIVIDER  int(1000 / 250)

/** Default format for line edit le8a8b */
//Qt::formateds Visa::le8a8bForm = Qt::form_hex;

class Visa : public QMainWindow {

   Q_OBJECT

#define QFOLDINGSTART {
   /**
    * @brief The HTMLcode struct
    * Struct of HTML codes used for text coloring
    */

   struct HTMLcode {
      QString
      RED,
      GRAY,
      BLUE,
      HEAD_BGCOL,
      DEFAULT;
   };
   static const HTMLcode html;

   /** Visa structures listing */

   /**
    * @brief The infoFromVisascope struct
    * Some infos read back from visa scope registers
    */

   struct infoFromVisascope {
      QString
      uC_FW,
      FPGA_FW,
      Hw_rev,
      Vcom,
      TESTBYTESrx;
      int SerialNo;
      quint8    BaudrateCode;
      double  BiasVoltage;
      int     Scope_State;
      bool    Trig_presc;
   };
   //   static infoFromVisascope visaInfo;

   /**
    * @brief The visaSeqences struct
    * const struct that holds the different visascope uart sequences
    */

   struct visaSeqences {
      QString
      STARTSEQ,
      STDBY,
      STDBYSML,
      STDBY_POWERON,
      STDBY_SCOPE,
      STDBY_SCOPE_AUTOTRIG,
      SCOPEON,
      VDCON,
      VDCOFF,
      VADCON,
      VADCOFF,
      FUGENON,
      FUGENOFF,
      POSPOWEREN;
   };
   static const visaSeqences vseq;

   /**
    * @brief The vsRamRequest struct
    * Holds information about prior configured registers.
    * Holds some values that are gathered via register reads, the values are
    * needed to generate tx stream for requesting RAM read out
    */

   struct ramReqInfo {
      quint8
      RAM_Size,        /**< 3-Bit value that contains encoded size of RAM bytes */
      num_act_chan,    /**< Number of active channels */
      act_channels,    /**< The active channels */
      ptr_addr;        /**< RAM read out pointer address */

      uint16_t
      act_RAM_addr,    /**< actual RAM address at the time where SCOPE_State 4 */
      num_req_byte;    /**< number of bytes to request, calced */
   };
   ramReqInfo reqInfo;
#define QFOLDINGEND }

public:
   /**
    * @brief The CFG_LED enum
    * Enum for config selection of the Duo-LED
    */
   enum CFG_LED {
      MDB_accessedDevice,
      MDB_leftDevice
   };

   enum formated {
      form_dec = 10,
      form_hex = 16,
      form_bin = 2
   };
   Q_DECLARE_FLAGS(formateds, formated)

   /**
    * Global state structure
    */
   struct globalState {
      bool        reReqRamDataf;           ///< boolean to control if another reqRam seq should be transmitted
      int         reqRamCtr;
      QByteArray  rseqA, rseqB;
      enum CFG_LED    ledState;
      int         txInStdbyCtr;
      bool        activeRxTimeout;
      bool        blockOnReadyRead;
      int         numReadlastTransmission;    ///< Holds the true count of received bytes
      QByteArray  differIdx;
      bool        errorHandleractive;
      bool        hw_xxx_dat_loaded;
      bool        calibrated;
      int         hbeatsCtr;
      bool        powerCfgAltered;
      bool        fuGenCfgAltered;
   };
   globalState gs;

   /**
    * Hardware dependent factors read from hardware_197000.dat
    * Necessary for calculating the physical expressions of integer values
    */
   QVector<QStringList> hDat;

   enum visaLED {
      led_gn = 0x01,
      led_rd = 0x02,
      led_flash_gnrd = 0x04,
      led_flash_gn = 0x08,
      led_flash_rd = 0x10,
   };
   Q_DECLARE_FLAGS(LEDs, visaLED)

   static infoFromVisascope visaInfo;
   static QTableWidget *pTw;

   explicit Visa(QWidget *parent = 0);
   static Visa *getInstance(QWidget *parent/* = 0*/) {
      if(inst == 0)
         inst = new Visa(parent);
      return inst;
   }
   static Visa *getObjectPtr() {
      return inst;
   }
   ~Visa();

   static const int MAX_CONSOLE_CHARS;

   void cfgVisaLED(enum CFG_LED state);
   char *convert(QString in);
   QVector<uint16_t> convert(QString in, quint8 nCharPack);
   void footerFormatInit();
   quint8 getBaudrate();
   QString getFPGA_FW();
   void setBaudrate(int value);
   QString getHw_rev();
   int getSerialNo();
   QString getVcom();
   double getBiasVoltage();
   bool getModeSnifferIsChecked();

signals:
      void clearConsole();

public slots:
   void footerRefreshVisa();
//   void SinToRam();
   void onActionRegistersOverview();
   void onBtnLoadSinClicked();
   void onTim1Timeout();
   void clearConsoleCaller();
   void about();
   void onTimHbeatTimeout();
   void CRASHME();
   void writeRam(const QByteArray *datIn, quint8 target);
   void onErrorReceived(QString errStr);
   void onDriverPortConnected();
   void onDriverPortDisconnected();
   void onSerialBaudChanged(qint32 baud, QSerialPort::Directions dir);
   bool savePersistance();
   bool loadPersistance();
   void onBtnDeleteCurrIdxClicked();
   void onBtnStopSamplingClicked();
   void onRbxxxContentClicked();
   void onBtnRefreshRegMap();
   void onBtnOpenRegView();
   void enableUiElements(bool uistate);
   void onOpenDvmClicked();
   void closeChildsAlso();
   void footerUpdate(PortDialog::Settings p);
   void onTransEditReturnPressed();
   void autoRunTimerSLOT();
   void onBtnOutputsOnOffClicked();
   void refreshGS();
   void childsToDbg();
//   void onTimRefreshTimeout();
   bool getParserIsChecked();
   void setVisaLED_VLogic(LEDs cLed, bool VlogicEn,
                          quint8 volt = 3, VisaReg *vr = 0);

   void onActSnapShotTriggered();
   void onBtn8a8bClicked();
   void onLe8a8bChanged(QString str);
   void onActPowerTriggered(bool onoff);
   void setSnifferTimEnabled(bool onoff);
   bool getUiClearConsoleIsChecked();
   void setUiClearConsoleChecked(bool b);
   bool getUiSuppressDefaultPutIsChecked();
   void setUiSuppressDefaultPutChecked(bool b);
   bool getUiPeriodicReqIsChecked();
   void setUiPeriodicReqChecked(bool b);
   void onActFuGenTriggered(bool onoff);
   void keyPressEvent(QKeyEvent *);
   void showEvent(QShowEvent *);
   void closeEvent(QCloseEvent *);
   void restoreAllGeometrys();
   void saveAllGeometrys();
   void onChildWidgetDestroyed(QObject *died);

protected:
   void initActionsConnections();

protected slots:

private slots:
   void on_btnOutputsOnOff_clicked(bool checked);
   void on_pbRepeatOnOff_clicked();
   void quit();

private:
   Ui::Visa *ui;
   QTimer /**timHbeat,*/ *timUserSeq, /**timRefresh,*/
   *txStdByTim, *rxTimeoutTim, *reqRefreshTim,
   *repeatTim;

   MQTimer_t *hbeat, *sniffTim;
   QString  txStr;
   QTextEdit *teStat;

   Register *regFrm;
   Driver   *driver;
   IOEdit   *ioeditL, *ioeditR;
   VisaReg  *visareg;
   Dvm      *dvmDc, *dvmAcDc;
   FuGen    *fugen;
   Calc     *calc;
   Power    *pwr;

   QWidget *newParent;

   QTextCharFormat formatDefault,  //< Object to store default format settings
   formatLHS,      //< Object to store format settings of LHS strings
   formatRHS,      //< Object to store format settings of RHS strings
   formatRHS_red,  //< ... if NotOpen, textcolor := red
   formatRHS_gn;   //< ... if Open, textcolor := green
   /**
    * @brief portSettings  Holds the port settings received in last
    * onPortConnected slot access.
    */
   PortDialog::Settings driverSettings;
   static Visa *inst;
   QStringListModel* cbModelRepeatSeq;
   static Reg_bebf * regLedVlogic;
   formateds le8a8bForm;

};
Q_DECLARE_OPERATORS_FOR_FLAGS(Visa::LEDs)


#endif // VISA_H


