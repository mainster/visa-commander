#include <QApplication>
#include <QScrollBar>
#include <QColorDialog>
#include <QGridLayout>
#include <QMenu>

#include "ioedit.h"
#include "driver.h"
#include "visa.h"


/**
 * Forward declaration of class IOEdit is ok since this is the implementation
 * file of IOEdit
 */
class IOEdit;

#define SETFONT
#undef SETFONT


IOEdit* IOEdit::inst = NULL;

/** RINGBUFFER */
const IOEdit::HTMLcode IOEdit::html = { 
                                        "<font color=\"Red\"><b>",
                                        "<font color=\"Red\">",
                                        "<font color=\"Gray\">",
                                        "<font color=\"Yellow\">",
                                        "<b style=\"color: #000000; background-color: #c0c0c0\">@@@REPLACE</b>",
                                        //    "</font><br><",
                                        "</font>",
                                        "<font color=\"White\"><b>",
                                        "<br>",
                                        "<b style=\"color: #%RGB\">",

                                      };
const IOEdit::visaSeqences IOEdit::vseq =  { 
                                             "00000046706700",                                           //< STARTSEQ
                                             "040001620100057200000000000002be15000047e0",               //< STDBY
                                             "04000162",                                                 //< STDBYSML
                                             "040001620100057200000000000002be15020047e0",               //< STDBY_POWERON
                                             "03000162500002be15020047e0",                               //< STDBY_SCOPE Fenster offen aber kein trigger
                                             "0200026000d81000ec",                                       //< STDBY_SCOPE Fenster offen mit auto trigger
                                             "060009700800f205074cf20520000e500000000001c0001770000000000000086201470000ea000e3d000162410002be15020047e0",   //< SCOPEON
                                             "0500018a010001620100057200000000000002be15000047e0",       //< VDCON
                                             "0500018a000001620100057200000000000002be15000047e0",       //< VDCOFF
                                             "0500018b420001620100057200000000000002be15000047e0",       //< VADCON
                                             "0500018b000001620100057200000000000002be15000047e0",       //< VADCOFF
                                             "05000f3004830a657fff031100369d030000000001620100057200000000000002be15000047e0",   //< FUGENON
                                             "05000f30000007ff00000000000000000000000001620100057200000000000002be15000047e0",   //< FUGENOFF
                                             "05000880065800B400000C830001620100057200000000000002BE30020047E0",  //< POSPOWEREN
                                           };
/** RINGBUFFER END */

