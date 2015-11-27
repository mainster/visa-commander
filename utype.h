#ifndef UTYPE_H
#define UTYPE_H

#include <stdint.h>
#include <QVector>
#include <QList>

class Utype {

public:
   /** User typedef of a vector that holds calibration data */
   typedef QVector<double> calDat_t;
   typedef QVector<uint16_t> calDatRaw_t;

   /** User Typedef for value of measured data, including tolerance, error, unit */
   struct measure_t {
      double   value;  //< Value   
      double   tol;    //< Tolerance?!   
      QString * unit;   //< Physical unit   
      quint8   pow;    //< Decimal power   
   };
   typedef struct measure_t measure_t;

   /** User typedef of a container that holds several user types measure_t */
   typedef QVector<measure_t> metrics_t;
};

#endif // UTYPE_H
