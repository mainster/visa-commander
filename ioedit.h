
#ifndef IOEDIT_H
#define IOEDIT_H

#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "console.h"
#include "portdialog.h"
#include "visa.h"
#include "globals.h"
#include "driver.h"

#include <qglobal.h>
#include <QMessageBox>
#include <QSettings>
#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QSerialPort>
#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QPlainTextEdit>


#include <QComboBox>
#include <QRadioButton>

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
class IOEdit : public QPlainTextEdit
{
   Q_OBJECT

public:
   /**
    * @brief
    *
    */
   /*!
    \brief

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

/**
 * @brief
 *
 */
/*!
 \brief

*/
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
   enum doBuffClear {
      ClearReferencedBuffer,
      KeepReferencedBuffer
   };
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
   Q_DECLARE_FLAGS(doBuffClears, doBuffClear)

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
   enum doWhitespChar {
      WhitespaceChars,
      NoWhitespaceChars
   };
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
   Q_DECLARE_FLAGS(doWhitespChars, doWhitespChar)


   /**
    * @brief
    *
    * @param maxChars
    * @param parent
    */
   /**
    * @brief
    *
    * @param maxChars
    * @param parent
    */
   /*!
    \brief

    \param maxChars
    \param parent
   */
   explicit IOEdit(quint64 maxChars = 0,
                   QWidget *parent = 0);
   /**
    * @brief
    *
    * @param maxChars
    * @param parent
    * @return IOEdit
    */
   /**
    * @brief
    *
    * @param maxChars
    * @param parent
    * @return IOEdit
    */
   /*!
    \brief

    \param maxChars
    \param parent
    \return IOEdit
   */
   static IOEdit* getInstance(quint64 maxChars = 0,
                              QWidget *parent = 0) {
      if (instance == NULL)
         instance = new IOEdit(maxChars, parent);
      return instance;
   }
   /**
    * @brief
    *
    * @return IOEdit
    */
   /*!
    \brief

    \return IOEdit
   */
   static IOEdit* getObjectPtr() {
      return instance;
   }
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
   ~IOEdit();
   int mouseWheelCnt;  

   /**
    * @brief
    *
    * @return QFont
    */
   /**
    * @brief
    *
    * @return QFont
    */
   /*!
    \brief

    \return QFont
   */
   QFont restoreFontConfig();
   /**
    * @brief
    *
    * @param font
    * @return bool
    */
   /**
    * @brief
    *
    * @param font
    * @return bool
    */
   /*!
    \brief

    \param font
    \return bool
   */
   bool storeFontConfig(QFont font);

   /** Internal char counter to initiate truncation if
    * max chars reached
    */
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
      qint64  numChars, maxChars;  
   } byteWatch;  

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
      bool enabled;  
      bool frmHeadItalic;  
      bool FullRuler;  
      bool NoRuler;  
   } parser;  

   /**
    * @brief
    *
    * @param start
    * @param end
    * @param delta
    * @param paddingStr
    * @param base
    * @param prefix
    * @param postfix
    * @param fieldWidth
    * @param filling
    * @return QString
    */
   /**
    * @brief
    *
    * @param start
    * @param end
    * @param delta
    * @param paddingStr
    * @param base
    * @param prefix
    * @param postfix
    * @param fieldWidth
    * @param filling
    * @return QString
    */
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
   QString seqGen(uint16_t start, uint16_t end,
                  uint16_t delta = 1, QString paddingStr = QLatin1String("  "),
                  quint8 base = 10, QString prefix = "", QString postfix = "",
                  quint8 fieldWidth = 0,
                  QLatin1Char filling = QLatin1Char('0') );
   /**
    * @brief
    *
    * @param vlines
    */
   /*!
    \brief

    \param vlines
   */
   void putInfoLine(const QVector<QString> vlines);
   /**
    * @brief
    *
    * @param str
    * @param n
    * @param paddingChar
    * @return QString
    */
   /*!
    \brief

    \param str
    \param n
    \param paddingChar
    \return QString
   */
   QString *strPadding(QString &str, int n, QChar paddingChar = ' ');
   /**
    * @brief
    *
    * @param pla
    * @return QString
    */
   /*!
    \brief

    \param pla
    \return QString
   */
   QString payloadMod(QByteArray pla);
