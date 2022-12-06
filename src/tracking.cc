#include "timetracker.hpp"
#include "./ui_timetracker.h"
#include <winuser.h>
#include <sysinfoapi.h>

#define USER_IDLING_DELAY 60

void TimeTracker::updateApplicationAtWorkUsage(QString name, std::chrono::seconds time)
{
  QSqlQuery checkAppWork;
  checkAppWork.prepare("SELECT name FROM ApplicationWorktimeUsage WHERE day=DATE('now', 'localtime') AND name=:appName");
  checkAppWork.bindValue(":appName", name);
  checkAppWork.exec();
  if (!checkAppWork.next())
  {
    QSqlQuery insertQueryWork;
    insertQueryWork.prepare("INSERT OR IGNORE INTO ApplicationWorktimeUsage(name, usage,day,last) VALUES(:appName, 0, DATE('now', 'localtime'),DATETIME('now', 'localtime'))");
    insertQueryWork.bindValue(":appName", name);
    insertQueryWork.exec();
  }
  QSqlQuery updateQueryWork;

  updateQueryWork.prepare("UPDATE ApplicationWorktimeUsage SET usage = usage + :timeIncrement, last=DATETIME('now', 'localtime') WHERE day = DATE('now', 'localtime') AND name=:appName");
  updateQueryWork.bindValue(":appName", name);
  updateQueryWork.bindValue(":timeIncrement", time.count());
  updateQueryWork.exec();
}

void TimeTracker::updateApplicationUsage(QString name, std::chrono::seconds time)
{
  if (QDate::currentDate() > currentSession)
  {
    qDebug("New day!");
    // it's a new day
    //! FIXME: this is a shitty temp solution
    //! BUG: https://github.com/Pugnator/ApplicationTimeTracker/issues/1

    updateDailyStats();

    currentSession = QDate::currentDate();
    loggedOnTime = 0s;
    loggedOffTime = 0s;
    userIdlingTime = 0s;
    activityTimer->reset();
    logonTimer->reset();
    appModelSetup(showWorkShiftStatsOnly ? "ApplicationWorktimeUsage" : "ApplicationUsage");
  }
  if (isSystemLocked)
  {
    loggedOffTime++;
    return;
  }

  loggedOnTime++;
  if (!trackingEnabled)
  {
    return;
  }

  bool isWorkShift = false;
  QDateTime curTime = QDateTime::currentDateTime();
  if (curTime.time() >= shiftStart && curTime.time() <= shiftEnd)
  {
    isWorkShift = true;
  }

  if (name.trimmed().isEmpty())
  {
    // some system windowless apps or no actual window is in focus
    return;
  }
  appUsageModel->select();

  QSqlQuery checkApp;
  checkApp.prepare("SELECT name FROM ApplicationUsage WHERE name=:appName AND day=DATE('now', 'localtime')");
  checkApp.bindValue(":appName", name);
  checkApp.exec();

  QSqlQuery insertQuery;
  if (!checkApp.next())
  {
    insertQuery.prepare("INSERT OR IGNORE INTO ApplicationUsage(name, usage,day,last) VALUES(:appName, 0, DATE('now', 'localtime'),DATETIME('now', 'localtime'))");
    insertQuery.bindValue(":appName", name);
    if (!insertQuery.exec())
    {
      qDebug("Failure");
    }
  }
  else
  {
    insertQuery.prepare("UPDATE ApplicationUsage SET usage = usage + :timeIncrement, last=DATETIME('now', 'localtime') WHERE day = DATE('now', 'localtime') AND name=:appName");
    insertQuery.bindValue(":appName", name);
    insertQuery.bindValue(":timeIncrement", time.count());
    insertQuery.exec();
    if (isWorkShift)
    {
      updateApplicationAtWorkUsage(name, time);
    }
  }

  appUsageModel->submitAll();

  LASTINPUTINFO info;
  info.cbSize = sizeof(info);
  GetLastInputInfo(&info);
  DWORD idleSeconds = (GetTickCount() - info.dwTime) / 1000;
  if (idleSeconds > USER_IDLING_DELAY)
  {
    userIdlingTime++;
    activityTimer->pause();
  }
  else
  {
    activityTimer->start();
  }
}

void TimeTracker::updateDailyStats()
{
  appUsageModel->select();
  QSqlQuery insertQuery, updateQuery;
  insertQuery.prepare("INSERT OR IGNORE INTO DailyUsage(logon, logoff, idle, day) VALUES(:logon, :logoff, :idle, date(:session))");
  updateQuery.prepare("UPDATE DailyUsage SET logon = logon + :logon, idle = idle + :idle, logoff = logoff + :logoff WHERE day=date(:session)");

  insertQuery.bindValue(":logon", loggedOnTime.count());
  insertQuery.bindValue(":logoff", loggedOffTime.count());
  insertQuery.bindValue(":idle", userIdlingTime.count());
  insertQuery.bindValue(":session", currentSession.toString("yyyy-MM-dd"));
  insertQuery.exec();

  updateQuery.bindValue(":logon", loggedOnTime.count());
  updateQuery.bindValue(":logoff", loggedOffTime.count());
  updateQuery.bindValue(":idle", userIdlingTime.count());
  updateQuery.bindValue(":session", currentSession.toString("yyyy-MM-dd"));
  updateQuery.exec();

  appUsageModel->submitAll();
}
