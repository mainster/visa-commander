#ifndef REGISTER_H
#define REGISTER_H

#include <QtGui>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QSpacerItem>

#define  MAX_ROWS    40

namespace Ui {
class Register;
}

class Register : public QDialog {
   Q_OBJECT

public:
   explicit Register(QWidget *parent = 0);
   ~Register();

   void onBtnOneClicked();
   void setRegister(uint8_t addr, QString text);
   void setLeWidth(uint16_t wmax, uint16_t wmin);
   void setLeWidth(uint16_t wmax);
   void doFadeOut(uint8_t steps);
   void setBgColor(QByteArray &y);
   void setBgColorOfInterestings();

protected:
   bool eventFilter(QObject *obj, QEvent *ev);

private:
   Ui::Register *ui;
   QLabel *labelArrC1[MAX_ROWS + 3];
   QLineEdit *leArrC2[MAX_ROWS + 3];
   QSpacerItem *spacer;
   QVector< uint8_t > vInterest;
};

#endif // REGISTER_H
