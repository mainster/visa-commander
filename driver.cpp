
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "portdialog.h"
#include "visa.h"
#include "globals.h"
#include "driver.h"
#include "sleeper.h"

#include <QtSerialPort/QSerialPort>
#include <QMessageBox>
#include <QSettings>

#include <QMainWindow>
#include <QObject>
#include <QWidget>

#define MIN_FRAMESIZE 30

Driver* Driver::instance = 0; 

/**
 * @brief Driver::Driver
 * @param eolPattern Optional QByteArray pattern used as eol or newline
 * sequence for the rx buffer.
 * @param parent
 */
/*!
 \brief

 \param eolPatt
 \param newLine
 \param removeHead
 \param parent
*/
Driver::Driver(QString eolPatt,
               bool newLine,
               bool removeHead,
               Visa *parent) :
   QObject(parent) {

   serial = new QSerialPort();
   settings = new PortDialog();
   ioedit   = IOEdit::getInstance();


   /////////////////////////////////////
   //   visa = Visa::getInstance();
   /////////////////////////////////////

   /** siehe TICKET_5548584
   rxBuff.dataSync = new QString();
   */
   /** siehe TICKET_5548584 */
   //   visa = Visa::getInstance();

   rxBuff.eolPattern = eolPatt;
   rxBuff.appendNewline = newLine;
   rxBuff.hideFrameheader = removeHead;

   rxBuffReq.data.clear();// = new QByteArray();
   rxBuffReq.expectedByteCount = 0;
   rxBuffReq.rxByteCount = 0;

   QSETTINGS;
   MIN_FRAME_SIZE_VAR = config.value("ReadOnly/MIN_FRAME_SIZE",
                                     MIN_FRAMESIZE).toInt();

   connect(serial,
           SIGNAL(error(QSerialPort::SerialPortError)), this,
           SLOT(handleError(QSerialPort::SerialPortError)));

   /** Connect the default serial data received slot. In normal program flow,
    * readData() should never be accessed in absence of a second, request-
    * dependent slot because the visascope only acts as a slave device.
    */
   connect(serial,
           SIGNAL(readyRead()), this,
           SLOT(onCheckRespCmpl()));
   connect(this,
           SIGNAL(reqResponseFrameError(qint64)), this,
           SLOT(handleError(qint64)));

}
/**
 * @brief
 *
 */
/*!
 \brief

*/
Driver::~Driver() {
   settings->close();
   closeSerialPort();
}
/*!
 \brief

*/
void Driver::openSerialPort() {
   PortDialog::Settings p = settings->settings();

   if (serial->isOpen())
      return;
   serial->setPortName(p.name);
   portName = p.name;

   serial->setBaudRate(p.baudRate);
   serial->setDataBits(p.dataBits);
   serial->setParity(p.parity);
   serial->setStopBits(p.stopBits);
   serial->setFlowControl(p.flowControl);

   if (serial->open(p.openMode)) {
      serial->flush();
      rxBuffReq.flush();
      emit portConnected();
   }
   else
      emit criticalDriverError(serial->errorString());
}
/*!
 \brief

*/
void Driver::closeSerialPort() {
   Visa *visa = Visa::getObjectPtr();

   visa->setVisaLED_VLogic(Visa::led_gn, false);
   serial->close();
   emit portDisconnected();
}
/*!
 \brief

 \param data
 \param putToConsole
 \return int
*/
int Driver::writeData(const QByteArray &data, bool putToConsole) {
   Q_ASSERT(data.length() > 0);
   static QString last;

   Visa *visa = Visa::getObjectPtr();

   if (serial->isOpen()) {
      serial->write( data );
      //      serial->flush();
   }
   else {
      ioedit->putTxData( "serial->isOpen() returns FALSE" );
      return -1;
   }
   if (putToConsole) {
      if (visa->getParserIsChecked()) {
         QByteArray ba;
         ba = data.toHex();
//         ba.append( visa->convert(data), data.length()/2 );

         QString s = ioedit->payloadDiff( ba, last, QColor(Qt::yellow));
         ioedit->putTxData( /*data.toHex()*/s );
         last = ba;
      }
   }

   return 0;
}
/* ======================================================================== */
/*             Check if read request response is complete                   */
/* ======================================================================== */
/**
 * @brief This slot should be connected to readyRead() driver signal as soon as
 * the driver has been successfully connected to an QIODevice.
 * When a read request is sent to the visascope, simultaneously a specific,
 * response processing slot should be connected to the readyRead() signal.
 * After processing, this specific slot becomes disconnected.
 * Due to the fact, that the scope never sends data without beeing requested for
 * it, in normal program flow the onDefaultDataRead() slot never should be
 * called without a specific slot is also connected to the signal.
 *
 * 16-10-2015     Sniffer mode
 * In sniffer mode, this slot is accessed by default. Senity check for
 * actionModeSniffer.isChecked and receivers(..) count should be 1
 * Thus, assert this state!
 *
 */
