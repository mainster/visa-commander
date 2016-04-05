#include "mdstatebar.h"
#include "driver.h"

#include <QLayout>
#include <QDebug>
#include <QVector>
#include <QList>
#include <QLabel>

//         slInfo                   slMsg                      slErr
/* --------------------------------------------------------------------------
|     Long time msg     |     msg changes all 5sec       |    Error msg      |
 --------------------------------------------------------------------------- */
#define N_MSG_SLOTS  3

MDStateBar * MDStateBar::inst = 0x0;

MDStateBar::MDStateBar(QWidget *parent) :
   QStatusBar(parent) {
   inst = this;

   this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);


   for (int i = 0; i < N_MSG_SLOTS; i++)
      mslot.append( new QLabel(parent));

   int k = 0;
   QList<QPalette> pals;

   foreach (QLabel *l, mslot) {
      l->setFocusPolicy(Qt::NoFocus);
      l->setSizePolicy(this->sizePolicy());
      pals.append( l->palette() );
      this->addWidget(l);
   }

   QColor col;
   col.setRgb(25,5,5,50);
   pals[type_info].setColor(QPalette::Background, col);

   col.setRgb(25,5,5,100);
   pals[type_msg].setColor(QPalette::Background, col);

   col.setRgb(25,5,5,150);
   pals[type_err].setColor(QPalette::Background, col);

   k = 0;
   foreach (QLabel *l, mslot)
      l->setPalette( pals[k++] );

   prefix.str.append( QString("Info: ") );
   prefix.str.append( QString("Message: ") );
   prefix.str.append( QString("Error: ") );

   //   this->setLayout( hlay );
   //   lay.addItem(mslot[0]);

   //   this->setLayout(QLayout().addItem( hlay ));

}
MDStateBar *MDStateBar::getObjectPtr() {
   return inst;
}
MDStateBar::~MDStateBar() {
}

void MDStateBar::showError(const QString s, int timeout) {
   mslot[type_err]->setText( prefix.str[type_err] + s );
   Q_INFO << s;
   if (! timeout)
      return;

   QTimer::singleShot(timeout, Qt::CoarseTimer,this, SLOT(clearError()));
}
void MDStateBar::showMessage(const QString s, int timeout) {
   mslot[type_msg]->setText( prefix.str[type_msg] + s );
   Q_INFO << s;
   if (! timeout)
      return;

   QTimer::singleShot(timeout, Qt::CoarseTimer,this, SLOT(clearMessage()));
}
void MDStateBar::showInfo(const QString s, int timeout) {
   mslot[type_info]->setText( prefix.str[type_info] + s );
   Q_INFO << s;
   if (! timeout)
      return;

   QTimer::singleShot(timeout, Qt::CoarseTimer,this, SLOT(clearInfo()));
}
void MDStateBar::appendError(const QString s, const QString sep) {
   mslot[type_err]->
         setText(tr("%1%2").arg(mslot[type_err]->text()).arg(sep + s));
   Q_INFO << s;
}
void MDStateBar::appendMessage(const QString s, const QString sep) {
   mslot[type_msg]->
         setText(tr("%1%2").arg(mslot[type_msg]->text()).arg(sep + s));
   Q_INFO << s;
}
void MDStateBar::appendInfo(const QString s, const QString sep) {
   mslot[type_info]->
         setText(tr("%1%2").arg(mslot[type_info]->text()).arg(sep + s));
   Q_INFO << s;
}
void MDStateBar::clearError() {
   mslot[type_err]->clear();
   mslot[type_err]->setText(prefix.str[type_err]);
}
void MDStateBar::clearMessage() {
   mslot[type_msg]->clear();
   mslot[type_msg]->setText(prefix.str[type_msg]);
}
void MDStateBar::clearInfo() {
   mslot[type_info]->clear();
   mslot[type_info]->setText(prefix.str[type_info]);
}

QTextEdit *MDStateBar::getTeStat() const {
   return new QTextEdit();
}
void MDStateBar::setTeStat(QTextEdit *value) {
   Q_UNUSED(value);
}


