
#ifndef DRIVER_H
#define DRIVER_H

#include "mainwindow.h"
#include "globals.h"

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QSerialPort>
#include <QMainWindow>
#include <QObject>
#include <QWidget>

//class Console;
class PortDialog;
class Visa;
class MainWindow;
class IOEdit;

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
class Driver : public QObject
{
   Q_OBJECT

public:
   /**
    * @brief
    *
    * @param eolPatt
    * @param newLine
    * @param removeHead
    * @param parent
    */
   /**
    * @brief
    *
    * @param eolPatt
    * @param newLine
    * @param removeHead
    * @param parent
    */
   /*!
    \brief

    \param eolPatt
    \param newLine
    \param removeHead
    \param parent
   */
   explicit Driver(QString eolPatt = "00000046706700",
                   bool newLine = 1,
                   bool removeHead = 0,
                   Visa *parent = 0);
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
   ~Driver();
   /**
    * @brief
    *
    * @return Driver
    */
   /**
    * @brief
    *
    * @return Driver
    */
   /*!
    \brief

    \return Driver
   */
   static Driver* getInstance(/*VisaReg *visaregPtr*/) {
      if (instance == 0) {
         instance = new Driver();
         //         visareg = visaregPtr;
      }
      return instance;
   }
   /**
    * @brief
    *
    * @return Driver
    */
   /*!
    \brief

    \return Driver
   */
   static Driver* getObjectPtr() {
      return instance;
   }
   //   Console *console;
   PortDialog *settings;   
   QSerialPort *serial;   
   int MIN_FRAME_SIZE_VAR;   

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
      /**
       * @brief
       *
       * @param value
       */
      /*!
       \brief

       \param value
      */
      void setByteCnt(quint64 value) {
         byteCount = value;
      }
      /**
       * @brief
       *
       */
      /*!
       \brief

      */
      void clearByteCnt(void) {
         byteCount = 0x00;
      }
      /**
       * @brief
       *
       * @return quint64
       */
      /*!
       \brief

       \return quint64
      */
      quint64 getByteCnt () {
         return byteCount;
      }

   } rxBuff;  

   /** RINGBUFFER */

   /**
    * @brief
    *
    */
   /*!
    \brief

   */
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
      /**
       * @brief
       *
       * @param value
       */
      /*!
       \brief

       \param value
      */
      void setByteCnt(quint64 value) {
         byteCount = value;
      }
      /**
       * @brief
       *
       */
      /*!
       \brief

      */
      void clearByteCnt(void) {
         byteCount = 0x00;
      }
      /**
       * @brief
       *
       * @return quint64
       */
      /*!
       \brief

       \return quint64
      */
      quint64 getByteCnt () {
         return byteCount;
      }
      /**
       * @brief
       *
       */
      /*!
       \brief

      */
      void flushAll() { dataSync.clear(); dataUnSync.clear(); }

   } rxBuffAlt;  

   /** RINGBUFFER END */


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
   struct rxBuffReq_t {
      QByteArray  data;  
      qint64      expectedByteCount;  
      quint64     rxByteCount;  

      /**
       * @brief
       *
       * @return quint64
       */
      /*!
       \brief

       \return quint64
      */
      quint64 getByteCnt ()
         { return rxByteCount; }

      /**
       * @brief
       *
       * @param add
       * @return quint64
       */
      /*!
       \brief

       \param add
       \return quint64
      */
      quint64 addByteCnt (quint64 add)
         { return rxByteCount += add; }

      /**
       * @brief
       *
       */
      /*!
       \brief

      */
      void clearByteCnt ()
         { rxByteCount = 0; }

      /**
       * @brief
       *
       */
      /*!
       \brief

      */
      void flush() {
         data.clear();
         expectedByteCount = 0;
      }
   };

   rxBuffReq_t rxBuffReq;  

   /**
    * @brief
    *
    * @return PortDialog::Settings
    */
   /**
    * @brief
    *
    * @return PortDialog::Settings
    */
   /*!
    \brief

    \return PortDialog::Settings
   */
   PortDialog::Settings getSettings();
   /**
    * @brief
    *
    * @return bool
    */
   /**
    * @brief
    *
    * @return bool
    */
   /*!
    \brief

    \return bool
   */
   bool isPortOpen();
   /**
    * @brief
    *
    * @return qint8
    */
   /**
    * @brief
    *
    * @return qint64
    */
   /*!
    \brief

    \return qint64
   */
   qint64 readRxResp();

