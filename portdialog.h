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

class PortDialog : public QDialog
{
    Q_OBJECT

public:
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
    Settings settings() const;

    explicit PortDialog(QDialog *parent = 0);
    ~PortDialog();

private slots:
    void showPortInfo(int idx);
    void apply();
    void checkCustomBaudRatePolicy(int idx);

private:
    void fillPortsParameters();
    void fillPortsInfo();
    void updateSettings();
    bool savePersistancePortDialog();
    bool loadPersistancePortDialog();

    Ui::PortDialog *ui;   
    Settings currentSettings;   
    QIntValidator *intValidator;   
};

#endif // PortDialog_H
