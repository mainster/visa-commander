#include <QtCore>
#include <QDebug>

#include "visareg.h"
#include "visa.h"
#include "ioedit.h"
#include "utype.h"
#include "hwreg.h"
#include "driver.h"
#include "calc.h"
#include "dvm.h"
#include "globals.h"

class VisaReg;

#define STREAM_HUMAN_READABLE

VisaReg* VisaReg::inst = 0;

QVector<double> VisaReg::H(100);

/* ======================================================================== */
/*                     class constructor                                    */
/* ======================================================================== */
VisaReg::VisaReg(QObject *parent) :
   QObject(parent) {

//   QSETTINGS;
//   int maxcfg = config.value("ReadOnly/MAX_CONSOLE_CHARS",
//                             Visa::MAX_CONSOLE_CHARS).toInt();
   /**
    * Initial state of HW register structures
    */
   init_HwRoImg();
   init_HwRwImg();

   timReqTimeout = new QTimer();
   /** Request for a pointer to driver instance */
   driver = Driver::getObjectPtr();    //?????
//   ioeditL= IOEdit::getInstance(maxcfg, parent->parent());
   ioeditL= IOEdit::getObjectPtr();
   visa   = (Visa*) parent;

   regActRamTrig = new Reg_0609();
   regTrigEvDist = new Reg_0a0d();
   regAdcPwr     = new Reg_1017();
   regAdcDvm     = new Reg_181f();
   regFgenOut    = new Reg_3037();
   regFgenFreq   = new Reg_383b();
   regSetPwr     = new Reg_8087();
   regDvmRngInp  = new Reg_8a8b();
   regLedVlogic  = new Reg_bebf();

//   calc = Calc::getInstance(this);
   calc = Calc::getInstance();

   connect( driver,        SIGNAL(reqRespKillTimeout()),
            timReqTimeout, SLOT(stop()));

   inst = this;
}
VisaReg::~VisaReg() {

}
QVector<double> VisaReg::getH()
{
    return H;
}

void VisaReg::setH(const QVector<double> &value)
{
    H = value;
}


/* ======================================================================== */
/*                     FPGA communication protocols                         */
/* ======================================================================== */
/* The low level protocol implementation is classified in 4 general
 * communication protocols or stream types. Write and WriteRead access
 * to a bunch of FPGA registers as well as Write and WriteRead access
 * to its RAM benches
 */

/** (A) Write to registers  -->  NO RESPONSE FROM SCOPE EXPECTED
 * Input parameter is a vector that contains preconfigured registers.
 * Combining several Blocks to one data-stream is only possible by sending
 * datas to writable Registers. In order to support this feature, the
 * data-stream to the FPGA has he form, shown in ﬁgure 2.5
 */
QByteArray VisaReg::writeToReg(QVector<HwReg*> &regObj,
                               RetFormat retFormat) {
   /** Sort registers in ascending address order */
   sortForAddrs(regObj);

   /** Assemble multiple block txStream */
   QByteArray retASCII = assambleStream(regObj, false);

   /** Convert to binary representation */
   QByteArray retRaw;
   retRaw.append( convert( retASCII ), retASCII.length()/2 );

   //   bool *ok;

   switch (retFormat) {
      case RetFormat_ASCII:   {
         driver->writeData(retASCII);
         return retASCII;
      } break;

      case RetFormBIN:  {
         driver->writeData(retRaw);
         return retRaw;
      } break;

      default:
         break;
   }
   return QByteArray("");
}

/** (B) Read from registers
 * Input parameter is the number of registers to read back. The FPGA always
 * starts responding with register 0x00. Therefor, if we want to read, for
 * eample, up to register address 0x08 NUM_READ must be set to 9.
 * In order to read-out the Registers from dec0 up to dec191, you have to
 * adress Register 0xe0 or 0xff. You must specify the number of Datas with
 * NM and NL and you must choose the adress 0xe0. Now, you obtain from the
 * VISASCOPE the datas of the N ﬁrst Registers. The received String has as
 * many bytes as you specify by the number NM and NL.
 */
void VisaReg::onReadReqTimeout() {
   if (! timReqTimeout->isActive())
      return;

   quint32 rxCount = driver->serial->bytesAvailable();
   QByteArray rxed;
   if (rxCount)
      rxed = QByteArray( driver->serial->read(rxCount) );
   else
      rxed.append("");

   QString msg = QString("Rx Timeout, bytes received: %1" + rxed).arg(rxCount);
   ioeditL->putInfoLine( msg );
   Q_INFO << msg;

   driver->serial->flush();

   /** Reset trx error mechanism */
   driver->rxBuffReq.flush();

}

/**
 * Insertionsort ist an das intuitive Vorgehen von Menschen angelehnt, wenn sie
 * vor ein Sortierproblem gestellt werden. Üblicherweise gibt es hier eine
 * Menge unsortierter Elemente (z. Bsp. Kondensatoren) die nach einem ganz
 * bestimmten Schlüssel (Kapazitätsangabe) sortiert werden müssen. Das Vorgehen
 * kann in folgenden Schritten beschrieben werden. Hierbei soll angenommen
 * werden, dass alle Elemente in einer Reihe liegen.
 *
 *
 * A multi byte txStream could only be used to address contiguous memory. After
 * the FPGA backend read a byte from the stream, the memory pointer becomes
 * incremented by one and the next byte read from the stream would be placed
 * adjacent thru the last one. There for, the multiple union structs must be
 * sorted for they're register address. Non-adjacent addressing union structs
 * have to splited into multiple blocks after sorting
 */
