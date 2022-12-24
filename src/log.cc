#include "timetracker.hpp"
#include <QMessageBox>

void TimeTracker::error(QString text)
{
  QMessageBox::critical(this, "Ошибка", text);
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  QByteArray localMsg = msg.toLocal8Bit();
  QTextStream ts(logFile.get());
  QString txt;
  switch (type)
    {
    case QtDebugMsg:
      txt = QString::asprintf("Debug: %s", localMsg.constData());
      break;
    case QtInfoMsg:
      txt = QString::asprintf("Info: %s", localMsg.constData());
      break;
    case QtWarningMsg:
      txt = QString::asprintf("Warning: %s", localMsg.constData());
      break;
    case QtCriticalMsg:
      txt = QString::asprintf("Critical: %s", localMsg.constData());
      break;
    case QtFatalMsg:
      txt = QString::asprintf("Fatal: %s", localMsg.constData());
      ts << txt << Qt::endl;
      abort();
    }
  ts << txt << Qt::endl;
}