/*!
 \brief

 \param maxChars
 \param parent
*/
IOEdit::IOEdit(quint64 maxChars, QWidget *parent)
   : QPlainTextEdit(parent) {

   QPalette p = palette();
   p.setColor(QPalette::Base, Qt::black);
   p.setColor(QPalette::Text, Qt::green);
   setPalette(p);

#define QFOLDINGSTART {
   /** RINGBUFFER */

   QWidget *ringWin = new QWidget( parentWidget() );
   cbFindSeq = new QComboBox(this);
   rbShowHeader = new QRadioButton(this);
   rbShowStdBy = new QRadioButton(this);
   rbDualFilter = new QRadioButton(this);

   cbFindSeq->setEditable(true);
   cbFindSeq->setMinimumWidth(200);
   //   cbFindSeq->setMaximumWidth(600);
   cbFindSeq->setMaximumHeight(25);

   cbFindSeq->addItem(vseq.FUGENOFF);
   cbFindSeq->addItem(vseq.FUGENON);
   cbFindSeq->addItem(vseq.SCOPEON);
   cbFindSeq->addItem(vseq.STARTSEQ);
   cbFindSeq->addItem(vseq.STDBY);
   cbFindSeq->addItem(vseq.STDBY_POWERON);
   cbFindSeq->addItem(vseq.STDBY_SCOPE);
   cbFindSeq->addItem(vseq.STDBY_SCOPE_AUTOTRIG);
   cbFindSeq->addItem(vseq.STDBYSML);
   cbFindSeq->addItem(vseq.VADCOFF);
   cbFindSeq->addItem(vseq.VADCON);
   cbFindSeq->addItem(vseq.VDCOFF);
   cbFindSeq->addItem(vseq.VDCON);


   QVector<QRadioButton *> rbs;
   rbs.append(rbShowHeader);
   rbs.append(rbShowStdBy);
   rbs.append(rbDualFilter);

   for (int i=0; i<3;i++) {
      rbs[i]->setAutoExclusive(false);
      rbs[i]->setCheckable(true);
      rbs[i]->setChecked(false);
   }

   rbs[0]->setText("rbShowHeader");
   rbs[1]->setText("rbShowStdBy");
   rbs[2]->setText("rbDualFilter");
   rbs[2]->setChecked(true);

   QGridLayout *gl = new QGridLayout(this);
   gl->addWidget(cbFindSeq,0,0,Qt::AlignLeft);
   gl->addWidget(rbShowHeader,1,0,Qt::AlignLeft);
   gl->addWidget(rbShowStdBy,2,0,Qt::AlignLeft);
   gl->addWidget(rbDualFilter,3,0,Qt::AlignLeft);

   ringWin->setLayout(gl);
   ringWin->show();


   /** RINGBUFFER END */
#define QFOLDINGEND }

   /** Enable whitespace visibility for the plain text edit */
   //   QTextOption option = document()->defaultTextOption();
   //   if (false)
   //      option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
   //   else
   //      option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces);
   //   option.setFlags(option.flags() | QTextOption::
   //                   AddSpaceForLineAndParagraphSeparators);
   //   document()->setDefaultTextOption(option);
   /**/

   QFont font = restoreFontConfig();
   font.setStyleName("Monospace");
   font.setStyleHint (QFont::Monospace);
   font.setFixedPitch (true);
   setFont(font);

   loadPersistanceSettings();
   mouseWheelCnt = 0;

   if (! maxChars) {
      QSETTINGS;
      maxChars = config.value("ReadOnly/MAX_CONSOLE_CHARS", 5000).toInt();
      byteWatch.maxChars = maxChars;
   }
   byteWatch.numChars = 0;

   // myWidget is any QWidget-derived class
   setContextMenuPolicy(Qt::CustomContextMenu);

   connect(this,     SIGNAL(customContextMenuRequested(const QPoint&)),
           this,     SLOT(ShowContextMenu(const QPoint&)));
   connect(this,     SIGNAL(maxCharCountReached()),
           this,     SLOT(onMaxCountDoRemove()));

   parser.frmHeadItalic = true;
   parser.enabled       = false;

}
/*!
 \brief

*/
IOEdit::~IOEdit() {

}

/*!
 \brief

 \param start
 \param end
 \param delta
 \param paddingStr
 \param base
 \param prefix
 \param postfix
 \param fieldWidth
 \param filling
 \return QString
*/
QString IOEdit::seqGen(uint16_t start, uint16_t end,
                       uint16_t delta, QString paddingStr,
                       quint8 base, QString prefix, QString postfix,
                       quint8 fieldWidth, QLatin1Char filling) {

   QString seq;
   //   seq.clear();

   uint16_t i;
   for (i = start; i < end; i+=1) {
      if ( (delta == 1) || (! (i % delta)) ) {
         seq.append(prefix)
               .append(QString("%1").arg(i, fieldWidth, base, filling))
               .append(postfix);
      }
      else {
         ((seq.append(prefix)).append(paddingStr)).append(postfix);
      }
   }

   return seq;
}

/*!
 \brief

 \param delta
*/
void IOEdit::onwWheelDeltaReceived(QPoint delta) {

   scroll(0, delta.y());
}

