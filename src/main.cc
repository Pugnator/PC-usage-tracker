#include "timetracker.hpp"
#include <tlhelp32.h>
#include "windows.h"

#include <QApplication>
#include <QLocale>
#include <QFile>
#include <QTranslator>
#include <QMessageBox>

QScopedPointer<QFile> logFile;

DWORD findProcessCount(wchar_t *processName)
{
  DWORD count = 0;
  PROCESSENTRY32 processInfo;
  processInfo.dwSize = sizeof(processInfo);

  HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
  if ( processesSnapshot == INVALID_HANDLE_VALUE )
    {
      return 0;
    }

  Process32First(processesSnapshot, &processInfo);
  if ( !wcscmp(processName, processInfo.szExeFile) )
    {
      count++;
    }

  while ( Process32Next(processesSnapshot, &processInfo) )
    {
      if ( !wcscmp(processName, processInfo.szExeFile) )
        {
          count++;
        }
    }
  CloseHandle(processesSnapshot);
  return count;
}

bool checkAlreadyRunning()
{
  wchar_t selfFileName[MAX_PATH] = {0};
  GetModuleFileName(NULL, selfFileName, MAX_PATH);
  auto name = QString::fromWCharArray(selfFileName);
  name = name.mid(name.lastIndexOf("\\"));
  name.remove(QChar('\\'));
  return findProcessCount(const_cast<wchar_t*>(name.toStdWString().c_str())) == 1 ? false : true;
}

int main(int argc, char *argv[])
{
  if(checkAlreadyRunning())
    {
      return 1;
    }
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
