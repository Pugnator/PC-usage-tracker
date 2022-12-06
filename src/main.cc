#include "timetracker.hpp"

#include <QApplication>
#include <QLocale>
#include <QFile>
#include <QTranslator>
#include <QMessageBox>

int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(app);
  QApplication a(argc, argv);

  if (!QSystemTrayIcon::isSystemTrayAvailable())
  {
    QMessageBox::critical(nullptr, QObject::tr("Трей"), QObject::tr("Не могу свернуться в трей."));
    return 1;
  }

  QCoreApplication::setOrganizationName("OpenSource");
  QCoreApplication::setOrganizationDomain("https://github.com/Pugnator");
  QCoreApplication::setApplicationName("TimeTracker");
  QApplication::setApplicationVersion("1.0.0");

  QFile file("stylesheet.сss");
  file.open(QFile::ReadOnly);
  QString styleSheet = QLatin1String(file.readAll());
  a.setStyleSheet(styleSheet);
  TimeTracker w;
  return a.exec();
}
