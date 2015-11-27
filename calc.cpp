
#include "calc.h"
#include "hwreg.h"
#include "globals.h"
#include "filebackup.h"
#include "utype.h"

#include <QDebug>
#include <sstream>
#include <stdint.h>
#include <algorithm>
#include <QString>
#include <QDataStream>
#include <QInputDialog>
#include <QFile>

namespace name {
class HwReg;
class VisaReg;
class FileBackup;
}

class FileBackup;
class HwReg;

Calc* Calc::inst = 0;

/* ======================================================================== */
/*                     class constructor                                    */
/* ======================================================================== */
Calc::Calc(VisaReg *parent) :
   QObject(parent) {

   if (parent != 0)
      visareg = parent;

   ioedit   = IOEdit::getInstance();
   eepromRx = new QVector<uint16_t>(100);


   //   german = QLocale::German, QLocale::Germany;
   //   loadHardwareDat(DEFAULT_DAT_FILEORIG);
}

Calc::~Calc() {

}

void Calc::onCalibInfoAvailable() {
   if (mathAdjustment(cCalib, *eepromRx))
      Q_INFO << "call to mathAdjustment() returns error!";
   else
      ioedit->putInfoLine("Calibration successful");
}
/**
 * -------------------------------------------------------------------------
 *             VisaScope - Programmers Manual   page 37
 * -------------------------------------------------------------------------
 * The column EEPROM shows the position of the corresponding calibration
 * value C(i) inside the EEPROM. The EEPROM-value consists of two bytes; the
 * smaller adress-value is the MSB, the higher the LSB-byte. After starting
 * the VISASCOPE , you have to proceed the following steps:
 *
 *
 * 1.  Read all Hardw.-values H(i) as well as the corresponding tolerance-
 * information from the section [hardware_values] of the ﬁle
 * share\hardware_values\hardware_xxxxxx.dat . The placeholder xxxxxx
 * denotes the serial number of the ﬁrst Visascope using this hardware-ﬁle.
 * Example: Assuming Your Visascope has the serial number 136123. The ﬁle
 * to be used is hardware_136007.dat. The ﬁle hardware_137000.dat should
 * not be used. This ﬁle would be valid for serial numbers higher than
 * 137000.
 *
 * 2.  Read the content of the EEPROM (see chapter 2.3) and calculate all
 * Calibration-values corresponding to the following formula: C(i) =
 * MSB_byte 256 + LSB_byte                                  (7.1)  Thus,
 * you will have an unsigned integer number in the range from 0 to 65535
 * (216 return 0; }
 *
 * 3.  Read from the ﬁle share\hardware_values\hardware_xxxxxx.dat the
 * corresponding line. All lines contain after the semicolon an keyword
 * no, abs or res. Hardware-values with the keyword no has to be used as
 * they are. No Correction or modiﬁcations must be made.  Hardware-values
 * with the keyword rel or abs are values wich must be calibrated. The
 * Number after the Keyword (seperated with - sign) is the scale-factor T.
 *
 * Example: in the list above we read the line H(2) = 0.0010031 ; rel-5.
 * This means, we have an average value of 0.0010031 which has a relative
 * tolerance of ±5% (T=5 means ±5%) Depending from the type of tolerance,
 * calculate the tolerance-corrected hardware values using the following
 * equations:
 *
 * Type:  abs  The tolerance value is given as an absolute value and the
 * corrected Hard- ware value has to be calculated corresponding to the
 * following formula:
 *
 * H(i)_corr = H(i)_hardware_xxx.dat + T *[ C(i)/32000 - 1 ]
 *
 *
 * Type:  rel  The tolerance value is given als relative tolerance, and the
 * corrected Hard- ware value has to be calculated corresponding to the
 * following formula:
 *
 * H(i)_corr = H(i)_hardware_xxx.dat * (1 + T/100 * [ C(i)/32000 - 1 ])
 *
 * -------------------------------------------------------------------------
 *
 *
 * The call of the math-Adjustment methode needs all information (also the
 * non-numerical) from the hardware_xxx.dat file. Also the eeprom content
 * of visascopes ATtiny is needed to provide the corrected, hardware specific
 * values which are needed for proper conversion between digital and physical
 * measurement domains.
 */
