#ifndef MDSTATEBAR_H
#define MDSTATEBAR_H

#include <QWidget>
#include <QTextEdit>
#include <QStatusBar>
#include <QTimer>
#include <QLayout>

class MDStateBar : public QStatusBar {
   Q_OBJECT
public:
   explicit MDStateBar(QWidget *parent = 0);

   QTextEdit *getTeStat() const;
   void setTeStat(QTextEdit *value);

signals:

public slots:
   void showMessage(const QString & message, int timeout = 0);

private:
   QTextEdit   * teStat;
   QTimer      * timMsg;
};

#endif // MDSTATEBAR_H
