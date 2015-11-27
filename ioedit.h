#ifndef IOEDIT_H
#define IOEDIT_H

#include <QtCore/QtGlobal>
#include <QSerialPort>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QRadioButton>

#include "globals.h"

class Driver;

#define MIN_FRAMESIZE 50        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


class IOEdit : public QPlainTextEdit {
   Q_OBJECT

public:
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

      /*  char *cSCOPEON,
           *cFUGENON,
           *cFUGENOFF,
           *cVDCON,
           *cVDCOFF,
           *cVADCON,
           *cVADCOFF;*/
      //      cPOSPOWEREN[];
   };
   static const visaSeqences vseq;

   struct HTMLcode {
      QString
      STARTSEQ,
      RED,
      STDBY,
      DYNAMIC,
      HEAD_BGCOL,
      DEFAULT,
      WHITE,
      ENDL,
      RGB;
   };
   static const HTMLcode html;

   enum doBuffClear {
      ClearReferencedBuffer,
      KeepReferencedBuffer
   };
   Q_DECLARE_FLAGS(doBuffClears, doBuffClear)

   enum doWhitespChar {
      WhitespaceChars,
      NoWhitespaceChars
   };
   Q_DECLARE_FLAGS(doWhitespChars, doWhitespChar)

   explicit IOEdit(quint64 maxChars = 0,
                   QWidget *parent = 0);
   static IOEdit* getInstance(quint64 maxChars, QWidget *parent/* = 0*/) {
      if (inst == 0x00)
         inst = new IOEdit(maxChars, parent);
      return inst;
   }
   static IOEdit* getObjectPtr() {
      return inst;
   }
   ~IOEdit();

   int mouseWheelCnt;
   QFont restoreFontConfig();
   bool storeFontConfig(QFont font);
   /** Internal char counter to initiate truncation if
    * max chars reached
    */
   struct {
      qint64  numChars, maxChars;
   } byteWatch;
   struct {
      bool enabled;
      bool frmHeadItalic;
      bool FullRuler;
      bool NoRuler;
   } parser;
   QString seqGen(uint16_t start, uint16_t end,
                  uint16_t delta = 1, QString paddingStr = QLatin1String("  "),
                  quint8 base = 10, QString prefix = "", QString postfix = "",
                  quint8 fieldWidth = 0,
                  QLatin1Char filling = QLatin1Char('0') );
   void putInfoLine(const QVector<QString> vlines);
   QString *strPadding(QString &str, int n, QChar paddingChar = ' ');
   QString payloadMod(QByteArray pla);

signals:
   void syncedBuffWasted();
   void syncedReqBuffWasted();
   void maxCharCountReached();
   void wheelDelta(int numDegrees);

public slots:
   void onwWheelDeltaReceived(QPoint delta);
   bool savePersistanceSettings();
   void loadPersistanceSettings();
   void putTxData(const QString &data);
   void putRxData(QString &data,
                  enum doBuffClear clear = ClearReferencedBuffer,
                  enum doWhitespChar space = WhitespaceChars);
   void putRxDataSniff(QString &data,
                       enum doBuffClear clear = ClearReferencedBuffer,
                       enum doWhitespChar space = WhitespaceChars);
   void ShowContextMenu(const QPoint &pos);
   //<<<<<<<<<<<<<<<<<<<<     blocks scrolling     >>>>>>>>>>>>>>>>>>>>//
   void wheelEvent(QWheelEvent *event);
   void setColorBack();
   void setColorText();
   void onMaxCountDoRemove();
   void setParserItalicHead(bool choose = 1);
   void setParserActive(bool choose = 0);
   void setParserFullRuler(bool choose);
   void setParserNoRuler(bool choose);
   void putInfoLine(const QString cline,
                    const QString prefix = "INFO:",
                    const int nChar = 0,
                    const QChar colSep = ' ',
                    const QChar paddingChar = '_');
   void onSniffFilter();
   QString payloadDiff(QByteArray &pla, QString &lastPay, QColor color);
   QStringList getCurrentTextBlocks();

private:
   QFont fontTxLines, fontRxLines;
   static IOEdit * inst;
   Driver * driver;
   qint64 rxCtr;

   /** RINGBUFFER */
   QComboBox *cbFindSeq;
   QRadioButton *rbDualFilter, *rbShowHeader, *rbShowStdBy;
   /** RINGBUFFER END*/
};

#endif // IOEDIT_H
