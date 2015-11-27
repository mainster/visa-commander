
/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef PortDialog_H
#define PortDialog_H

#include <QDialog>
#include <QSerialPort>
QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

namespace Ui {
class PortDialog;
}

class QIntValidator;

QT_END_NAMESPACE

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
class PortDialog : public QDialog
{
    Q_OBJECT

public:
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
    struct Settings {
        QString name;   
        qint32 baudRate;   
        QString stringBaudRate;   
        QSerialPort::DataBits dataBits;   
        QString stringDataBits;   
        QSerialPort::Parity parity;   
        QString stringParity;   
        QSerialPort::StopBits stopBits;   
        QString stringStopBits;   
        QSerialPort::FlowControl flowControl;   
        QString stringFlowControl;   
        bool localEchoEnabled;   
        QIODevice::OpenModeFlag openMode;   
    };

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
    explicit PortDialog(QDialog *parent = 0);
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
    ~PortDialog();

    /**
     * @brief
     *
     * @return Settings
     */
    /**
     * @brief
     *
     * @return Settings
     */
    /*!
     \brief

     \return Settings
    */
    Settings settings() const;

private slots:
    /**
     * @brief
     *
     * @param idx
     */
    /**
     * @brief
     *
     * @param idx
     */
    /*!
     \brief

     \param idx
    */
    void showPortInfo(int idx);
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
    void apply();
    /**
     * @brief
     *
     * @param idx
     */
    /**
     * @brief
     *
     * @param idx
     */
    /*!
     \brief

     \param idx
    */
    void checkCustomBaudRatePolicy(int idx);

private:
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
    void fillPortsParameters();
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
    void fillPortsInfo();
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
    void updateSettings();
    /**
     * @brief
     *
     * @return bool
     */
    /**
     * @brief
     *
     * @return bool
     */
    /*!
     \brief

     \return bool
    */
    bool savePersistancePortDialog();
    /**
     * @brief
     *
     * @return bool
     */
    /**
     * @brief
     *
     * @return bool
     */
    /*!
     \brief

     \return bool
    */
    bool loadPersistancePortDialog();

private:
    Ui::PortDialog *ui;   
    Settings currentSettings;   
    QIntValidator *intValidator;   
};

#endif // PortDialog_H
