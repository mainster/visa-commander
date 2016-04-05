#include <QByteArray>
#include <QString>
#include <QSettings>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QtCore>
#include <QStandardPaths>

#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>

#include "globals.h"

#define  DEFAULT_PATH_TO_DAT     "/opt/Visatronic/visascope-3.1/share/hardware_values/hardware_137000.dat"
#define  DEFAULT_PATH_TO_BACKUP  "/tmp/"

const QStringList DEFAULT_PATH_TO_CONFIG = QStringList()
      << "$HOME/.config/visa-commander/config"
      << "$HOME/.visa-commander/config";

Globals *Globals::inst = 0x00;


Globals::Globals() {
   Q_INFO << "Try to read config file...";

//   Q_INFO << QStandardPaths::
//          locate(QStandardPaths::ConfigLocation, "visa-commander",
//                 QStandardPaths::LocateDirectory);

//   Q_INFO << QStandardPaths::
//          locate(QStandardPaths::AppConfigLocation, "visa-commander",
//                 QStandardPaths::LocateDirectory);



   Q_INFO << "DesktopLocation" << QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
   Q_INFO << "DocumentsLocation" << QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
   Q_INFO << "FontsLocation" << QStandardPaths::standardLocations(QStandardPaths::FontsLocation);
   Q_INFO << "ApplicationsLocation" << QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
   Q_INFO << "MusicLocation" << QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
   Q_INFO << "MoviesLocation" << QStandardPaths::standardLocations(QStandardPaths::MoviesLocation);
   Q_INFO << "PicturesLocation" << QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
   Q_INFO << "TempLocation" << QStandardPaths::standardLocations(QStandardPaths::TempLocation);
   Q_INFO << "HomeLocation" << QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
   Q_INFO << "DataLocation" << QStandardPaths::standardLocations(QStandardPaths::DataLocation);
   Q_INFO << "CacheLocation" << QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
   Q_INFO << "GenericDataLocation" << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
   Q_INFO << "RuntimeLocation" << QStandardPaths::standardLocations(QStandardPaths::RuntimeLocation);
   Q_INFO << "ConfigLocation" << QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
   Q_INFO << "DownloadLocation" << QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
   Q_INFO << "GenericCacheLocation" << QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation);
   Q_INFO << "GenericConfigLocation" << QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
   Q_INFO << "AppDataLocation" << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
   Q_INFO << "AppConfigLocation" << QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
   Q_INFO << "AppLocalDataLocatio" << QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);

//   conf = new
//   QStringList envs;
//   pathToConfig->append( DEFAULT_PATH_TO_CONFIG );
//   /** Search for valied config file within DEFAULT_PATH_TO_CONFIG */
//   foreach (QString *s, pathToConfig) {
//      while (s->contains("$")) {
//         envs << s->mid( s->indexOf("$",0), s->indexOf("/",0)
//      }
//      QSettings(s applicationName()
//   }
//   QSettings config(getPathToConfig(), QSettings::IniFormat);
//   Q_INFO << config.allKeys();
//   Q_INFO << getPathToConfig();
//   QUrl ur = getPathToConfig2();
//   Q_INFO << ur.path(QUrl::FullyDecoded );
//   //   Q_INFO << QUrl::path(getPathToConfig2(), QUrl::FullyDecoded);

//   QByteArray ba1 = qgetenv("$HOME");
//   QByteArray ba2 = qgetenv("$HOME/.config/visa-commander/config");

//   Q_INFO << ba1 << ba2;
//   Q_INFO << qgetenv("HOME");

//   if (config.status() == QSettings::NoError) {

//      tmp = config.value("paths/pathToBackup").toString();
//      if (! tmp.isEmpty())
//         setPathToBackup( tmp );

//      tmp = config.value("paths/pathToDat").toString();
//      if (! tmp.isEmpty())
//         setPathToDat( tmp );
//   }
//   else {
//      Q_INFO << "config file access error";
//   }
}

Globals::~Globals() {

}

void Globals::setPathToDat(QString str) {
   pathToDat = str;
}
void Globals::setPathToBackup(const QString &str) {
   pathToBackup = str;
}
void Globals::setPathToConfig(const QString &str) {
   pathToConfigss = str;
}
QString Globals::getPathToDat() {
    return QString("/opt/Visatronic/visascope-3.1/share/hardware_values/hardware_137000.dat");
//   return pathToDat;
}
QString Globals::getPathToBackup() {
   //   if (pathToBackup.contains("$"))
   //      return qgetenv(pathToBackup).constData();
   //   else
   return pathToBackup;
}
QString Globals::getPathToConfig() {
   //   if (pathToConfig.contains("$"))
   //      return qgetenv(pathToConfig).constData();
   //   else
   return pathToConfigss;
}
