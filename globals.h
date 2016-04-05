#ifndef GLOBALS_H
#define GLOBALS_H

#include <QByteArray>
#include <QString>
#include <QSettings>
#include <QTimer>
#include <QTime>

#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>

class Globals;

#define LOAD_HARDWARE_DAT_FROM_FILESYSTEM
//#undef LOAD_HARDWARE_DAT_FROM_FILESYSTEM

#define CONFIG_PATH     "/home/mainster/.config/visa-commander/config"

#define TMP_PATH        "/tmp/"
/** Heartbeat timer periode - periodically req default register information */
#define T_HEARTBEAT     10e-3

/** Refresh timer - periodically update footer information and others.. */
#define T_REFRESH       1000e-3

/** Timer periode for serial mode "sniffer" */
#define T_SNIFFER     250e-3


#define SHOW(a) std::cout << #a << ": " << (a) //<< std::endl
#define TO_STREAM(stream,variable) (stream) <<#variable": "<<(variable)
#define qoutLf(x) (std::cout << #x << std::endl)
#define qout(x) (std::cout << #x) << std::endl

#define milliSec_1   1
#define QLC QLatin1Char('0')
#define QLCS QChar(' ')
#define INIT QString("@")
#define FRAMEHEAD QString("00 00 00 46 70 67 00")
#define INIT_WRITE_REG QString("%START %BLOCKS ")
#define INIT_READ_REG_REQUEST QString("%START 01 %NUM_READ e0")

/**
 * Define ascii coded header sequences
 *
 * As explained in the last chapter, a * data-stream sent from the PC starts
 * with an header, con- taining 3 * zero-bytes, 3 indicator bytes and 1
 * zero-byte. This 7-byte-header deﬁnes the * differ- ent types of data-stream.
 */

#define HEAD_UC_INFO          tr("00 00 00 75 50 63 00")
#define HEAD_CONFIG_FPGA      tr("00 00 00 50 72 6f 00")
#define HEAD_READ_CAL_VAL     tr("00 00 00 45 50 72 00 %S %N")
#define HEAD_WRITE_CAL_VAL    tr("00 00 00 65 70 57 00")
#define HEAD_FPGA_COMM        tr("00 00 00 46 70 67 00")
#define HEAD_DATA_TO_SERIAL   tr("00 00 00 54 73 74 00")
#define HEAD_DEFAULT_REQ      QString("00 00 00 46 70 67 00 01 %NUM_READ e0")
//#define HEAD_DEFAULT_ORIG_REQ QString("00 00 00 46 70 67 00 01 %NUM_READ e0")

#define  TESTBYTES  tr("66 77 88 99")


#define HTML_FRAMEHEAD_IT1      "<i> 00000046706700 </i>"
#define HTML_FRAMEHEAD_IT2      "<b style=\"color: #22ff00; \"> 00000046706700 </b>"
#define HTML_FRAMEHEAD_DARKGN_IT    "<font color=DarkGreen><i> 00000046706700 </i></font>"
#define HTML_FRAMEHEAD_DARKRD_IT    "<font color=DargRed><i> 00000046706700 </i></font>"

#define HTML_FONT_RED            QString("<font color=\"Red\"> %1 </font>")
#define HTML_FONT_DARKRED        QString("<font color=\"color: #AD0000;\"> %1 </font>")
#define HTML_FONT_xCOLOR_IT      QString("<b style=\"color: #%COLOR; background-color: #000000\"> <i> %1  </i> </b>")
#define HTML_FONT_GRAY           QString("<font color=\"Gray\">")
#define HTML_FONT_xCOLOR_RULER   QString("<b style=\"color: #%COLOR; background-color: #000000\"><i>%1</i></b>")   //QString("<font color=\"Gray\"> %1 </font>")
#define HTML_FONT_BLUE           QString("<font color=\"Blue\">")
#define HTML_FONT_BG             QString("<b style=\"color: #000000; background-color: #c0c0c0\">@@@REPLACE</b>")
#define HTML_FONT_DEF            QString("</font><br>")
#define HTML_INFO_LINE           QString("<i style=\"color: #FF6A00;\"> %PREFIX %1 </i></font>")

#define QSETTINGS QSettings config(CONFIG_PATH, QSettings::IniFormat);
//#define Q_INFO (qDebug().noquote() << Q_FUNC_INFO )
#define Q_INFO (qDebug().noquote() << QTime::currentTime() << Q_FUNC_INFO )
//#define Q_INFO (qDebug())

#define LIM_12BIT_UINT ((uint16_t) pow(2, 12)-1)   // 4095


//#define pathToDat tr("/opt/Visatronic/visascope-3.1/share/hardware_values/hardware_137000.dat")
#define DEFAULT_BACKUP_PATH tr("/tmp/visacommand_tmp")
//#define pathToDat_SORT "/opt/Visatronic/visascope-3.1/share/hardware_values/rewSort_hardware_137000.dat"
//typedef QVector<double> calDat_t;

#define DVM_DC_TITLE QString("Dvm DC")
#define DVM_ACDC_TITLE QString("Dvm ACDC")

class Globals {

   explicit Globals();

public:
   ~Globals();
   static Globals *getObjPtr() {
      return inst;
   }
   static Globals *getInstance() {
      if (inst == 0) {
         inst = new Globals();
      }
      return inst;
   }
//   static void initGlobals();

   QSettings * conf;
public slots:
   QString getPathToDat();
   QString getPathToBackup();
   QString getPathToConfig();

   void setPathToDat(const QString str);
   void setPathToBackup(const QString &str);
   void setPathToConfig(const QString &str);

private:
   QString pathToDat;
   QString pathToBackup,pathToConfigss;

   static const QStringList DEFAULT_PATH_TO_CONFIG;
   QStringList * pathToConfig;

   static Globals *inst;

};


#endif

