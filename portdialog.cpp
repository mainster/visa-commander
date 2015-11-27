#include <QIntValidator>
#include <QLineEdit>
#include <QSettings>
#include <QTime>
#include <QFile>
#include <QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "portdialog.h"
#include "ui_portdialog.h"
#include "globals.h"


QT_USE_NAMESPACE

PortDialog::PortDialog(QDialog *parent) :
   QDialog(parent),
   ui(new Ui::PortDialog) {
   ui->setupUi(this);

   intValidator = new QIntValidator(0, 4000000, this);

   ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

   connect(ui->applyButton, SIGNAL(clicked()),
           this, SLOT(apply()));
   connect(ui->serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
           this, SLOT(showPortInfo(int)));
   connect(ui->baudRateBox, SIGNAL(currentIndexChanged(int)),
           this, SLOT(checkCustomBaudRatePolicy(int)));

   fillPortsParameters();
   fillPortsInfo();

   loadPersistancePortDialog();
   updateSettings();
}
PortDialog::~PortDialog() {
   delete ui;
}
PortDialog::Settings PortDialog::settings() const {
   return currentSettings;
}
void PortDialog::showPortInfo(int idx) {
   if (idx != -1) {
      QStringList list = ui->serialPortInfoListBox->itemData(idx).toStringList();
      ui->descriptionLabel->setText(tr("Description: %1").arg(list.at(1)));
      ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.at(2)));
      ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.at(3)));
      ui->locationLabel->setText(tr("Location: %1").arg(list.at(4)));
      ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.at(5)));
      ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.at(6)));
   }
}
void PortDialog::apply() {
   updateSettings();
   hide();
}
void PortDialog::checkCustomBaudRatePolicy(int idx) {
   bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
   ui->baudRateBox->setEditable(isCustomBaudRate);
   if (isCustomBaudRate) {
      ui->baudRateBox->clearEditText();
      QLineEdit *edit = ui->baudRateBox->lineEdit();
      edit->setValidator(intValidator);
   }
}
void PortDialog::fillPortsParameters() {
   ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
   ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
   ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
   ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
   ui->baudRateBox->addItem(QStringLiteral("921600"), 921600);
   ui->baudRateBox->addItem(QStringLiteral("Custom"));

   ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
   ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
   ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
   ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
   ui->dataBitsBox->setCurrentIndex(3);

   ui->parityBox->addItem(QStringLiteral("None"), QSerialPort::NoParity);
   ui->parityBox->addItem(QStringLiteral("Even"), QSerialPort::EvenParity);
   ui->parityBox->addItem(QStringLiteral("Odd"), QSerialPort::OddParity);
   ui->parityBox->addItem(QStringLiteral("Mark"), QSerialPort::MarkParity);
   ui->parityBox->addItem(QStringLiteral("Space"), QSerialPort::SpaceParity);

   ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
   ui->stopBitsBox->addItem(QStringLiteral("1.5"), QSerialPort::OneAndHalfStop);
#endif
   ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

   ui->flowControlBox->addItem(QStringLiteral("None"), QSerialPort::NoFlowControl);
   ui->flowControlBox->addItem(QStringLiteral("RTS/CTS"), QSerialPort::HardwareControl);
   ui->flowControlBox->addItem(QStringLiteral("XON/XOFF"), QSerialPort::SoftwareControl);
}
void PortDialog::fillPortsInfo() {
   ui->serialPortInfoListBox->clear();
   static const QString blankString = QObject::tr("N/A");
   QString description;
   QString manufacturer;
   QString serialNumber;
   foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
      QStringList list;
      description = info.description();
      manufacturer = info.manufacturer();
      serialNumber = info.serialNumber();
      list << info.portName()
           << (!description.isEmpty() ? description : blankString)
           << (!manufacturer.isEmpty() ? manufacturer : blankString)
           << (!serialNumber.isEmpty() ? serialNumber : blankString)
           << info.systemLocation()
           << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
           << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

      ui->serialPortInfoListBox->addItem(list.first(), list);
   }
}
void PortDialog::updateSettings() {
   currentSettings.name = ui->serialPortInfoListBox->currentText();

   if (ui->baudRateBox->currentIndex() == 4) {
      currentSettings.baudRate = ui->baudRateBox->currentText().toInt();
   } else {
      currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
               ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
   }
   currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

   currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
            ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
   currentSettings.stringDataBits = ui->dataBitsBox->currentText();

   currentSettings.parity = static_cast<QSerialPort::Parity>(
            ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
   currentSettings.stringParity = ui->parityBox->currentText();

   currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
            ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
   currentSettings.stringStopBits = ui->stopBitsBox->currentText();

   currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
            ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
   currentSettings.stringFlowControl = ui->flowControlBox->currentText();

   currentSettings.localEchoEnabled = ui->localEchoCheckBox->isChecked();

   if (ui->cbChooseOpenMode->isChecked())
      currentSettings.openMode = QIODevice::ReadWrite;
   else
      currentSettings.openMode = QIODevice::ReadOnly;

   savePersistancePortDialog();
}
/* @brief MainWin::loadPersistancePortDialog ?!
 * Call this method in the main frame constructor for configance
 * of widget settings. (Text values, slider states, button clicked...)
 *
 * \todo {  Implement "No configfile" or "Configfile not accessable"
 *          Message  }
 */
bool PortDialog::loadPersistancePortDialog() {
   QSETTINGS;
   ///< Load configuration file, pointed to by configFile
   // QSettings config(QString::fromUtf8( &configFile )), QSettings::IniFormat);
   //  QSettings config(configFile, QSettings::IniFormat);
   /** @todo Select port by portname and not by portBox index.*/

   config.beginGroup("Tab_Serial_Settings");  {
      ui->serialPortInfoListBox->setCurrentIndex(config.value("portBoxIdx").toInt());
      ui->baudRateBox->setCurrentIndex(config.value("baudRateBoxIdx").toInt());
      ui->dataBitsBox->setCurrentIndex(config.value("dataBitsBoxIdx").toInt());
      ui->stopBitsBox->setCurrentIndex(config.value("stopBitsBoxIdx").toInt());
      ui->flowControlBox->setCurrentIndex(config.value("flowControlBoxIdx").toInt());
      config.endGroup();
   }
   config.beginGroup("Tab_UI_settings");  {
      //        ui->mainTab->setCurrentIndex(config.value("mainTabIdx").toInt());

      //        QStringList list = config.value("StringList/repeatCustomSeq").toString().split(",");
      //        cbModelRepeatSeq = new QStringListModel(list);
      //        ui->cbRepeatCustSeq->setModel(cbModelRepeatSeq);

      config.endGroup();
   }
   return true;
}
/* @brief MainWin::savePersistancePortDialog
* @param configFile
*
* Call this method at the end of the main frame destructor to store the
* widget states in some where on the filesystem.
*
* \todo {  Implement a "Read only permission for Configfile" Message  }
*/
bool PortDialog::savePersistancePortDialog() {
   /**
     * Save all user settings in a configuration file on the filesytem
     */
   QTime time = QTime::currentTime();
   QString timestr =
         QString::number(time.hour()) +
         QString::number(time.minute()) +
         QString::number(time.second());
   //@@@1MDB//qDebug()() << "Writing config at" << time.toString() << "h";
   QFile::copy(CONFIG_PATH, QString(CONFIG_PATH) + "_" + timestr);
   QSETTINGS;

   config.beginGroup("Tab_Serial_Settings");  {
      config.setValue("portBoxIdx", ui->serialPortInfoListBox->currentIndex());
      config.setValue("baudRateBoxIdx", ui->baudRateBox->currentIndex());
      config.setValue("dataBitsBoxIdx", ui->dataBitsBox->currentIndex());
      config.setValue("stopBitsBoxIdx", ui->stopBitsBox->currentIndex());
      config.setValue("flowControlBoxIdx", ui->flowControlBox->currentIndex());
      config.setValue("String/portBox", ui->serialPortInfoListBox->currentText());
      config.setValue("String/baudRateBox", ui->baudRateBox->currentText());
      config.setValue("String/dataBitsBox", ui->dataBitsBox->currentText());
      config.setValue("String/stopBitsBox", ui->stopBitsBox->currentText());
      config.endGroup();
   }
   config.beginGroup("Tab_UI_settings");  {
      //        config.setValue("mainTabIdx", ui->mainTab->currentIndex());

      //        QStringList list = cbModelRepeatSeq->stringList();
      //        config.setValue("StringList/repeatCustomSeq", list.join(","));
      config.endGroup();
   }
   return true;
}