void VisaReg::sortForAddrs( QVector<HwReg* > &data) {
   QVector< HwReg* >::iterator it1,it2;

   /** HwReg object as auxiliary calculation variable */
   HwReg *_ac;

   for( it1 = data.begin()+1; it1 != data.end(); it1++) {
      _ac = (*it1);
      for( it2 = it1; it2 != data.begin(); it2--) {
         if( (*(it2 - 1))->getAddr() < _ac->getAddr())
            break;
         *it2 = *(it2 - 1);
      }
      (*it2) = _ac;
   }
}
QByteArray VisaReg::assambleStream( QVector<HwReg*> &regsV,
                                    bool subdividedStream ) {

   /** Iterator for the register vector parameter */
   QVector<HwReg*>::iterator it;

   /** Init register iterator */
   it = regsV.begin();

   /** Generate / Init txStream */
   QString blks;
   blks.append( INIT_WRITE_REG );

   /** Deriving the first start address passed by the sorted vector */
   uint16_t adjac = (*it)->getAddr() - 2;

   short blkCtr = 0; //i = (find( names.begin(), names.end(), s ) - names.begin());

   /**
    * Iterate thru the std::vector< HwReg* > vector object
    */
   for (it = regsV.begin(); it < regsV.end(); it++) {
      /**
       * If next reg.addr is not adjacent to the last one, new address and and
       * byte counter needs to be assembled into the stream
       */
      if(! (adjac + 1 == (*it)->getAddr())) {
         /**
          * If the stream still contains the %BLOCK identifier, this is the
          * first iteration. Nevertheless, this identifier is replaced at the
          * end of itteration when the block count is known
          */
         if (blks.contains("%BLOCKS", Qt::CaseSensitive)) {
            /**
             * Place code that depends on wether this is the first iteration
             */
         }
         /**
          * Append the next start address in front of the 16 Bit block
          * bytes counter
          */
         blks.append( QString(" %1 %2 ")
                      .arg((*it)->size(), 4, 16, QLC)     /** 16 Bit uint */
                      .arg((*it)->getAddr(), 2, 16, QLC));    /** 8-Bit uint */
         adjac = (*it)->getAddr() - 1;
         ++blkCtr;

         /**
          * Places payload in case of adjacent register adresses
          */
         blks.append(QString("%1")
                     .arg((*it)->getData(), 2*(*it)->size(), 16, QLC ));
      }
      else {
         /**
          * This branch places the payload in the non-adjacent case
          */
         blks.append(QString(" %1")
                     .arg((*it)->getData(), 2*(*it)->size(), 16, QLC ));
         qDebug() << (*it)->getData();
         /**
          * increment adjacent addr by size(reg)
          */
      }
      adjac += (*it)->size();
   }

   blks.replace("%START", FRAMEHEAD.remove(" "));
   blks.replace("%BLOCKS", QString("%1").arg(blkCtr, 2, 16, QLC));
#ifndef STREAM_HUMAN_READABLE
   blks.remove(" ");
   return blks.remove("|");
#else
   if (subdividedStream)
      return blks.replace("  ", " ").toLatin1();
   else
      return QString(blks).remove(" ").toLatin1();
#endif
}

/* ======================================================================== */
/*                Transmitter receiver exchange                             */
/* ======================================================================== */
/*!
 \brief Transmitter - Receiver exchange. Transceiver exchange initiates a
 request from visa hardware (uC, FPGA). Input parameters are an assembled
 req stream and the name of the related slot function as string literal.
 The class of the slot function which is used to process the rxStream is
 restricted to this (VisaReg).

 \param rawReq
 \param slotName
 \param nBytes
 \param queable If true, the actual request could be cueued if necessary.
 \return VisaReg::trxSts
 */