/*!
 \brief

*/
void Driver::onCheckRespCmpl() {
#ifdef VERBOSITY_LVL_5
   Q_INFO << "was called";
#endif
   Visa *l_visa = Visa::getObjectPtr();

   /** If visa-commander is in sniffer mode... */
   if (l_visa->getModeSnifferIsChecked()) {
      sniffReceived();
      return;
   }

   /** In standard mode (commander): onCheckResp Cmpl() accessed even though
    * zero rx bytes are expected. This seems to be an error state!
    */
   if (! rxBuffReq.expectedByteCount) {
      ioedit->putInfoLine(tr("onCheckResp Cmpl() accessed but"
                             "rxBuffReq.expectedByteCount == 0!"));
      rxBuff.dataSync = QString(serial->readAll().toHex());
      ioedit->putRxData(rxBuff.dataSync,
                        IOEdit::ClearReferencedBuffer,
                        IOEdit::WhitespaceChars);

      return;
   }

   /** In standard mode (commander): If rxBuffReq.expectedByteCount is
    * non-empty, this is a valied state to call readRxResp()
    */
   if (rxBuffReq.expectedByteCount > 0) {
      readRxResp();
   }
   else
      qWarning();

}
/* ======================================================================== */
/**
 * @brief Driver::readRxResp
 * @return  -1 if n bytes received and n > nBytesExp is true. This is an
 *          overflow scenario.
 *          0  if rx byte count matches the expected count of bytes
 *          n  count of rxbytes received at this point and n < nBytesExp
 *
 * Is called from inside onDefaultDataRead() slot.
 * If (serial->bytesAvailable() >= nByteExp) is true, this methode emits
 * reqResponseCmpl(..) signal
 */
/*!
 \brief

 \return qint64
*/
qint64 Driver::readRxResp() {
#ifdef VERBOSITY_LVL_5
   Q_INFO << "was called";
#endif

   qint64 nByteExp = rxBuffReq.expectedByteCount;
   qint64 nBytesRec = serial->bytesAvailable();

   /** 15-10-2015    The attribute returned by methode bytesAvailable() must
    * exactly match to the nByteExpected variable.
    * If bytesAvailable() < nByteExp, everything is ok as this means, the os task
    * sheduler has returned to the thread before the fpga transmission completes.
    * If bytesAvailable() > nByteExp, definitely some (timing) error occured as
    * the buffered rxstream isn't read back fast enough.
    */
   if (nBytesRec < nByteExp) {
      /**
       * Smaller means, not all bytes of the rxstream are received yet. This is
       * NOT an error case!
       */
      return nBytesRec;
   }
   else {
      /** Read exactly the amount of expected bytes from serial buffer */
      rxBuffReq.data.append( serial->read(nByteExp) );
      rxBuffReq.addByteCnt( nByteExp );

      if (nBytesRec == nByteExp) {
         /** Clear rxBuffReq.expectedByteCount = 0 */
         rxBuffReq.expectedByteCount = 0;

         /** Emit request-completly-answered-signal and attatch rx data to the
          * emitted signal.
          */
         emit reqResponseCmpl( rxBuffReq.data );

         /** Emit signal reqRespKillTimeout() to kill active timeout timers */
         emit reqRespKillTimeout();
         return 0;
      }
      else {
         /** Set the rxBuffReq.expectedByteCount value to the negative
          * difference as this value is used to check for previous errors
          * befor trxVisa(..) initiates a new request.
          */
         rxBuffReq.expectedByteCount = nByteExp - nBytesRec;

         /** Not smaller and not equal indicates that an error has occured!
          * There for, don't flush the rest of the buffer but emit a protocol
          * error signal.
          */
         emit reqResponseFrameError( nBytesRec );
         return -1;
      }
   }
}
/* ======================================================================== */
/*!
 \brief

 \param error
*/
void Driver::handleError(QSerialPort::SerialPortError error) {
   if (error == QSerialPort::ResourceError) {
      emit criticalDriverError(serial->errorString());
      closeSerialPort();
   }
}
/**
 * @brief Driver::handleError This function overloads the error handler for
 * QSerialPort::SerialPortError so we can attach uder defined error "codes" to
 * a driver emitted error signal.
 * @param customError
 */
