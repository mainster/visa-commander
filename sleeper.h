
#ifndef SLEEPER
#define SLEEPER

#include <QThread>

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
class Sleeper : public QThread
{
public:
    /**
     * @brief
     *
     * @param usecs
     */
    /**
     * @brief
     *
     * @param usecs
     */
    /*!
     \brief

     \param usecs
    */
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    /**
     * @brief
     *
     * @param msecs
     */
    /**
     * @brief
     *
     * @param msecs
     */
    /*!
     \brief

     \param msecs
    */
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    /**
     * @brief
     *
     * @param secs
     */
    /**
     * @brief
     *
     * @param secs
     */
    /*!
     \brief

     \param secs
    */
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};

#endif // SLEEPER