VisaReg::trxSts VisaReg::trxVisa(QByteArray rawReq, QString slotName,
                                 quint32 nBytes, bool queable) {
   bool ok;
   //< Check vector trxQue for already qued transceiver operations
   if (! trxQue.isEmpty() ) {

      /** If this call is initiated by a previously installed queue guard,
       * rawReq, slotName... MUST be empty. If they are not, this call
       * should be handled as error
       */
      if (! rawReq.isEmpty()) {
         Q_INFO << tr("VisaReg::trx_que_overrun");
         trxQue.removeLast();
         driver->rxBuffReq.flush();
         return VisaReg::trx_que_overrun;
      }

      if ( trxQue.length() < 2 ) {
         disconnect(driver, SIGNAL(reqRespKillTimeout()),
                    this,   SLOT(trxVisa()));
      }

      rawReq   = trxQue.last().rawReq;
      slotName = trxQue.last().slotName;
      nBytes   = trxQue.last().nBytes;
      trxQue.removeLast();
   }

   /** Check the rxBuffer structure against not acknowledged overrun errors */
   if (driver->rxBuffReq.expectedByteCount < 0) {
      Q_INFO << tr("VisaReg::trx_NACKed_Overrun");
      return VisaReg::trx_NACKed_Overrun;
   }

   /** Check the rxBuffer structure against uncompleted request and connect
    * this slot to the trx kill-running-timeout-timer if necessary. The
    * consequence behavior is that the call-back acts like a queue guardian if
    * the parameter of this canceled trxVisa(..) call are appended to the
    * trxQue structure;
    */
   if (driver->rxBuffReq.expectedByteCount > 0) {
      if (queable) {
         connect(driver, SIGNAL(reqRespKillTimeout()),
                 this,   SLOT(trxVisa()));
         trxQue_t quereq;
         quereq.rawReq = rawReq;
         quereq.slotName = slotName;
         quereq.nBytes = nBytes;

         trxQue.append( quereq );
      }
      driver->rxBuffReq.flush();
      Q_INFO << tr("VisaReg::trx_running_req");
      return VisaReg::trx_running_req;
   }

   /** Register the count of expected bytes returned by raw rxStream
    * The expected count "could be" derived from rawTxStream in case of default
    * registers are requested ...0047e0 --> nBytes := 0x47
    */
   if (nBytes) {
      driver->rxBuffReq.expectedByteCount = nBytes;
   }
   else if ( rawReq.right(1).contains(0xe0) ) {
      driver->rxBuffReq.expectedByteCount =
            rawReq.mid(rawReq.length() - 3, 2).toHex().toInt(&ok, 16);
   }
   else {
      return VisaReg::trx_bad_numOfExpRx;
   }

   /**
    * Since at any given time, only one receiver is allowed to to be connected
    * to the reqResponseCmpl() signal, this test must never evaluates true.
    */
   if (receivers( SIGNAL(reqResponseCmpl(QByteArray&)) ) > 0) {
      Q_INFO << tr("VisaReg::trx_multiple_recvrs");
      qWarning();
      return VisaReg::trx_multiple_recvrs;
   }

   /** Connect the related slot to the reqResponseCmpl() driver signal
    * The function returns a QMetaObject::Connection that represents a
    * handle to a connection if it successfully connects the signal to the
    * slot. The connection handle will be invalid if it cannot create the
    * connection, for example, if QObject is unable to verify the existence of
    * either signal or method, or if their signatures aren't compatible. You
    * can check if the handle is valid by casting it to a bool.
    */
   QMetaObject::Connection chkHandle =
         connect (driver, SIGNAL(reqResponseCmpl(QByteArray&)),
                  this, QString("1" + slotName).toLatin1());

   /** Initiate communication if the connection handle is valied. If not, do not
    * initiate communication but return false boolean
    */
   if ((bool)chkHandle) {

      /** If writeData returns -1, something goes wrong while try to initiate
       * communication with fpga --> disconnect the slot to avoid multiple
       * connections between identical objects!!!
       */
      if (! driver->writeData( rawReq )) {
         /** 15-10-2015
          * driver->serial->waitForBytesWritten(1) auskommentiert, irgendwie
          * fehlen die ersten zwei bytes vom rx stream (vergl. v2.1, da passts

         driver->serial->waitForBytesWritten(1);
         */
      }
      else {
         Q_INFO << tr("driver returned error while processing write command. "
                      "Running trx request must be cancelled, disconnect "
                      "specific slot.");

         /** Disconnect a specific receiver: */
         if (! driver->disconnect(this)) {
            Q_INFO << tr("QMetaObject disconnect handle returned false. This "
                         "means, the disconnection command also fails :-(");
         }
      }
   }
   else {
      Q_INFO << tr("VisaReg::trx_bad_con_handle");
      return VisaReg::trx_bad_con_handle;
   }

   /** After communication init has been processed, start a timeout timer to
    * reset the trx error mechanism.
    */
   timReqTimeout->singleShot(10, Qt::PreciseTimer, this,
                             SLOT(onReadReqTimeout()));

   return VisaReg::trx_success;
}
/**
 * Request basic visascope infos from microcontroller and write them
 * to structure visaInfo
 */
int VisaReg::reqVisaInfoFromMicro() {
   QString txSeqInfo = HEAD_UC_INFO;
   QByteArray trxReq;

   /** Append testbytes to the "Get info from uC" reqStream */
   txSeqInfo.append( TESTBYTES );

   /**
 * TODO: Die Abfrage rxBuffReq.expectedByteCount > 0 im slot readData kann
 * entfernt werden wenn zu jedem request ein eigener slot connected wird.
 */

   /**
    * By setting .expectedByteCount to a value != 0, the readyRead slot runs
    * a separate branch for request receive check
    *
    * Becomes depricted, see TODO!
    */
   trxReq.append( visa->convert(txSeqInfo), txSeqInfo.length()/2 );

   Q_ASSERT(trxReq.length() > 0);
   /** Initiate transceiver receiver exchange, register a rx count of 12bytes
    * as the condition for emitting requestComplete() signal. As the overall
    * slot for reqVisaInfoFromMicro(), string literal
    * "onReadRespInfoFromUc(QByteArray*)" is used for connection:: call
    */
   VisaReg::trxSts state =
         trxVisa(trxReq, "onRxRespInfoUcCmpl(QByteArray&)", 12);

   if (state != VisaReg::trx_success) {
      qDebug() << "trxVisa Error";
      return -1;
   }

   return 0;
}
bool VisaReg::procVisaInfo(QByteArray& raw) {
   //*NPTR*//  bool VisaReg::procVisaInfo(QByteArray& raw) {
   quint8 i;
   bool *ok = false;

   QByteArray txTestbyte, rxTestbyte;
   txTestbyte.append( visa->convert(TESTBYTES), TESTBYTES.length() );
   rxTestbyte = raw.mid(6, 4);

   /** Check received test bytes, they must contain ones-complement of
     * hey coded string literal TESTBYTES
     * ones-complement = 0xff - testbyteX
     */
   for ( i=0; i < rxTestbyte.length(); i++ ) {
      if (! rxTestbyte[i] == (quint8) 0xff - txTestbyte[i])
         return false;
   }

   visa->visaInfo.uC_FW =
         QString::number(raw.mid(4,1).toHex().toInt(ok, 16)) + "." +
         QString::number(raw.mid(5,1).toHex().toInt(ok, 16));

   visa->visaInfo.SerialNo       = raw.mid(6,4).toHex().toInt(ok, 16);
   visa->visaInfo.BaudrateCode   = raw.mid(10, 1).toInt();
   visa->visaInfo.Scope_State    = raw.mid(11, 1).toInt();

   //   visa->footerRefreshVisa();
   return true;
}