/*!
 \brief

 \param customError
*/
void Driver::handleError(qint64 customError) {
   Q_INFO << tr("bytes received: ") << QString::number(customError);
   /** TODO: Error handling in cases of buffer overrun */

   /** Start timer to produce a delay between error detection and error ack */
   QTimer::singleShot(10, Qt::CoarseTimer,
                      this, SLOT(ackResponseFrameError()));
}
/*!
 \brief

*/
void Driver::ackResponseFrameError() {
   /** This slot is called after a delay time if a rx request frame error
    * occured, i.e. a rx buffer underrun error.
    * While the error is NACKed, further trxReq are blocked.
    */
   Visa *visa = Visa::getObjectPtr();

   if (visa->gs.errorHandleractive) {
      rxBuffReq.flush();
      serial->flush();
   }
}
/*!
 \brief

*/
void Driver::sniffReceived() {

   ioedit->onSniffFilter();
   return;

#ifdef VERBOSITY_LVL_5
   Q_INFO << "was called";
#endif
   int pos = 0, cnt = 0;
   const QString LF = tr("\n");

   rxBuff.dataSync.append( QString(serial->readAll().toHex()) );
   rxBuff.dataSync.remove("00000046706700040001620100057200000000000002be15000047e0");

   if (rxBuff.dataSync.contains(FRAMEHEAD.remove(" "))) {
            while ((pos = rxBuff.dataSync.indexOf(FRAMEHEAD.remove(" "),
                                            pos + LF.length() + 1)) > 0) {
         rxBuff.dataSync.insert(pos, LF);
         cnt++;
      }
      rxBuff.dataSync.remove(0, rxBuff.dataSync.indexOf("\n") + 1);

      if (cnt >= 2) {
         QString tmp = rxBuff.dataSync.left(rxBuff.dataSync.lastIndexOf( LF ));
         ioedit->putRxDataSniff(tmp,
                           IOEdit::ClearReferencedBuffer,
                           IOEdit::WhitespaceChars);
      }
   }
}
/*!
 \brief

*/
void Driver::onOpenPortDialog() {
   settings->show();
}
/*!
 \brief

 \return QIODevice::OpenMode
*/
QIODevice::OpenMode Driver::getOpenMode() {
   return serial->openMode();
}
/*!
 \brief

 \return PortDialog::Settings
*/
PortDialog::Settings Driver::getSettings() {
   return settings->settings();
}
/*!
 \brief

 \return bool
*/
bool Driver::isPortOpen() {
   return serial->isOpen();
}
/*!
 \brief

 \param choose
*/
void Driver::setHideFrameheader(bool choose) {
   rxBuff.hideFrameheader = choose;
}
/*!
 \brief

 \param choose
*/
void Driver::setAppendNewlineChar(bool choose) {
   rxBuff.appendNewline = choose;
}
/** IOEdit object emits a syncedBufferWasted signal after all frames
 * has been processed. Connect this signal to the flushSyncedBuffer
 * methode to ensure a clean rxBuff.dataSync buffer on next driver call.
 */
/*!
 \brief

 \param pos
 \param length
*/
void Driver::flushSyncedBuff(qint32 pos, qint32 length) {
   /** TODO: Implement this accessor inside rxBuff structure */
   /** TODO: Implement range check */
   if ((pos > 0) && (length > 0))
      /** siehe TICKET_5548584
      rxBuff.dataSync->remove(pos, length);
      */
      rxBuff.dataSync.remove(pos, length);
   else
      /** siehe TICKET_5548584
      rxBuff.dataSync->clear();
      */
      rxBuff.dataSync.clear();
}
/*!
 \brief

 \param pos
 \param length
*/
void Driver::flushSyncedReqBuff(qint32 pos, qint32 length) {
   /** TODO: Implement this accessor inside rxBuff structure */
   /** TODO: Implement range check */
   if ((pos > 0) && (length > 0))
      rxBuffReq.data.remove(pos, length);
   else
      rxBuffReq.data.clear();
}