int Calc::mathAdjustment( QVector<hw_calib_t> &hwCal,
                          QVector<uint16_t> &Ctol) {
   int i; bool ok = false;

   QVector<hw_calib_t>::iterator it;
   it = hwCal.begin();

   /** 16-10-2015    There are only Ctol.length() = 64 tolerance values
    * available (128 byte eeprom, 16bit words). But for the following for loop
    * we need a break condition of hwCal.length() because this loop also have to
    * copy the values which are not altered by a tolerance factor ( ; no)
    */
   for (i = 0; i < /*Ctol*/ hwCal.length(); i++) {
      if (i < Ctol.length() )
         it->C = Ctol.at( i );
      else
         it->C = 0x00;

      /** Check the last parameter of hardware_xxx.dat file. If "no" is given as
       * line end, this value don't have to be calibrated, else the value must be
       * calibrated. The string contains then "abs" or "rel" followed by an ascii
       * coded float expression which has the meaning of a scale factor.
       */
      if (it->note.indexOf("no") == -1) {
         it->type = it->note.contains("abs") ?
                  Calc::absolut  :  Calc::relativ;
      }
      else
         it->type = Calc::none;
      //      if (it->note.contains("no"))
      //         it->type = Calc::none;
      //      else if (it->note.contains("abs"))
      //         it->type = Calc::absolut;
      //      else if (it->note.contains("rel"))
      //         it->type = Calc::relativ;

      /** Convert the scale factor */
      QString tmp;
      if ( ! (tmp = it->note.remove( QRegExp("(\\D{2,3}-+)"))).isEmpty() ) {
         if (! tmp.contains("no")) {
            it->T = QString( tmp ).toDouble(&ok);

            if (!ok)
               qDebug() << tr("QString conversion failed!");// :
         }
         else
            it->T = 0;

      }

      /**
       * Type: relativ
       * H(i)_corr = H(i)_hardware_xxx.dat * (1 + T/100 * [ C(i)/32000 - 1 ])
       *
       * Type: absolut
       * H(i)_corr = H(i)_hardware_xxx.dat + T *[ C(i)/32000 - 1 ]
       */
      switch (it->type) {
         case Calc::relativ:  {
            it->H = it->Hd * (1 + it->T/100 * ((float) it->C/ 32000 - 1));
         }; break;

         case Calc::absolut:  {
            it->H = it->Hd + it->T * ((float) it->C/ 32000 - 1);
         }; break;

         case Calc::none:  {
            it->H = it->Hd;
         }; break;
         default: {
            qDebug() << "BAD CASE!!!!!";
         }; break;
      }

      it++;
   }

   /** put some important columns of the original and the corrected hw values
    * to the console windows */
   int k = 0; QString str;

   /** Table header row */
   ioedit->putInfoLine( tr("H(#): @H_alt @Hcorr @Note @Ctol"), "", 8, '@', '_');

   for(it = hwCal.begin(); it < hwCal.end(); it++, k++) {
      if ( hwCal[k].Hd ||  hwCal[k].H || (! hwCal[k].note.isEmpty()) ) {
         str.sprintf("H(%i):@%.4g@%.4g@%s@0x%02x",
                     k, hwCal[k].Hd, hwCal[k].H,
                     hwCal[k].note.toStdString().c_str(), hwCal[k].C);
         ioedit->putInfoLine( str, "", 10, '@', '_');
      }
   }

   /**
    * Disconnect any SIGNALS connected to ??? receiver
    * If you write disconnect(object), you’re really disconnecting all signals
    * received by object, NOT all signals sent by object.
    */
   disconnect(this);

   /** Backup the prior used hardware_xxx.dat file */
   /*QString bkpFile = */
   QString bkpFile = FileBackup::doBackup(DEFAULT_DAT_FILE);

   /** Reopen the prior used hardware_xxx.dat file and append the corrections */
   QFile fd(DEFAULT_DAT_FILE);
   if (! fd.open(QIODevice::ReadOnly | QIODevice::Text))
      return -1;

   /** Open temporary file */
   //   QFile fw(tr("/tmp/visacommand_") + QTime::currentTime()
   //            .toString("hh:mm:ss").remove(":"));
   QFile fw(tr("/tmp/visacommand_tmp"));

   if (! fw.open(QIODevice::ReadWrite | QIODevice::Text)) {
      fd.close();
      return -1;
   }

   QTextStream fout(&fw);
   QString line;

   while (! fd.atEnd()) {
      line = fd.readLine();
      if (line.contains("File generated at")) {
         fout << line;
         fout << "File modified  at " << (QDateTime::currentDateTime())
                 .toString("dd.MMMM yyyy - hh:mm:ss") << "\n";
         break;
      }
      else
         fout << line;
   }
   QRegExp rx("(\\d{1,2})");
   QStringList rxLst;
   int idx; QString s;

   while (! fd.atEnd()) {
      line = fd.readLine();

      if (line.contains("end"))
         break;

      rx.indexIn( line );
      rxLst = rx.capturedTexts();
      idx = rxLst.first().toInt(&ok);
      rxLst.clear();

      if (idx + 1 >= hwCal.length()) {
         Q_INFO << "Bad value found for indexing vector, return -1";
         return -1;
      }

      if (ok) {
         s.sprintf("  Hc(%i):\t% .5f", idx, hwCal[idx].H);

         line.insert( line.indexOf(";"), s);
         rxLst = line.split(";");
         rxLst.first().insert(45, tr("; ") + rxLst.last().remove(" "));
         fout << rxLst.first();   // << "\n";
      }
      else
         fout << line; // << "\n";
   }
   fd.close();
   fw.close();

   visareg->H.clear();
   /** single vector of tolerance adusted calibration data */
   for (int j=0; j < hwCal.length(); j++)
      visareg->H.append( hwCal[j].H );
   return 0;
}

