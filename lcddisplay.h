#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H

#include <QtCore/QtGlobal>
#include <QSerialPort>
#include <QMainWindow>
#include <QSettings>
#include <QObject>
#include <QLCDNumber>
#include <QDebug>
#include <QProcess>
#include <QWidget>
#include <QKeyEvent>
#include <QApplication>
#include <QMessageBox>
#include <QStyle>
#include <QLayout>
#include <QGridLayout>
#include <QLabel>

#include "globals.h"

/** Enums to set different LcdDisplay-selection frames */
namespace Lcd {
enum type_selection {
   type_select,
   type_altered,
   type_confirmed,
   type_none,
};
Q_DECLARE_FLAGS(type_selections, type_selection)
Q_DECLARE_OPERATORS_FOR_FLAGS(Lcd::type_selections)
}

class LcdDisplay : public QLCDNumber /*, private QLabel*/ {
   Q_OBJECT
public:

      Lcd::type_selections mSelected;

   explicit LcdDisplay(QString lableStr, QWidget *parent = 0);
   ~LcdDisplay() {}

   QLabel      *mlable;

signals:
   /**
    * Emit this signal in a focusIn() event could be used to inform
    * other LcdDisplay:: instances to lose theire select-indicator
    * frame.
    */
//   void lcdDisplaySelectionTrigger(LcdDisplay *obj);

public slots:
   void wheelEvent(QWheelEvent *ev);
   bool eventFilter(QObject *obj, QEvent *ev);
   void keyPressEvent(QKeyEvent *ev);
   void setSelected(Lcd::type_selections selectType);
   bool isSelected() const;

protected slots:
   void focusInEvent(QFocusEvent *ev);
   void focusOutEvent(QFocusEvent *ev);

private:
   QList<Qt::Alignment> al;
   static int ali;


};


#endif // LCDDISPLAY_H
