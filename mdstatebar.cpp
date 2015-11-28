#include "mdstatebar.h"

#include <QLayout>
#include <QDebug>
#include <QVector>
#include <QList>

MDStateBar::MDStateBar(QWidget *parent) :
   QStatusBar(parent) {

   teStat = new QTextEdit(parent);
   teStat->setFixedHeight(25);

   QLayout * la = layout();
   QList<QObject *> childs = la->children();

   qDebug().noquote() << "childs.count:" << childs.count();

   layout()->addWidget(teStat);
   teStat->setText(tr("teStat"));
}

void MDStateBar::showMessage(const QString &message, int timeout) {
   teStat->setText( message );

   if ( timeout )
      QTimer::singleShot( timeout, Qt::CoarseTimer,
                          this, SLOT(clearMessage()));
}
QTextEdit *MDStateBar::getTeStat() const
{
   return teStat;
}

void MDStateBar::setTeStat(QTextEdit *value)
{
   teStat = value;
}