/*!
 \brief

 \param data
*/
void IOEdit::putTxData(const QString &data) {
   QString tmp;

   moveCursor(QTextCursor::End);

   if (parser.enabled) {
      tmp = HTML_FONT_RED;
      tmp.replace("%1", data);
      tmp.replace(FRAMEHEAD.remove(" "),
                  HTML_FONT_xCOLOR_IT
                  .replace("%COLOR", "800000")
                  .replace("%1", FRAMEHEAD.remove(" ")));
      appendHtml(tmp);
   }
   else {
      appendPlainText( data );
   }

   if( (byteWatch.numChars += tmp.length()) > byteWatch.maxChars )
      emit maxCharCountReached();

   /** This signal should make a call to driver
    * interface for flushing its synced rxBuff.
    */
   emit syncedBuffWasted();
}
/*!
 \brief

 \param data
 \param clear
 \param space
*/
void IOEdit::putRxDataSniff(QString &data,
                            enum doBuffClear clear,
                            enum doWhitespChar space) {
   int i, k;
   static QString lastLine;
   QStringList linesSep;

   moveCursor(QTextCursor::End);

   if ((parser.enabled) && (parser.frmHeadItalic)) {
      QStringList lines = data.split("\n");
      lines.removeDuplicates();

      if (space == IOEdit::WhitespaceChars) {
         /**
          * Insert spaces for byte separation.
          * Generate a "ruler" sequence
          */
         for (i = 0; i < lines.count(); i++) {
            linesSep.push_back("");
            lines[i].replace(FRAMEHEAD.remove(" "),
                             QString(HTML_FRAMEHEAD_DARKGN_IT));
            int hdLen = tr(HTML_FRAMEHEAD_DARKGN_IT).length();

            if ( (lines[i].length() - hdLen) > 1) {
               int nchar2x = lines[i].length(); // - headLength;

               linesSep[i].append(lines[i].left( hdLen ));
               for (k = hdLen; k < nchar2x; k += 2) {
                  linesSep[i].append("   ");
                  linesSep[i].append(lines[i].mid(k,2));
               }

               if (! linesSep.isEmpty()) {
                  /** equal to the last printed */
                  if (/*linesSep[i] != lastLine*/true) {
                     appendHtml(linesSep[i]);
                     lastLine = linesSep[i];

                     if (! parser.NoRuler) {
                        if (parser.FullRuler) {
                           appendHtml( HTML_FONT_xCOLOR_RULER
                                       .replace("%COLOR", "383838")
                                       .replace("%1",
                                                seqGen(0, (nchar2x - hdLen)/2, 1, "",
                                                       16, "", " ", 2, QLC)) );
                        }
                        else {
                           appendHtml( HTML_FONT_xCOLOR_RULER
                                       .replace("%COLOR", "383838")
                                       .replace("%1",
                                                seqGen(0, (nchar2x - hdLen)/2, 5,
                                                       "__", 16, "", " ", 2, QLC)) );
                        }
                     }
                  }
               }
            }
         }
      }
   }
   else {
      insertPlainText(data);
   }
   /** Check if max. chars for ioedit is reached and emit signal */
   if( (byteWatch.numChars += data.length()) > byteWatch.maxChars )
      emit maxCharCountReached();

   //   Driver * l_driver = Driver::getObjectPtr();
   /**
    * Check if referenced buffer should be cleared.
    * This signal should make a call to driver interface for flushing its
    * synced rxBuff.
    */
   if (clear == IOEdit::ClearReferencedBuffer) {
      //      if ( data == l_driver->rxBuff.dataSync )
      emit syncedBuffWasted();
      //      if ( data == l_driver->rxBuffReq.data )
      emit syncedReqBuffWasted();
   }
}
/*!
 \brief

 \param data
 \param clear
 \param space
*/
void IOEdit::putRxData(QString &data,
                       enum doBuffClear clear,
                       enum doWhitespChar space) {
   moveCursor(QTextCursor::End);
   int i, k;
   QStringList linesSep;

   if ((parser.enabled) && (parser.frmHeadItalic)) {
      QStringList lines = data.split("\n");

      if (space == IOEdit::WhitespaceChars) {
         /**
          * Insert spaces for byte separation.
          * Generate a "ruler" sequence
          */
         for (i = 0; i < lines.count(); i++) {
            linesSep.push_back("");
            lines[i].replace(FRAMEHEAD.remove(" "),
                             QString(HTML_FRAMEHEAD_DARKGN_IT));
            if (lines[i].length() > 1) {
               int nchar2x = lines[i].length();

               for (k = 0; k < nchar2x; k+=2) {
                  //                  if ((k != 0x10) && (k != 0x12) &&
                  //                      (k != 0x14) && (k != 0x16)) {
                  //                  }
                  linesSep[i].append(" ");
                  linesSep[i].append(lines[i].mid(k,2));
               }

               appendHtml(linesSep[i]);
               if (! parser.NoRuler) {
                  if (parser.FullRuler) {
                     appendHtml( HTML_FONT_xCOLOR_RULER
                                 .replace("%COLOR", "383838")
                                 .replace("%1", seqGen(0, (nchar2x)/2, 1, "",
                                                       16, "", " ", 2, QLC)) );
                  }
                  else {
                     appendHtml( HTML_FONT_xCOLOR_RULER
                                 .replace("%COLOR", "383838")
                                 .replace("%1", seqGen(0, (nchar2x)/2, 5, "__",
                                                       16, "", " ", 2, QLC)) );
                  }
               }
            }
         }
      }
   }
   else {
      insertPlainText(data);
   }
   /** Check if max. chars for ioedit is reached and emit signal */
   if( (byteWatch.numChars += data.length()) > byteWatch.maxChars )
      emit maxCharCountReached();

   /**
    * Check if referenced buffer should be cleared.
    * This signal should make a call to driver interface for flushing its
    * synced rxBuff.
    */
   if (clear == IOEdit::ClearReferencedBuffer)
      emit syncedReqBuffWasted();
}
/*!
 \brief

 \param cline
 \param nChar
 \param colSep
 \param paddingChar
*/
void IOEdit::putInfoLine(const QString cline,
                         const QString prefix,
                         const int nChar,
                         const QChar colSep,
                         const QChar paddingChar) {
   QString tmp; int i;  QStringList lline;   QString line = cline;
   QString HTML_INFO_LINE_MOD = HTML_INFO_LINE;

   if (! prefix.isEmpty())
      HTML_INFO_LINE_MOD.replace("%PREFIX", prefix/* + "INFO"*/);

   if (line.length() > (line.remove(" ")).length())
      Q_INFO << tr("Whitespace truncated!");

   if ( nChar ) {
      lline = line.split( colSep );

      for (i=0; i < lline.length(); i++) {
         strPadding( lline[i], 10, paddingChar);
      }
      tmp = HTML_INFO_LINE_MOD.replace("%1", lline.join('_'));
   }
   else {
      tmp = HTML_INFO_LINE_MOD.replace("%1", line);
   }
   appendHtml( tmp );
}
/*!
 \brief

 \param str
 \param n
 \param paddingChar
 \return QString
*/
QString *IOEdit::strPadding(QString &str, int n, QChar paddingChar) {
   int currPos = str.length();

   if (currPos < n) {
      for (int k=0; k < (n - currPos); k++)
         str.append(paddingChar);
   }
   return &str;
}
/**
 * This slot truncates the subclassed QPlainEdit widget if max lines are
 * received
 */