/* ======================================================================== */
/*                Tx to registers implementations                           */
/* ======================================================================== */
/*!
 \brief  write to LED register 0xbe and VLOGIC 0xbf
*/
void VisaReg::testWriteReg() {

   Reg_8087 *regSetPwr     = new Reg_8087(); Q_UNUSED(regSetPwr);
//   Reg_bebf *regLedVlogic  = new Reg_bebf();
//   Reg_8a8b *regDvmRngInp  = new Reg_8a8b();

   QVector<HwReg*> hwRegs;
   QVector<HwReg*>::iterator it;

   //   hwRegs.push_back( regSetPwr );
   hwRegs.push_back( regSetPwr );
   for ( it = hwRegs.begin(); it != hwRegs.end(); *it++) {
      (*it)->AboutRegister();
   }
   for ( it = hwRegs.begin(); it != hwRegs.end(); *it++) {
      qout() << "Union size:" << (*it)->size();
   }


   regSetPwr->h8087_pows.Vplus_set = 4000;
   regSetPwr->h8087_pows.ILim_plus = 3000;
   regSetPwr->h8087_pows.Vminus_set = 3800;
   regSetPwr->h8087_pows.ILim_minus = 2500;

   /*
   regDvmRngInp->h8a8b_rng.AD2_on   = 0x01;
   regDvmRngInp->h8a8b_rng.AD2_T    = 0x01;
   regDvmRngInp->h8a8b_rng.M2_rng   = Dvms::Input_single;
   regDvmRngInp->h8a8b_rng.M1_rng   = Dvms::Input_single;

         regDvmRng->h8a8b_rng.M1_rng         = Reg_8a8b::Range20V;
         regDvmRng->h8a8b_rng.M2_rng         = Reg_8a8b::Range20V;
         regDvmRng->h8a8b_rng.M1_inpt        = Reg_8a8b::Input_single;
         regDvmRng->h8a8b_rng.M2_inpt        = Reg_8a8b::Input_single;

         sortForAddrs(hwRegs);
         QByteArray ret = assambleStream(hwRegs, false);
         QByteArray retRaw;
         retRaw.append( convert( ret ), ret.length()/2);

         qDebug() << "\t" << ret;
         qDebug() << "\t" << QString(ret).remove(" ");
         qDebug() << "\t" << retRaw.toHex();

      //   serial.write(ret);
      //   serial.waitForBytesWritten(1);
         serial.write(retRaw);
         serial.waitForBytesWritten(10);
      */
   QByteArray ret = writeToReg(hwRegs, VisaReg::RetFormBIN);

}
void VisaReg::testWriteReg2() {

//   Reg_8087 *regSetPwr     = new Reg_8087(); Q_UNUSED(regSetPwr);
//   Reg_bebf *regLedVlogic  = new Reg_bebf();
   Reg_8a8b *regDvmRngInp  = new Reg_8a8b();

   QVector<HwReg*> hwRegs;
   QVector<HwReg*>::iterator it;

   //   hwRegs.push_back( regSetPwr );
   hwRegs.push_back( regDvmRngInp );
   for ( it = hwRegs.begin(); it != hwRegs.end(); *it++) {
      (*it)->AboutRegister();
   }
   for ( it = hwRegs.begin(); it != hwRegs.end(); *it++) {
      qout() << "Union size:" << (*it)->size();
   }

   regDvmRngInp->h8a8b_rng.AD2_on   = 0x01;
   regDvmRngInp->h8a8b_rng.AD2_T    = 0x01;
   regDvmRngInp->h8a8b_rng.M2_rng   = Dvms::Input_single;
   regDvmRngInp->h8a8b_rng.M1_rng   = Dvms::Input_single;


   /*
         regSetPwr->h8087_pows.Vplus_set = 0x0400;
         regSetPwr->h8087_pows.ILim_plus = 0x0200;
         regSetPwr->h8087_pows.Vminus_set = 0x0800;
         regSetPwr->h8087_pows.ILim_minus = 0x0200;

         regDvmRng->h8a8b_rng.M1_rng         = Reg_8a8b::Range20V;
         regDvmRng->h8a8b_rng.M2_rng         = Reg_8a8b::Range20V;
         regDvmRng->h8a8b_rng.M1_inpt        = Reg_8a8b::Input_single;
         regDvmRng->h8a8b_rng.M2_inpt        = Reg_8a8b::Input_single;

         sortForAddrs(hwRegs);
         QByteArray ret = assambleStream(hwRegs, false);
         QByteArray retRaw;
         retRaw.append( convert( ret ), ret.length()/2);

         qDebug() << "\t" << ret;
         qDebug() << "\t" << QString(ret).remove(" ");
         qDebug() << "\t" << retRaw.toHex();

      //   serial.write(ret);
      //   serial.waitForBytesWritten(1);
         serial.write(retRaw);
         serial.waitForBytesWritten(10);
      */
   QByteArray ret = writeToReg(hwRegs, VisaReg::RetFormBIN);

}
/* ======================================================================== */
/*                Tx request slot implementations                           */
/* ======================================================================== */
/* ======================================================================== */
int VisaReg::reqDefaultRegs() {
#define NUM_REGS  0x8c
   QByteArray trxReq, trxReqRaw;
   trxReq.clear();
   trxReqRaw.clear();

   /** Assemble standard req stream */
   trxReq.append( HEAD_DEFAULT_REQ.remove(" ")
                  .replace("%NUM_READ", tr("%1").arg(NUM_REGS, 4, 16, QLC)));

   if (false) {
      trxReq = QString(trxReq)
            .replace(tr("%NUM_READ"), tr("%1"))
            .arg(NUM_REGS, 4, 16, QLC)
            .remove(" ")
            .toLatin1();
   }

   /** Convert ASCII coded stream to binary representation */
   trxReqRaw.append( visa->convert(trxReq), trxReq.length()/2 );

   Q_ASSERT(trxReqRaw.length() > 0);

   if( trxVisa(trxReqRaw, "onRxRespDefaultReadCmpl(QByteArray&)", NUM_REGS)
       != VisaReg::trx_success) {
      qWarning();
      return -1;
   }

   return 0;
}
/* ======================================================================== */
/* ======================================================================== */
void VisaReg::testReadReg() {
   QByteArray trxReq, trxReqRaw;
   trxReq.append(INIT_READ_REG_REQUEST.replace(
                    "%START", FRAMEHEAD));

   bool ok;
   //   int ret = QInputDialog::getInt(this, tr("QInputDialog::getInt()"),
   //                                  tr("Number of regs to read:"),
   //                                  1,1,254,1,&ok);
   ok = true;
   int ret = 40;

   if (ok)
      trxReq = QString(trxReq)
            .replace(tr("%NUM_READ"), tr("%1"))
            .arg(ret, 4, 16, QLC)
            .remove(" ")
            .toLatin1();

   trxReqRaw.append( visa->convert(trxReq), trxReq.length()/2 );
   Q_ASSERT(trxReqRaw.length() > 0);

   VisaReg::trxSts state = trxVisa(trxReqRaw,
                                   "onRxRespTestReadCmpl(QByteArray&)", ret);

   if (state != VisaReg::trx_success)
      qDebug() << "trxVisa error";
}
void VisaReg::reqDvmADC() {
#define READ_N_BYTES    0x20
   qWarning();

   QByteArray trxReq, trxReqRaw;
   /**
    * Setup initial string literal for trx streams
    */
   trxReq.append(INIT_READ_REG_REQUEST
                 .replace("%START", FRAMEHEAD));
   /**
    * Insert requested byte count
    */
   trxReq = QString(trxReq).replace(tr("%NUM_READ"), tr("%1"))
         .arg(READ_N_BYTES, 4, 16, QLC).remove(" ").toLatin1();

   trxReqRaw.append( convert(trxReq), trxReq.length()/2 );

   Q_ASSERT(trxReqRaw.length() > 0);

   VisaReg::trxSts state =
         trxVisa(trxReqRaw, "onRxRespDvmCmpl(QByteArray&)",READ_N_BYTES);

   if (state != VisaReg::trx_success)
      qDebug() << "trxVisa error";
}
int VisaReg::reqCalibDataFromAtiny(bool onoff) {
#define NUM_CAL_READ  128

   if (! onoff) {
      qDebug() << tr("triggered low!");
      return -1;
   }

   QByteArray trxReqRaw,
         trxReq = (HEAD_READ_CAL_VAL.replace("%S", "00")
                   .replace("%N", QString::number(NUM_CAL_READ, 16))
                   .remove(" ")
                   .toLatin1());

   /** convert hex coded char sequence to its binary representation */
   trxReqRaw.append( visa->convert(trxReq), trxReq.length()/2 );

   /** Assert on conversion error */
   Q_ASSERT(trxReqRaw.length() > 0);

   /** Initiate tranceiver exchange and check if the MetaObject connection
    * handle is valied due to the slot function is passed as string literal
    */
   if (trxVisa(trxReqRaw, tr("onRxRespCalibData(QByteArray&)"), NUM_CAL_READ)
       != VisaReg::trx_success) {
      qDebug() << tr("trxVisa error");
      return -1;
   }
   else {
      visa->gs.calibrated = true;
      return 0;
   }
}

