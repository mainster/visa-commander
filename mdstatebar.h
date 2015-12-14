#ifndef MDSTATEBAR_H
#define MDSTATEBAR_H

#include <QWidget>
#include <QTextEdit>
#include <QStatusBar>
#include <QTimer>
#include <QLayout>
#include <QLabel>

class MDStateBar : public QStatusBar {
   Q_OBJECT

public:
   enum msgType {
      type_info = 0,
      type_msg = 1,
      type_err = 2,
   };
   Q_DECLARE_FLAGS(msgTypes, msgType)

   explicit MDStateBar(QWidget *parent = 0);
   static MDStateBar *getObjectPtr();
   ~MDStateBar();

   QTextEdit *getTeStat() const;
   void setTeStat(QTextEdit *value);

public slots:   
   void showError(const QString s, int timeout = 0);
   void showMessage(const QString s, int timeout = 0);
   void showInfo(const QString s, int timeout = 0);
   void appendError(const QString s, const QString sep = " ");
   void appendMessage(const QString s, const QString sep = " ");
   void appendInfo(const QString s, const QString sep = " ");
   void clearError();
   void clearMessage();
   void clearInfo();

private slots:
//   void onCriticalDriverError(QString err);

private:
   typedef struct {
      QList<QString> str;
   } prefix_t;

   prefix_t prefix;

   QList<QLabel *> mslot;

   static MDStateBar *inst;
   QTimer *timMsg;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MDStateBar::msgTypes)

#endif // MDSTATEBAR_H
