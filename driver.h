#ifndef DRIVER_H
#define DRIVER_H

#include <QObject>
#include <QWidget>
#include <QtCore/QtGlobal>
#include <QSerialPort>
#include <QMainWindow>
#include <QObject>
#include <QWidget>

#include "globals.h"
#include "portdialog.h"

/**
 * Forward declaration of following classes is ok since we are only using
 * it for declaring class pointers
 */
class Visa;
class MainWindow;
class IOEdit;
//class PortDialog;


class Driver : public QObject {

   Q_OBJECT

public:
   explicit Driver(QString eolPatt = "00000046706700",
                   bool newLine = 1,
                   bool removeHead = 0,
                   Visa *parent = 0);

   ~Driver();
   static Driver *getInstance(/*VisaReg *visaregPtr*/) {
      if (instance == 0) {
         instance = new Driver();
         //         visareg = visaregPtr;
      }

      return instance;
   }
   static Driver *getObjectPtr() {
      return instance;
   }

   PortDialog *settings;
   QSerialPort *serial;
   int MIN_FRAME_SIZE_VAR;

   struct {
      /** 16-10-2015    abschaltbarer rx frame error handler eingeführt,
         * weil sniffen nicht mehr funktioniert hat ohne frame errors zu erzeugen.
         * In diesem zusammenhang wurde der member *dataSync in dataSync geändert.
         * TICKET_5548584
        QString *dataSync;
         */
      QString dataSync;
      QString eolPattern;
      quint64 byteCount;
      bool hideFrameheader, appendNewline;
      void setByteCnt(quint64 value) {
         byteCount = value;
      }
      void clearByteCnt(void) {
         byteCount = 0x00;
      }
      quint64 getByteCnt () {
         return byteCount;
      }

   } rxBuff;

   struct {
      /** 16-10-2015    abschaltbarer rx frame error handler eingeführt,
         * weil sniffen nicht mehr funktioniert hat ohne frame errors zu erzeugen.
         * In diesem zusammenhang wurde der member *dataSync in dataSync geändert.
         * TICKET_5548584
        QString *dataSync;
         */
      QByteArray dataSync;
      QByteArray dataUnSync;
      QString eolPattern;
      quint64 byteCount;
      bool hideFrameheader, appendNewline;
      void setByteCnt(quint64 value) {
         byteCount = value;
      }
      void clearByteCnt(void) {
         byteCount = 0x00;
      }
      quint64 getByteCnt () {
         return byteCount;
      }
      void flushAll() {
         dataSync.clear();
         dataUnSync.clear();
      }
   } rxBuffAlt;

   struct rxBuffReq_t {
      QByteArray  data;
      qint64      expectedByteCount;
      quint64     rxByteCount;
      quint64 getByteCnt () {
         return rxByteCount;
      }
      quint64 addByteCnt (quint64 add) {
         return rxByteCount += add;
      }
      void clearByteCnt () {
         rxByteCount = 0;
      }
      void flush() {
         data.clear();
         expectedByteCount = 0;
      }
   };

   rxBuffReq_t rxBuffReq;
   PortDialog::Settings getSettings();
   bool isPortOpen();
   qint64 readRxResp();

signals:
   void portDisconnected(bool MUSTBEFALSE = false);
   void portConnected(bool MUSTBETRUE = true);
   void criticalDriverError(QString errStr);
   void reqResponseCmpl(QByteArray &response);
   void reqResponseFrameError(qint64 nBytesRec);
   void reqRespKillTimeout();

public slots:
   void clearRxBuffByteCnt() {
      rxBuff.clearByteCnt();
   }
   void clearRxBuffReqByteCnt() {
      rxBuffReq.clearByteCnt();
   }
   void openSerialPort();
   void closeSerialPort();
   void handleError(
         QSerialPort::SerialPortError error);
   void onOpenPortDialog();
   QIODevice::OpenMode getOpenMode();
   int writeData(const QByteArray &data,
                 bool putToConsole = true);
   void setHideFrameheader(bool choose = 0);
   void setAppendNewlineChar(bool choose = 1);
   void flushSyncedBuff(qint32 pos = -1,
                        qint32 length = -1);
   void flushSyncedReqBuff(qint32 pos = -1,
                           qint32 length = -1);
   void handleError(qint64 customError);
   void onCheckRespCmpl();
   void ackResponseFrameError();
   void sniffReceived();

private:
   QString portName;
   MainWindow *mw;
   int Baudrate;
   static Driver *instance;
   IOEdit *ioedit;
   Visa *visa;
};

#endif // DRIVER_H
