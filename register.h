
#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>
#include <QFrame>
#include <QDockWidget>
#include <QLabel>
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <stdint.h>
#include <QTimer>
#include <QTextEdit>
#include <QGridLayout>
#include <QLineEdit>
#include <vector>
#include <QEvent>
#include <QSettings>

namespace Ui {
class Register;
}

static const int MAX_ROWS = 40;   

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
class Register : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief
     *
     * @param parent
     */
    /**
     * @brief
     *
     * @param parent
     */
    /*!
     \brief

     \param parent
    */
    explicit Register(QWidget *parent = 0);
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
    ~Register();
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
    void onBtnOneClicked();
    /**
     * @brief
     *
     * @param addr
     * @param text
     */
    /**
     * @brief
     *
     * @param addr
     * @param text
     */
    /*!
     \brief

     \param addr
     \param text
    */
    void setRegister(uint8_t addr, QString text);
    /**
     * @brief
     *
     * @param wmax
     * @param wmin
     */
    /**
     * @brief
     *
     * @param wmax
     * @param wmin
     */
    /*!
     \brief

     \param wmax
     \param wmin
    */
    void setLeWidth(uint16_t wmax, uint16_t wmin);
    /**
     * @brief
     *
     * @param wmax
     */
    /**
     * @brief
     *
     * @param wmax
     */
    /*!
     \brief

     \param wmax
    */
    void setLeWidth(uint16_t wmax);
    /**
     * @brief
     *
     * @param steps
     */
    /**
     * @brief
     *
     * @param steps
     */
    /*!
     \brief

     \param steps
    */
    void doFadeOut(uint8_t steps);
    //    void setBgColor(QByteArray differs[], int nbyte);
    /**
     * @brief
     *
     * @param y
     */
    /**
     * @brief
     *
     * @param y
     */
    /*!
     \brief

     \param y
    */
    void setBgColor(QByteArray &y);
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
    void setBgColorOfInterestings();

protected:
    /**
     * @brief
     *
     * @param obj
     * @param ev
     * @return bool
     */
    /**
     * @brief
     *
     * @param obj
     * @param ev
     * @return bool
     */
    /*!
     \brief

     \param obj
     \param ev
     \return bool
    */
    bool eventFilter(QObject *obj, QEvent *ev);
private:
    Ui::Register *ui;   
    QLabel *labelArrC1[MAX_ROWS + 3];   
    QLineEdit *leArrC2[MAX_ROWS + 3];   
    QSpacerItem *spacer;   
    QVector< uint8_t > vInterest;   
//    QSettings config;
};

#endif // REGISTER_H
