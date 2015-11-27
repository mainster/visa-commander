
#ifndef FILEBACKUP_H
#define FILEBACKUP_H
#include <QTime>
#include <QString>
#include <QFile>
#include <QMessageBox>
#include <QObject>
#include <QDebug>

#define TMP tr("/tmp/")

namespace Qt {
enum fileMode {
   overwrite,
};
Q_DECLARE_FLAGS(fileModes, fileMode)

}

class FileBackup : public QFile {

   Q_OBJECT

public:
   FileBackup(){;}


   static QString doBackup(QString pathToFile,
                           QString toTargetDir = "") {
      static QString filename;
      static QString fileDir;
      static QString toTargetFile;

      /** Check if file exists and inform user if not */
      if (! exists(pathToFile)) {
         switch (msgBox("File " + pathToFile + " does not exist!")) {
            case QMessageBox::Cancel:
               return tr(""); break;
            case QMessageBox::Save:
               qDebug() << "makedir"; break;
            default: break;
         }
      }

      QStringList lst = pathToFile.split("/");
      filename = lst.last();
      lst.removeLast();
      fileDir = QString( lst.join(QChar('/')));

      QTime time = QTime::currentTime();
      QString timestr =
            QString::number(time.hour()) +
            QString::number(time.minute()) +
            QString::number(time.second());
      if (toTargetDir.isEmpty())
         toTargetFile = fileDir;
      else
         toTargetFile = toTargetDir;

      if (toTargetFile.at(toTargetFile.length() - 1) != QChar('/'))
         toTargetFile += QChar('/');

      toTargetFile += filename.replace(".", tr("_") + timestr + ".");

      QFile::copy(pathToFile, toTargetFile);
      QFile::setPermissions(toTargetFile,
                            QFileDevice::WriteOwner |
                            QFileDevice::WriteUser |
                            QFileDevice::WriteGroup);
      return toTargetFile;
   }

   static QString toFilesys(QStringList lines,
                            QString filename) {

      static QString toTargetFile;

      /** prepend default dir if parameter "file" is not an absolute path */
      if (filename.at(0) != '/')
         toTargetFile = TMP + filename;
      else
         toTargetFile = filename;

      QTime time = QTime::currentTime();
      QString timestr =
            QString::number(time.hour()) +
            QString::number(time.minute()) +
            QString::number(time.second());

      /** Check if file exists and move it to backup */
      if (exists(toTargetFile)) {
         QFile fileold(toTargetFile);
         fileold.rename(toTargetFile, toTargetFile + "_" + timestr);
         fileold.close();
      }

      /** Check if file exists and inform user if so */
      if (exists(toTargetFile)) {
         switch (msgBox("File " + toTargetFile + " does exist!")) {
            case QMessageBox::Cancel:
               return tr(""); break;
            case QMessageBox::Save: {
               qDebug() << "Overwrite implementation";
               return "";
            }; break;
            default: break;
         }
      }

      QFile file(toTargetFile);
      if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
              return "";

      QTextStream out(&file);

      for (int i = 0; i < lines.length(); i++) {
         out << lines.at(i) << "\n";
      }
      file.close();

      return toTargetFile;
   }

   static int msgBox(QString subject,
                     QString informativeText)  {
      QMessageBox m_msgBox;
      m_msgBox.setText( subject );
      m_msgBox.setInformativeText( informativeText );
      m_msgBox.setStandardButtons(QMessageBox::Save |
                                  QMessageBox::Cancel);
      m_msgBox.setDefaultButton(QMessageBox::Save);
      return m_msgBox.exec();
   }

   static int msgBox(QString subject)  {
      QMessageBox m_msgBox;
      m_msgBox.setText( subject );
      m_msgBox.setStandardButtons(QMessageBox::Save |
                                  QMessageBox::Cancel);
      m_msgBox.setDefaultButton(QMessageBox::Save);
      return m_msgBox.exec();
   }

   ~FileBackup() {;}

private:

};
#endif // FILEBACKUP_H