/* ======================================================================== */
/*                Rx response slot implementations                          */
/* ======================================================================== */
void VisaReg::onRxRespDefaultReadCmpl(QByteArray& rxdata) {

   /** First of all, disconnect this slot from readyRead() driver signal */
   disconnect(driver, SIGNAL(reqResponseCmpl(QByteArray&)),
              this,   SLOT(onRxRespDefaultReadCmpl(QByteArray&)));

//   beVerbose(rxdata);
//   return;

   qDebug() << rxdata.toHex();

   /**
    * Split and write back unioned stream data
    */
   uint32_t ram = (uint32_t) rxdata.mid(0x06, regActRamTrig->size()).toInt();
   qDebug() << "ram 0609: " << ram;
   regActRamTrig->h0609_RAM_Addr.dword = ram;

   uint32_t trigEv = (uint32_t) rxdata.mid(0x0a, regTrigEvDist->size()).toInt();
   qDebug() << "trig ev: " << trigEv;
   regTrigEvDist->h0a0d_trig_ev.dist = trigEv;

   uint64_t AdcPwr = (uint64_t) rxdata.mid(0x10, regAdcPwr->size()).toInt();
   qDebug() << "AdcPwr: " << AdcPwr;
   regAdcPwr->h1017_adcPwr.lword = AdcPwr;

   uint64_t AdcDvm = (uint64_t) rxdata.mid(0x18, regAdcDvm->size()).toInt();
   qDebug() << "AdcDvm: " << AdcDvm;
   regAdcDvm->h181f_dvm.lword = AdcDvm;

   uint16_t ledVlog = (uint16_t) rxdata.mid(0xbe, regLedVlogic->size()).toInt();
   qDebug() << "ledVlog: " << ledVlog;
   regLedVlogic-> hbebf_led.word = ledVlog;

   uint64_t setPwr = (uint64_t) rxdata.mid(0x80, regSetPwr->size()).toInt();
   qDebug() << "setPwr: " << setPwr;
   regSetPwr->h8087_pows.lword = setPwr;

   uint16_t DvmRng = (uint16_t) rxdata.mid(0x8a, regDvmRngInp->size()).toInt();
   qDebug() << "DvmRng: " << DvmRng;
   regDvmRngInp-> h8a8b_rng.word = DvmRng;


   /* ===================================================================== */
   QString rxdataStr = rxdata.toHex();
   static qint16 nOverwrite = 0;

   /** Local shortcut */
   Reg_181f::h181f_dvm_t rdvm = regAdcDvm->h181f_dvm;

   /** External supply voltage */
   double V_in =
         (rdvm.ADC_Vbias - rdvm.ADC_gnd) * H[14];

   /** Common scale factor */
   double SCAL =
         calc->cCalib[80].Hd / calc->cCalib[70].Hd;

   /** Calc all caombinations page 45/46 */
   QVector<double> dvms;
   for (int j=21; j<27; j++) {
      dvms.append(( (rdvm.ADC_avg & 0x3fff) -
                    (rdvm.ADC_sel_gnd & 0x3fff) +
                    H[j]) * SCAL * H[j-6]);
   }

   QString mode =
         tr("Dvm1 range 0.2V: #Dvm1 range 2.0V: #Dvm1 range 20V : #") +
         tr("Dvm2 range 0.2V: #Dvm2 range 2.0V: #Dvm2 range 20V : #");
   QStringList lst = mode.split("#");

   ioeditL->putInfoLine(QString::number(V_in), "V_in  :");

   for (int j=0; j<dvms.length(); j++)
      ioeditL->putInfoLine(QString::number(dvms[j]), lst.at(j));

   if (nOverwrite < 0)
      nOverwrite = ioeditL->blockCount();

   ioeditL->putRxData( rxdataStr );

   Dvm *DvmDC     = Dvm::getObjectPtr(dvmType_DC);
   Dvm *DvmACDC   = Dvm::getObjectPtr(dvmType_ACDC);

   /** Dvm lcd refresh */
   if ( (DvmDC != 0) && (DvmACDC != 0) ) {
      DvmDC->setLCDval(dvms.at(1));
      DvmACDC->setLCDval(dvms.at(1));
   }
}
void VisaReg::beVerbose(QByteArray& rxdata) {
   bool ok;
   static qint16 nOverwrite = 0;

   //   QString rxBuffStr = driver->rxBuffReq.data.toHex();
   QString rxdataStr = rxdata.toHex();
   QByteArray rxPart1017 = rxdata.mid(0x10, sizeof(uint64_t)).toHex();
   QByteArray rxPart181f = rxdata.mid(0x18, sizeof(uint64_t)).toHex();

   ///////////////////////////
   // V+   I+    V-   I-
   // cc7d 009b 804e c87d
   ///////////////////////////
   // V_in AVG  GND  SEL_GND
   // 007d 009e 4396 7691
   ///////////////////////////
//   qDebug() << rxdataStr;
#ifdef VERBOSITY_LVL_5
   Q_INFO << "part 0x10..0x17:" << rxPart1017;
   Q_INFO << "part 0x18..0x1f:" << rxPart181f;
#endif

   /*
   QRegExp text_to_find("INFO*Calibration", Qt::CaseInsensitive);
   text_to_find.setPatternSyntax(QRegExp::Wildcard);
   QTextCursor find_result = ioeditL->document()->find(text_to_find);

   if (! find_result.isNull()) {
      ioeditL->clear();
      nOverwrite = -1;
   }
   else {
      QTextBlock block = ioeditL->document()->begin();
      for (uint i = 0; i < ioeditL->blockCount()+1; i++) {
           QTextCursor cursor(block);
           cursor.select(QTextCursor::BlockUnderCursor);
           block = block.next();

           QRegExp text_to_find("@:", Qt::CaseInsensitive);
           QTextCursor find_result = ioeditL->document()->find(text_to_find);

           if (! find_result.isNull())
              cursor.removeSelectedText();
      }
      QTextCursor cur = ioeditL->textCursor();
      cur.document()->begin();
      Q_INFO << cur.position() << cur.positionInBlock();
   }
*/

   if (visa->getUiClearConsoleIsChecked())
      ioeditL->clear();

   uint32_t tmp0 = ((uint32_t)
                    (rxdata[0x10] << 24) +
         (rxdata[0x11] << 16) +
         (rxdata[0x12] << 8) +
         rxdata[0x13]) & ~0xc000;

   uint32_t tmp1 = ((uint32_t)
                    (rxdata[0x14] << 24) +
         (rxdata[0x15] << 16) +
         (rxdata[0x16] << 8) +
         rxdata[0x17]) & ~0xc000;
   // cc7c3f9b804d087d (tmp0 << tmp1)

   //   quint64 tmp1 = (quint64) rxPart.toHex().toInt(ok, 16);
   //   quint64 tmp2 = (quint64) rxPart.toInt(ok, 16);
   regAdcPwr->h1017_adcPwr.lword = (uint64_t) (tmp0 << 31) + tmp1;
   regAdcPwr->h1017_adcPwr.lword = rxPart1017.toLongLong(&ok, 16);
   regAdcDvm->h181f_dvm.lword    = rxPart181f.toLongLong(&ok, 16);
   Reg_181f::h181f_dvm_t rdvm = regAdcDvm->h181f_dvm;

   double V_in = (rdvm.ADC_Vbias - rdvm.ADC_gnd) * H[14];
   double SCAL = calc->cCalib[80].Hd / calc->cCalib[70].Hd;
#ifdef VERBOSITY_LVL_5
   Q_INFO << tr("scale factor H[80]/H[70] =") << SCAL;
#endif
   /*
   dvm1_02 = (rdvm.ADC_avg - rdvm.ADC_sel_gnd + HH[21]) * fact * HH[15];
   dvm1_2  = (rdvm.ADC_avg - rdvm.ADC_sel_gnd + HH[22]) * fact * HH[16];
   dvm1_20 = (rdvm.ADC_avg - rdvm.ADC_sel_gnd + HH[23]) * fact * HH[17];

   dvm2_02 = (rdvm.ADC_avg - rdvm.ADC_sel_gnd + HH[24]) * fact * HH[18];
   dvm2_2  = (rdvm.ADC_avg - rdvm.ADC_sel_gnd + HH[25]) * fact * HH[19];
   dvm2_20 = (rdvm.ADC_avg - rdvm.ADC_sel_gnd + HH[26]) * fact * HH[20];
*/
   QString mode =
         tr("Dvm1 range 0.2V: #Dvm1 range 2.0V: #Dvm1 range 20V : #") +
         tr("Dvm2 range 0.2V: #Dvm2 range 2.0V: #Dvm2 range 20V : #");
   QStringList lst = mode.split("#");

   QVector<double> dvms;
   for (int j=21; j<27; j++)
      dvms.append(( (rdvm.ADC_avg & 0x3fff) -
                    (rdvm.ADC_sel_gnd & 0x3fff) +
                    H[j]) * SCAL * H[j-6]);

   if (! visa->getUiSuppressDefaultPutIsChecked()) {
      ioeditL->putInfoLine(QString::number(V_in), "V_in  :");

      for (int j=0; j<dvms.length(); j++)
         ioeditL->putInfoLine(QString::number(dvms[j]), lst.at(j));

      if (nOverwrite < 0)
         nOverwrite = ioeditL->blockCount();

      ioeditL->putRxData( rxdataStr );
   }
   Dvm *DvmDC     = Dvm::getObjectPtr(dvmType_DC);
   Dvm *DvmACDC   = Dvm::getObjectPtr(dvmType_ACDC);

   if ( (DvmDC != 0) && (DvmACDC != 0) ) {
//      DvmDC->updateDvmConfig();
//      DvmACDC->updateDvmConfig();

      DvmDC->setLCDval(dvms.at(1));
      DvmACDC->setLCDval(dvms.at(1));
   }

   /* ---------------------------------------------------------------- */
   /*                      next try                                    */
   /* ---------------------------------------------------------------- */

//   QString str;
//   QVector<quint16> vr;
//   QVector<quint16>::iterator it;
//   Driver *driver = Driver::getObjectPtr();
#ifdef VERBOSITY_LVL_5
   qDebug().noquote() << tr("0x1a1b")
                      << driver->rxBuffReq.data.mid(0x1a, 2).toHex().toInt(&ok, 16);
#endif
   //   for (int k = 0; k < 22; k += 2) {
   //      vr.append( driver->rxBuffReq.data.mid(0x10 + k, 2)
   //                 .toHex().toInt(&ok, 16) );
   //      str = QString("%1 ").arg(vr.first(), 4, 16, QLC);
   //      ioeditL->putRxData( str );
   //   }

}

