
#ifndef UTYPE_H
#define UTYPE_H

#include <stdint.h>
#include <QVector>
#include <QList>


/**
 * @brief
 *
 */
/**
 * @brief
 *
 */
/*!
 \brief

*/
class Utype {
public:
   /** User typedef of a vector that holds calibration data */
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   typedef QVector<double> calDat_t;
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   typedef QVector<uint16_t> calDatRaw_t;

   /** User Typedef for value of measured data, including tolerance, error, unit */
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   typedef struct {
      double   value;  //< Value   
      double   tol;    //< Tolerance?!   
      QString * unit;   //< Physical unit   
      quint8   pow;    //< Decimal power   
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   } measure_t;

   /** User typedef of a container that holds several user types measure_t */
   /**
    * @brief
    *
    */
   /**
    * @brief
    *
    */
   /*!
    \brief

   */
   typedef QVector<measure_t> metrics_t;
};

#endif // UTYPE_H