signals:
   /**
    * @brief
    *
    * @param MUSTBEFALSE
    */
   /**
    * @brief
    *
    * @param MUSTBEFALSE
    */
   /*!
    \brief

    \param MUSTBEFALSE
   */
   void portDisconnected(bool MUSTBEFALSE = false);
   /**
    * @brief
    *
    * @param MUSTBETRUE
    */
   /**
    * @brief
    *
    * @param MUSTBETRUE
    */
   /*!
    \brief

    \param MUSTBETRUE
   */
   void portConnected(bool MUSTBETRUE = true);
   /**
    * @brief
    *
    * @param errStr
    */
   /**
    * @brief
    *
    * @param errStr
    */
   /*!
    \brief

    \param errStr
   */
   void criticalDriverError(QString errStr);
   /**
    * @brief This signal is emitted from readRxResp() after checking serial->
    * bytesAvailable against rxBuffReq.expectedByteCount. If they match, this
    * signal gots thrown.
    *
    * @param response. Attach a rxbuffer reference thru the valied content
    * to the emitted signal
    */
   /**
    * @brief
    *
    * @param response
    */
   /*!
    \brief

    \param response
   */
   void reqResponseCmpl(QByteArray& response);
   /**
    * @brief This signal is emitted if an buffer overrun has occured. This means
    * there are more bytes available for reading than expected.
    * @param nBytesRec Available byte count.
    */
   /**
    * @brief
    *
    * @param nBytesRec
    */
   /*!
    \brief

    \param nBytesRec
   */
   void reqResponseFrameError(qint64 nBytesRec);

   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   void reqRespKillTimeout();
public slots:
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   void clearRxBuffByteCnt() { rxBuff.clearByteCnt(); }
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   void clearRxBuffReqByteCnt() { rxBuffReq.clearByteCnt(); }
   //   void setup();
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
   void openSerialPort();
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
   void closeSerialPort();
   /**
    * @brief
    *
    * @param error
    */
   /**
    * @brief
    *
    * @param error
    */
   /*!
    \brief

    \param error
   */
   void handleError(
         QSerialPort::SerialPortError error);
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
   void onOpenPortDialog();
   /**
    * @brief
    *
    * @return QIODevice::OpenMode
    */
   /**
    * @brief
    *
    * @return QIODevice::OpenMode
    */
   /*!
    \brief

    \return QIODevice::OpenMode
   */
   QIODevice::OpenMode getOpenMode();
   /**
    * @brief
    *
    * @param data
    * @param putToConsole
    * @return int
    */
   /**
    * @brief
    *
    * @param data
    * @param putToConsole
    * @return int
    */
   /*!
    \brief

    \param data
    \param putToConsole
    \return int
   */
   int writeData(const QByteArray &data,
                  bool putToConsole = true);
   /**
    * @brief
    *
    * @param choose
    */
   /**
    * @brief
    *
    * @param choose
    */
   /*!
    \brief

    \param choose
   */
   void setHideFrameheader(bool choose = 0);
   /**
    * @brief
    *
    * @param choose
    */
   /**
    * @brief
    *
    * @param choose
    */
   /*!
    \brief

    \param choose
   */
   void setAppendNewlineChar(bool choose = 1);
   /**
    * @brief
    *
    * @param pos
    * @param length
    */
   /**
    * @brief
    *
    * @param pos
    * @param length
    */
   /*!
    \brief

    \param pos
    \param length
   */
   void flushSyncedBuff(qint32 pos = -1,
                        qint32 length = -1);
   /**
    * @brief
    *
    * @param pos
    * @param length
    */
   /**
    * @brief
    *
    * @param pos
    * @param length
    */
   /*!
    \brief

    \param pos
    \param length
   */
   void flushSyncedReqBuff(qint32 pos = -1,
                           qint32 length = -1);

   /**
    * @brief
    *
    * @param customError
    */
   /*!
    \brief

    \param customError
   */
   void handleError(qint64 customError);

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
   void onCheckRespCmpl();
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   void ackResponseFrameError();

   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   void sniffReceived();
private:
   QString portName;   
   MainWindow * mw;   
   int Baudrate;   
   static Driver * instance;   
   IOEdit * ioedit;   
   Visa * visa;   
};

#endif // DRIVER_H