/*!
 \brief

*/
void IOEdit::onMaxCountDoRemove() {
   byteWatch.numChars = this->toPlainText().length();

   if (byteWatch.numChars > byteWatch.maxChars) {
      QTextCursor c = this->textCursor();
      c.setPosition(0);
      c.setPosition((int)(.20 * byteWatch.maxChars),
                    QTextCursor::KeepAnchor);
      c.removeSelectedText();
      byteWatch.numChars -= (int)(.20 * byteWatch.maxChars);
   }

   /*   qDebug() << "Clear: " << QString::number(
                  this->toPlainText().length());*/
}
/*!
 \brief

 \param choose
*/
void IOEdit::setParserItalicHead(bool choose) {
   parser.frmHeadItalic = choose;
}
/*!
 \brief

 \param choose
*/
void IOEdit::setParserActive(bool choose) {
   parser.enabled = choose;
}
/*!
 \brief

 \param choose
*/
void IOEdit::setParserFullRuler(bool choose) {
   parser.FullRuler = choose;
}
/*!
 \brief

 \param choose
*/
void IOEdit::setParserNoRuler(bool choose) {
   parser.NoRuler = choose;
}
/**
 * Store font parameters to config file
 */
/*!
 \brief

 \param font
 \return bool
*/
bool IOEdit::storeFontConfig(QFont font) {
   QSETTINGS;
   config.setValue("ioedit/lastFont", font.toString());
   return true;
}
/**
 * Restore font from saved string
 */
