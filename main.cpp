#include <QApplication>
#include <qapplication.h>
#include <stdio.h>
#include <stdlib.h>
#include <QTimer>

#include "mainwindow.h"
#include "visa.h"
#include "globals.h"

void myMessageOutput(QtMsgType type,
                     const QMessageLogContext &context,
                     const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg: {
          QString lmsg = localMsg;
          lmsg.replace(" , ","\n");

//          fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
          fprintf(stderr, "Debug: %s \n", QByteArray(lmsg.toLocal8Bit()).constData());
       }
        break;
//    case QTypeInfo
//        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}

MainWindow *w; 
int x,y;

int main(int argc, char *argv[]) {
//   qInstallMessageHandler(myMessageOutput);
   QApplication a(argc, argv);

    w = new MainWindow();
    w->show();

    return a.exec();
}