signals:
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
   void syncedBuffWasted();
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
   void syncedReqBuffWasted();
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
   void maxCharCountReached();
   /**
    * @brief
    *
    * @param numDegrees
    */
   /**
    * @brief
    *
    * @param numDegrees
    */
   /*!
    \brief

    \param numDegrees
   */
   void wheelDelta(int numDegrees);

public slots:
   /**
    * @brief
    *
    * @param delta
    */
   /**
    * @brief
    *
    * @param delta
    */
   /*!
    \brief

    \param delta
   */
   void onwWheelDeltaReceived(QPoint delta);
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
   bool savePersistanceSettings();
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
   void loadPersistanceSettings();
   /**
    * @brief
    *
    * @param data
    */
   /**
    * @brief
    *
    * @param data
    */
   /*!
    \brief

    \param data
   */
   void putTxData(const QString &data);

   /**
    * @brief
    *
    * @param data
    * @param clear
    * @param space
    */
   /**
    * @brief
    *
    * @param data
    * @param clear
    * @param space
    */
   /*!
    \brief

    \param data
    \param clear
    \param space
   */
   void putRxData(QString &data,
                  enum doBuffClear clear = ClearReferencedBuffer,
                  enum doWhitespChar space = WhitespaceChars);
   /**
    * @brief
    *
    * @param data
    * @param clear
    * @param space
    */
   /*!
    \brief

    \param data
    \param clear
    \param space
   */
   void putRxDataSniff(QString &data,
                       enum doBuffClear clear = ClearReferencedBuffer,
                       enum doWhitespChar space = WhitespaceChars);

   /**
    * @brief
    *
    * @param pos
    */
   /**
    * @brief
    *
    * @param pos
    */
   /*!
    \brief

    \param pos
   */
   void ShowContextMenu(const QPoint &pos);
   //<<<<<<<<<<<<<<<<<<<<     blocks scrolling     >>>>>>>>>>>>>>>>>>>>//
   /**
    * @brief
    *
    * @param event
    */
   /**
    * @brief
    *
    * @param event
    */
   /*!
    \brief

    \param event
   */
   void wheelEvent(QWheelEvent *event);

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
   void setColorBack();
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
   void setColorText();
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
   void onMaxCountDoRemove();
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
   void setParserItalicHead(bool choose = 1);
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
   void setParserActive(bool choose = 0);
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
   void setParserFullRuler(bool choose);
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
   void setParserNoRuler(bool choose);
   /**
    * @brief This function is used to print out non-received or non-transmitted
    * info lines on the console window.
    *
    * @param lines String that contains the info line.
    * @param nChar If non-zero, the function uses \a colSep and \a paddingChar
    * to produce a "table-like" output line. Due to the fact, that neither
    * <a href="http://doc.qt.io/qt-4.8/qplaintextedit.html">QPlainTextEdit</a>
    * nor <a href="http://doc.qt.io/qt-4.8/qtextedit.html">QTextEdit</a> is able
    * to place the cursor to a specific column inside a text view, this function
    * can be used to produce rudimentary tables.
    * @param colSep
    * @param paddingChar
    */

   void putInfoLine(const QString cline,
                    const QString prefix = "INFO:",
                     const int nChar = 0,
                     const QChar colSep = ' ',
                     const QChar paddingChar = '_');

   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   void onSniffFilter();
   /**
    * @brief
    *
    * @param pla
    * @param lastPay
    * @return QString
    */
   /*!
    \brief

    \param pla
    \param lastPay
    \return QString
   */
   QString payloadDiff(QByteArray &pla, QString &lastPay, QColor color);
   QStringList getCurrentTextBlocks();
private:
   QFont fontTxLines, fontRxLines;  
   static IOEdit * instance;  
   Driver * driver;  
   qint64 rxCtr;  

   /** RINGBUFFER */
   QComboBox *cbFindSeq;  
   QRadioButton *rbDualFilter, *rbShowHeader, *rbShowStdBy;  
   /** RINGBUFFER END*/
};

#endif // IOEDIT_H