/*!
 \brief

 \return QFont
*/
QFont IOEdit::restoreFontConfig() {
   QFont ret;
   QSETTINGS;
   ret.fromString(config.value("ioedit/lastFont","").toString());
   return ret;
}
/*!
 \brief

*/
void IOEdit::loadPersistanceSettings() {
   QSETTINGS;
   ///< Load configuration file, pointed to by configFile
   // QSettings config(QString::fromUtf8( &configFile )), QSettings::IniFormat);
   //  QSettings config(configFile, QSettings::IniFormat);
   /** @todo Select port by portname and not by portBox index.*/
   config.beginGroup("ioedit");  {
      QFont ret = restoreFontConfig();
      this->setFont(ret);

      QPalette p = palette();
      QColor col;
      col.setNamedColor( config.value("colorBase").toString() );
      p.setColor(QPalette::Base, col );
      col.setNamedColor( config.value("colorText").toString() );
      p.setColor(QPalette::Text, col );
      setPalette( p );

      config.endGroup();
   }
}
/*!
 \brief

 \return bool
*/
bool IOEdit::savePersistanceSettings() {
   /**
     * Save all user settings in a configuration file on the filesytem
     */
   QTime time = QTime::currentTime();
   QString timestr =
         QString::number(time.hour()) +
         QString::number(time.minute()) +
         QString::number(time.second());
   //@@@1MDB//qDebug()() << "Writing config at" << time.toString() << "h";
   QFile::copy(CONFIG_PATH, QString(CONFIG_PATH) + "_" + timestr);
   QSETTINGS;

   config.beginGroup("ioedit");  {
      storeFontConfig(this->font());
      QPalette p = palette();
      config.setValue("colorBase",p.color(QPalette::Base));
      config.setValue("colorText",p.color(QPalette::Text));
      config.endGroup();
   }
   return true;
}
/*!
 \brief

*/
void IOEdit::setColorBack() {
   QColor color;
   if (true)
      color = QColorDialog::getColor(Qt::green, this);
   else
      color = QColorDialog::getColor(Qt::green, this,
                                     "Select Color",
                                     QColorDialog::DontUseNativeDialog);

   if (color.isValid()) {
      this->setPalette(QPalette(color));
      this->setAutoFillBackground(true);
   }
}
/*!
 \brief

*/
void IOEdit::setColorText() {
   QColor color;
   if (true)
      color = QColorDialog::getColor(Qt::green, this);
   else
      color = QColorDialog::getColor(Qt::green, this,
                                     "Select Color",
                                     QColorDialog::DontUseNativeDialog);

   if (color.isValid()) {
      this->setPalette(QPalette(color));
      this->setAutoFillBackground(true);
   }
}
/*!
 \brief

 \param pos
*/
void IOEdit::ShowContextMenu(const QPoint& pos) {
   /** for most widgets */
   QPoint globalPos = mapToGlobal(pos);

   /** for QAbstractScrollArea and derived classes you would use:
    * QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);
    */

   QMenu myMenu;
   myMenu.addAction("Menu Item 1");
   // ...

   QAction* selectedItem = myMenu.exec(globalPos);
   if (selectedItem) {
      // something was chosen, do stuff
   }
   else {
      // nothing was chosen
   }
}
/*!
 \brief

 \param event
*/
void IOEdit::wheelEvent ( QWheelEvent * event ) {
   /** Add current step.
    * event->delta() can be negative or positive.
    */
   mouseWheelCnt += event->delta()/120;

   /** query current keyboard modifiers
    */
   Qt::KeyboardModifiers mods = QApplication::queryKeyboardModifiers();

   int dsize = event->delta()/120;     /**< Delta size */

   /** Ctrl modifier + mouse wheel changes font size in QPlainText
    * derived class IOEdit.
    * Do some range check of min/max pointsize
    */
   if (mods == Qt::ControlModifier) {
      if ( ((dsize < 0) && (fontInfo().pointSize() > 6)) ||
           ((dsize > 0) && (fontInfo().pointSize() < 20)) ) {
         QFont cfont = QFont( this->font() );
         QFont mfont = QFont("Monospace");
         mfont.setPointSize( cfont.pointSize() + dsize );
         mfont.setStyleHint(QFont::Monospace);
         setFont( mfont );
         storeFontConfig( this->font() );
      }
   }
   else {
      if(mods == Qt::ShiftModifier) {
         verticalScrollBar()->setValue(
                  verticalScrollBar()->value() - 4 * dsize );
      }
      else {
         verticalScrollBar()->setValue(
                  verticalScrollBar()->value() - dsize );
      }
   }
   event->accept();
}