/* ======================================================================== */
/* ======================================================================== */
void VisaReg::onRxRespInfoUcCmpl(QByteArray& rxdata) {
   /** First of all, disconnect this slot from readyRead() driver signal */
   disconnect(driver, SIGNAL(reqResponseCmpl(QByteArray&)),
              this,   SLOT(onRxRespInfoUcCmpl(QByteArray&)));

   Q_INFO << "was called with rxdata (hxed): " << rxdata.toHex();

   QString str;

   /**
    * False if received test bytes are not the ones complement
    * of the transmitted test bytes
    */
   if ( procVisaInfo(driver->rxBuffReq.data) )
      str = QString("info from uC: %1 0x%2 ")
            .arg( visa->visaInfo.SerialNo, 0, 10 )
            .arg( visa->visaInfo.SerialNo, 8, 16, QLC);
   else
      str.append("Testbytes not ones-complements");

   ioeditL->putInfoLine( str );

   QString rxBuffStr = driver->rxBuffReq.data.toHex();
   ioeditL->putRxData( rxBuffStr );
}
void VisaReg::onRxRespTestReadCmpl(QByteArray& rxdata) {
   /** First of all, disconnect this slot from readyRead() driver signal */
   disconnect(driver, SIGNAL(reqResponseCmpl(QByteArray&)),
              this,   SLOT(onRxRespTestReadCmpl(QByteArray&)));

   Q_INFO << "was called with rxdata (hxed): " << rxdata.toHex();

   QString rxBuffStr = driver->rxBuffReq.data.toHex();
   ioeditL->putRxData( rxBuffStr );
   //   driver->rxCircular();
}
void VisaReg::onRxRespDvmCmpl(QByteArray& rxdata) {
   Q_UNUSED(rxdata)
   /** First of all, disconnect this slot from readyRead() driver signal */
   disconnect(driver, SIGNAL(reqResponseCmpl(QByteArray&)),
              this,   SLOT(onRxRespDvmCmpl(QByteArray&)));


   //   driver->rxCircular();
}
void VisaReg::onRxRespCalibData(QByteArray& rxdata) {
   /** First of all, disconnect this slot from readyRead() driver signal */
   disconnect(driver, SIGNAL(reqResponseCmpl(QByteArray&)),
              this,   SLOT(onRxRespCalibData(QByteArray&)));
   //   /** Disconnect the receiver onReadReqTimeout() from timeout timer */
   //   disconnect(driver, SIGNAL(reqResponseCmpl(QByteArray&)),
   //              this,   SLOT(onRxRespCalibData(QByteArray&)));

   /** Since at any given time, only one receiver is allowed to to be connected
    * to the reqResponseCmpl() signal, this test must never evaluates true. */
   if (receivers( SIGNAL(reqResponseCmpl(QByteArray&)) ))
      qWarning();

   /** Convert the calibration byte stream to uint16_t vector */
   /**calc->eepromRx*/
   Utype::calDatRaw_t calRawTmp = convertCalib( rxdata.toHex(), 4 );
   calc->eepromRx = &calRawTmp;

   //   Q_INFO << calc->eepromRx;

   QString rxBuffStr = rxdata.toHex();
   ioeditL->putRxData( rxBuffStr );

   /**
    * Now we can correct the H(i) hardware dependent factors. Emitting signal
    * calibration info available. Prior to this task, we will merge the
    * received tolerance vector into the container data type, already furnished
    * by the hardware_xxx.dat load process.
    *
    * For the knowladge about how the tolerance data and the calibration data
    * must be merged together, we have to refer to the "VisaScope - Programmers
    * manual" page 39-40
    */
   emit calibInfoAvailable();
}

