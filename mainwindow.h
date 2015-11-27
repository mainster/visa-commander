#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QSerialPort>

#include "visa.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow {

   Q_OBJECT

public:
   explicit MainWindow(QWidget *parent = 0);
   ~MainWindow();

public slots:
   void CRASHME();
   char *convert(QString in);
   void about();
   void onActionVisa();
   void onBtnClearRxClicked();
   void onBtnClearTxClicked();
   void quit();
   void mousePressEvent(QMouseEvent *event);

private:
   void initActionsConnections();

private:
   Ui::MainWindow *ui;  
   Visa *visa;  
   bool visaNotNull;  
};

#endif // MAINWINDOW_H