int Calc::loadHardwareDat(QString path) {
   QStringList lst; QByteArray line;
   int i;

   cCalib.clear();
   cCalib.resize(100);
   QFile file(path);

   if (!file.open(QIODevice::ReadOnly |
                  QIODevice::Text))
      return -1;

   /** supress header lines */
   for (i = 0; i<4; i++)
      file.readLine();

   while (!file.atEnd()) {
      line = file.readLine();
      if (line.contains("end"))
         break;

      lst = QString(line).split(QRegExp("[=;\n]"));
      lst[0].remove("H(").remove(")");
      if (lst.length() == 4)
         lst.removeAt(lst.length() - 1);

      cCalib[lst[0].toInt()].Hd = lst[1].toDouble();
      cCalib[lst[0].toInt()].note = lst[2].remove(" ");
   }
   IOEdit *ioedit = IOEdit::getInstance();
   ioedit->putInfoLine("Hardware values read!");

   /** Remove the prior over-resized count of vector components */
   int k = cCalib.length() - 1;

   while (cCalib[--k].Hd == 0) {
      cCalib.removeLast();
   }

   for (int i = 0; i < cCalib.length(); i++)
      hwNotes.append( cCalib[i].note);

   return 0;
}

void Calc::loadHardwareDat() {
   if (loadHardwareDat(DEFAULT_DAT_FILE) < 0)
      qDebug() << "load hardware_dat failed!";
   else {
      visa = Visa::getObjectPtr();
      visa->gs.hw_xxx_dat_loaded = true;
   }
}

/**
 * Calculate physical values for 0x10..0x27
 */
int Calc::physCalcADC() {

   //   Visa *visa2  = Visa::getInstance();
   visareg->reqDvmADC();

   //   QVector<hw_calib_t>::iterator it;
   //   it = cCalib.begin();

   //   /** put some important columns of the original and the corrected hw values
   //    * to the console windows */
   //   qDebug() << tr("H_old   H_corr    Ctol");
   //   int k = 0; QString str;

   //   /** Table header row */
   //   ioedit->putInfoLine( tr("H(#): @H_alt @Hcorr @Note @Ctol"), 8, '@', '_');

   //   for(it = cCalib.begin(); it < cCalib.end(); it++, k++) {
   //      if ( cCalib[k].Hd ||  cCalib[k].H || (! cCalib[k].note.isEmpty()) ) {
   //         str.sprintf("H(%i):@%.4g@%.4g@%s@0x%02x",
   //                     k, cCalib[k].Hd, cCalib[k].H,
   //                     cCalib[k].note.toStdString().c_str(), cCalib[k].C);
   //         qDebug() << str;
   //         ioedit->putInfoLine( str, 10, '@', '_');
   //      }
   //   }
   //   qDebug() << tr("d");
   //   qDebug() << visareg->regAdcDvm->h181f_dvm.ADC_Vbias;
   //   qDebug() << visareg->regAdcDvm->h181f_dvm.ADC_avg;
   //   qDebug() << visareg->regAdcPwr->h1017_adcPwr.ADC_Iplus;
   //   qDebug() << visareg->regAdcPwr->h1017_adcPwr.ADC_Iminus;

   return 0;
}