/* ======================================================================== */
/*                     Instance initialization                              */
/* ======================================================================== */
void VisaReg::init_HwRoImg(bool initVal) {
   uint8_t by = 0x00; uint32_t byby = 0x00000000;

   if (initVal) {
      by = 0xff;
      byby--;
   }

   roRegImg.h0001.word  =  (uint16_t)  (by << 8) | by;
   roRegImg.h0203.txed  =  (uint16_t)  (by << 8) | by;
   roRegImg.h04.byte    =  (uint8_t)   by;
   roRegImg.h05         =  (uint8_t)   by;
   roRegImg.h0609.dword =  (uint32_t)  byby;
   roRegImg.h0a0d.dist  =  (uint32_t)  byby;
   roRegImg.h1017.lword =  (uint64_t)  (byby << 31) | byby;
   roRegImg.h181f.lword =  (uint64_t)  (byby << 31) | byby;
   roRegImg.h2023.dword =  (uint32_t)  byby;
   roRegImg.h2427.dword =  (uint32_t)  byby;
   roRegImg.h2b.byte    =  (uint8_t)   by;
   roRegImg.h2c2f.dword =  (uint32_t)  byby;
}
void VisaReg::init_HwRwImg(bool initVal) {
   uint8_t by = 0x00; uint32_t byby = 0x00000000;

   if (initVal) {
      by = 0xff;
      byby--;
   }
   rwRegImg.h8047.lword =  (uint32_t) byby;
   rwRegImg.h8a8b.word  =  (uint32_t) byby;
   rwRegImg.hbebf.word  =  (uint16_t)  (by << 8) | by;
}

