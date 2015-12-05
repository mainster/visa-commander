#ifndef EVENTS_H
#define EVENTS_H

#include <QObject>

class Events : public QObject
{
   Q_OBJECT
public:
   explicit Events(QObject *parent = 0);

signals:

public slots:

};

#endif // EVENTS_H
