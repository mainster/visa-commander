
#ifndef HLED_H
#define HLED_H

#include <QWidget>

class QColor;

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
class HLed : public QWidget
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
    HLed(QWidget *parent = 0);
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
    ~HLed();

    /**
     * @brief
     *
     * @return QColor
     */
    /**
     * @brief
     *
     * @return QColor
     */
    /*!
     \brief

     \return QColor
    */
    QColor color() const;
    /**
     * @brief
     *
     * @return QSize
     */
    /**
     * @brief
     *
     * @return QSize
     */
    /*!
     \brief

     \return QSize
    */
    QSize sizeHint() const;
    /**
     * @brief
     *
     * @return QSize
     */
    /**
     * @brief
     *
     * @return QSize
     */
    /*!
     \brief

     \return QSize
    */
    QSize minimumSizeHint() const;

public slots:
    /**
     * @brief
     *
     * @param color
     */
    /**
     * @brief
     *
     * @param color
     */
    /*!
     \brief

     \param color
    */
    void setColor(const QColor &color);
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
    void toggle();
    /**
     * @brief
     *
     * @param on
     */
    /**
     * @brief
     *
     * @param on
     */
    /*!
     \brief

     \param on
    */
    void turnOn(bool on=true);
    /**
     * @brief
     *
     * @param off
     */
    /**
     * @brief
     *
     * @param off
     */
    /*!
     \brief

     \param off
    */
    void turnOff(bool off=true);

protected:
    /**
     * @brief
     *
     * @param
     */
    /**
     * @brief
     *
     * @param
     */
    /*!
     \brief

     \param
    */
    void paintEvent(QPaintEvent *);
    /**
     * @brief
     *
     * @return int
     */
    /**
     * @brief
     *
     * @return int
     */
    /*!
     \brief

     \return int
    */
    int ledWidth() const;

private:
    struct Private;
    Private * const m_d;   
};

#endif // HLED_H