/* ======================================================================== */
/*                     pretty little helpers                                */
/* ======================================================================== */
/*!
 \brief  Convert QString/QByteArray to raw uint16_t QVector

 \param in
 \param nCharPack
 \return QVector<uint16_t>
*/
QVector<uint16_t> VisaReg::convertCalib(QString in,
                                        uint8_t nCharPack) {
   uint16_t i,k; QString str;
   bool ok; QVector<uint16_t> rawv;


   if ( (nCharPack != 4) ) {
      Q_INFO << "Parameter nCharPack must either be NOT(2), 4, NOT(8)!";
      return rawv;
   }

//   if ((in.remove(" ")).length() < in.length())
//      Q_INFO << "Whitespace truncated!";

   /**
    * We expect that in is a String that encodes a sequence of hex-coded
    * symbols "0004f408" --> 0x00 0x04 0xf4 0x08
    */
   if (in.length() % nCharPack) {
      Q_INFO << tr("in.length() mod(%1) > 0?!").arg(nCharPack);
      return rawv;
   }
   else {
      for (i=0; i < in.length(); i+=nCharPack) {
         str = in.at(i);
         for (k = 1; k < nCharPack; k++)
            str.append( in.at(i + k) );

         rawv.append( (uint16_t) str.toInt(&ok, 16));
      }
      return rawv;
   }
}
/*!
 \brief Convert a char sequence that encodes hexadecimal bytes to a QByteArray
 Example call: QByteArray locSTARTSEQ; locSTARTSEQ.append((char *)
 convert(vseq.STARTSEQ), vseq.STARTSEQ.length()/2); or: seqInfoArr.append(
 convert(seqInfo), seqInfo.length()/2 );

 \param in
 \return char
*/
char *VisaReg::convert(QString in) {
   uint16_t i;
   QString str;
   bool ok;
   char * raw;

   int le = in.length()/2;
   raw = (char *) calloc(le, 1);

//   if ((in.remove(" ")).length() < in.length()) {
////      Q_INFO << "Whitespace truncated!";
//   }

   /**
    * We expect that in is a String that encodes a sequence of hex-coded
    * symbols "0004f408" --> 0x00 0x04 0xf4 0x08
    */
   if (in.length() % 2) {
      Q_INFO << "in.length() mod(2) > 0?!";
      return NULL;
   }
   else {
      for (i=0; i < in.length(); i+=2) {
         str = in.at(i);
         str.append( in.at(i+1) );

         *raw = (char) str.toInt(&ok,16);
         raw++;
      }
      return (raw - le);
   }
}

/**
 * Convert a char sequence that encodes hexadecimal bytes to a QByteArray
 * Example call:
 * QByteArray locSTARTSEQ;
 * locSTARTSEQ.append((char *) convert(vseq.STARTSEQ), vseq.STARTSEQ.length()/2);
 * or:
 * seqInfoArr.append( convert(seqInfo), seqInfo.length()/2 );
 */
char *VisaReg::convertVisa(QString in) {
   uint16_t i;
   QString str;
   bool ok;
   char * raw;

   int le = in.length()/2;
   raw = (char *) calloc(le, 1);

   if ((in.remove(" ")).length() < in.length()) {
      Q_INFO << tr("Whitespace truncated!\nRemember to truncate whitespaces of"
                   " attribute used to call (..).length/2!!!\n\n");
   }

   /**
    * We expect that in is a String that encodes a sequence of hex-coded
    * symbols "0004f408" --> 0x00 0x04 0xf4 0x08
    */
   if (in.length() % 2) {
      Q_INFO << "in.length() mod(2) > 0?!";
      return NULL;
   }
   else {
      for (i=0; i < in.length(); i+=2) {
         str = in.at(i);
         str.append( in.at(i+1) );

         *raw = (char) str.toInt(&ok,16);
         raw++;
      }
      return (raw - le);
   }
}