QStringList IOEdit::getCurrentTextBlocks() {
   QStringList blks;
   blks = toPlainText().split("\n");
   return blks;
}

/** RINGBUFFER */


/*!
 \brief

*/
void IOEdit::onSniffFilter() {
   static QString lastPayLoad;
   /**
     * If a transceiver operation is active, don't read any byte outside
     * of the waiting function.
     */
   Visa *visa = Visa::getObjectPtr();
   Driver *driver = Driver::getObjectPtr();

   if (visa->gs.blockOnReadyRead)
      return;

   /**
     * Distinguish if opened port is /dev/visascope (ttyUSB0) or not
     */
   if ( !( driver->serial->objectName().contains("visascope???") ||
           driver->serial->objectName().contains("ttyUSB0???") )) {
      /**
         * Port opened is NOT /dev/visascope
         */
      QByteArray pattA, pattStdBy; //, pattB;
      QByteArray frameHead, framePayload;
      int pattPos, pattPosB;
      QTextCursor cursor = /*ui->recvEdit->*/textCursor();
      QString line;

      //------------------------------------------
      rxCtr += driver->serial->bytesAvailable();      ///< new bytes count
      driver->rxBuffAlt.dataUnSync.append(driver->serial->readAll().toHex());
      //------------------------------------------

      /**
         * Check if there are enough chars in rxBuffer so we can try to sync for the next frame
         */
      if (driver->rxBuffAlt.dataUnSync.length() >= MIN_FRAMESIZE) {
         /**
             * get hex representation of the frame header search pattern
             * START seq
             */
         pattA = vseq.STARTSEQ.toLatin1();

         /**
             * Check if rxBuff contains pattern sequence 2 times so we can identify the
             * frame length. If only one or none pattern seq found, return and keep buffer
             */
         if ((pattPos  = driver->rxBuffAlt.dataUnSync.indexOf(pattA)) < 0 ||
             (pattPosB = driver->rxBuffAlt.dataUnSync.indexOf(pattA, pattPos + 1)) < 0)
            return;

         /**
             * Sync rxBuffer contents down to first position of pattern seq
             * Also subtract the same number from the next pattern seq position
             */
         driver->rxBuffAlt.dataUnSync.remove(0, pattPos);
         pattPosB -= pattPos;

         /**
             * Copy chars from pos 0 to pattPosB
             * The frame consists of the pattern seq as frame header and a payload of variant length
             * Remove the extracted frame from receive buffer
             */
         QByteArray tmp = driver->rxBuffAlt.dataUnSync.left(pattPosB);
         driver->rxBuffAlt.dataUnSync.remove(0, pattPosB);
         frameHead = pattA;
         tmp.remove(0, pattA.length());
         framePayload = tmp;

         /**
             * Write the frame payload to rxBuffSynced array without STARTSEQ
             */
         driver->rxBuffAlt.dataSync.append(framePayload);

         /**
             * Try to remove the STDBY seq from synced payload array.
             * Wenn des hin haut, bleiben nur noch die interessanten frames uebrig
             */
         pattStdBy = cbFindSeq->currentText().toLatin1();

         while ((pattPos = driver->rxBuffAlt.dataSync.indexOf(pattStdBy)) >= 0) {
            driver->rxBuffAlt.dataSync.remove(0, pattStdBy.length()); ///< Remove STDBY seq

            if (! pattStdBy.length())
               return;
         }

         if (rbDualFilter->isChecked()) {
            pattStdBy = vseq.STDBY.toLatin1();

            while ((pattPos = driver->rxBuffAlt.dataSync.indexOf(pattStdBy)) >= 0) {
               driver->rxBuffAlt.dataSync.remove(0, pattStdBy.length());       ///< Remove STDBY seq
            }
         }

         /**
             * If there are still some bytes in rxBuffSynced, this bytes would be some of
             * the interesting one...
             * STARTSEQ sowie STDBY wurden aus dem rxBuffSynced raus geschnitten, was
             * bleibt sind die Befehle die gesendet werden, wenn man in der vs-gui module
             * benutzt...
             */
         if (driver->rxBuffAlt.dataSync.length() > 0) {
            /**
                 * Print out the synced frame
                 */

            if (rbShowHeader->isChecked())
               line = html.STARTSEQ + vseq.STARTSEQ + html.DEFAULT;

            if (rbShowStdBy->isChecked()) {
               line = line + html.STDBY + vseq.STDBY + html.DEFAULT;
               QString framePayload2 =
                     payloadDiff(framePayload, lastPayLoad, QColor(Qt::blue));
               line = line + html.DYNAMIC + framePayload2 + html.DEFAULT /*+ html.ENDL*/;
               appendHtml(line);
            }
            lastPayLoad = framePayload;

            cursor.movePosition(QTextCursor::End);
            setTextCursor(cursor);
            driver->rxBuffAlt.flushAll();
         }
      }
   }
   else {
      /**
     * Port opened IS /dev/visascope
     */
      //ui->recvEdit->setTextCursor(QTextCursor::End);
      if (driver->serial->bytesAvailable()) {
         moveCursor (QTextCursor::End);
         insertPlainText(driver->serial->readAll().toHex());
      }
   }
}

