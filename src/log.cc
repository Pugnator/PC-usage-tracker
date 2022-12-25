#include "timetracker.hpp"
#include <QMessageBox>

void TimeTracker::error(QString text)
{
  QMessageBox::critical(this, "Ошибка", text);
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  QDateTime date = QDateTime::currentDateTime();
  QString formattedTime = date.toString("dd.MM.yyyy hh:mm:ss");
  QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
  QByteArray localMsg = msg.toLocal8Bit();
  QTextStream ts(logFile.get());
  QString txt;
  switch (type)
    {
    case QtDebugMsg:
      txt = QString::asprintf("[%s] Debug: %s", formattedTimeMsg.constData(), localMsg.constData());
      break;
    case QtInfoMsg:
      txt = QString::asprintf("[%s] Info: %s", formattedTimeMsg.constData(), localMsg.constData());
      break;
    case QtWarningMsg:
      txt = QString::asprintf("[%s] Warning: %s", formattedTimeMsg.constData(), localMsg.constData());
      break;
    case QtCriticalMsg:
      txt = QString::asprintf("[%s] Critical: %s", formattedTimeMsg.constData(), localMsg.constData());
      break;
    case QtFatalMsg:
      txt = QString::asprintf("[%s] Fatal: %s", formattedTimeMsg.constData(), localMsg.constData());
      ts << txt << Qt::endl;
      abort();
    }
  ts << txt << Qt::endl;
}
