#include "timetracker.hpp"

#include <QApplication>
#include <QLocale>
#include <QFile>
#include <QTranslator>
#include <QMessageBox>

QScopedPointer<QFile> logFile;

int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(app);

  QApplication app(argc, argv);
  logFile.reset(new QFile("log.txt"));
  logFile.data()->open(QFile::Append | QFile::Text);
  qInstallMessageHandler(messageHandler);

  if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
      QMessageBox::critical(nullptr, QObject::tr("System tray"), QObject::tr("Can't hind intp the system tray."));
      return 1;
    }

  QCoreApplication::setOrganizationName("OpenSource");
  QCoreApplication::setOrganizationDomain("https://github.com/Pugnator");
  QCoreApplication::setApplicationName("TimeTracker");
  QApplication::setApplicationVersion("1.0.0");

  QFile file("stylesheet.сss");
  file.open(QFile::ReadOnly);
  QString styleSheet = QLatin1String(file.readAll());
  app.setStyleSheet(styleSheet);
  TimeTracker w;
  app.setQuitOnLastWindowClosed(true);
  return app.exec();
}