/*!
 \brief

 \param pla
 \param lastPay
 \return QString
*/
QString IOEdit::payloadDiff(QByteArray& pla,
                            QString& lastPay,
                            QColor color) {
   //   static QByteArray lstp = lastPay.toLatin1();
   QString pl = QString(pla.toStdString().c_str());
   QString out;

   if (pl.isEmpty())
      return tr("*empty*");

   if (pl.contains(lastPay))
      return QString(lastPay);

   QString htmlCol = html.RGB;
   htmlCol.replace("%RGB", QString("%1%2%3")
                   .arg(color.red(), 2, 16, QLC)
                   .arg(color.green(), 2, 16, QLC)
                   .arg(color.blue(), 2, 16, QLC));

   for (int i=0; i<pl.length(); i+=2) {
      if (pl.mid(i, 2).contains( lastPay.mid(i, 2) ))
         out.append(pl.mid(i,2));
      else
         out.append(htmlCol + pl.mid(i, 2) + html.DEFAULT);
   }
   return out;
}

/*!
 \brief

 \param pla
 \return QString
*/
QString IOEdit::payloadMod(QByteArray pla) {
   bool *ok;
   ok = false;

   QString pl = QString(pla.toStdString().c_str());

   if (pl.isEmpty())
      return tr("*empty*");

   /** Get # of blocks as integer */
   //   QString sub = pl.left(2);
   int nbl = (pl.left(2)).toInt(ok, 16);
   (!ok) ? qDebug() << tr("conversion fail") : qDebug();
   pl.remove(0, 2);

   /** Get # of bytes of next block*/
   int *nby; QVector<QString *> reg;

   QString s;
   s.append(QString("%1 ").arg(nbl, 4, 16, QLC));
   pl.remove(0,4);

   while (! pl.isEmpty()) {
      *nby++ = (pl.left(4)).toInt(ok, 16);
      (!ok) ? qDebug() << tr("conversion fail") : qDebug();
      pl.remove(0, 4);
      //      s.append(pl.left(0, 2* (*(nby - 1))) + " ");
      pl.remove(0, 2* (*(nby - 1)));
   }

   return s;
}

/** RINGBUFFER END */
