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
  if (QDate::currentDate() > currentSession_)
  {
    qDebug("New day!");
    // it's a new day
    //! FIXME: this is a shitty temp solution
    //! BUG: https://github.com/Pugnator/ApplicationTimeTracker/issues/1

    updateDailyStats();

    currentSession_ = QDate::currentDate();
    loggedOnTime_ = 0s;
    loggedOffTime_ = 0s;
    userIdlingTime_ = 0s;
    activityTimer->reset();
    logonTimer->reset();
    appModelSetup(showWorkShiftStatsOnly_ ? "ApplicationWorktimeUsage" : "ApplicationUsage");
  }

  if (isSystemLocked_)
  {
    loggedOffTime_++;
    if (isHaveToRest_)
      {
        qDebug() << "resting...";
        haveToRest_--;
        if(haveToRest_ <= 0s)
          {
            qDebug() << "Rest finished";
            isHaveToRest_ = false;
            userWorkTime_ = 0s;
            emit deleteTrayMessage();
            setRestTimer();
          }
      }
    return;
  }

  loggedOnTime_++;
  if (!trackingEnabled_)
  {
    return;
  }

  if(restControlEnabled_ && !isHaveToRest_ && userWorkTime_ > maxWorkTimeInRow_)
    {
      qDebug("You have to rest");
      isHaveToRest_ = true;      
      doSomeRestNotification();
      if(LockWorkStation())
        {
          qDebug("Locked the workstation");
          return;
        }
    }
  else if (isHaveToRest_)
    {
      qDebug("Ignoring rest");
    }

  userWorkTime_++;

  if(restControlEnabled_ && maxWorkTimeInRow_ - userWorkTime_  < 350s)
    {
      haveToRest_ = timerToRest_;
      setRestTimer();
      emit showTrayMessage("Less than 5min left");
    }


  bool isWorkShift = false;
  QDateTime curTime = QDateTime::currentDateTime();
  if (curTime.time() >= shiftStart_ && curTime.time() <= shiftEnd_)
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
    userIdlingTime_++;
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
  insertQuery.prepare("INSERT OR IGNORE INTO DailyUsage(logon, logoff, idle, day, haveToRest) VALUES(:logon, :logoff, :idle, date(:session), :haveToRest)");
  updateQuery.prepare("UPDATE DailyUsage SET haveToRest = :haveToRest, logon = :logon, idle = :idle, logoff = :logoff WHERE day=date(:session)");
  qDebug() << haveToRest_.count();
  insertQuery.bindValue(":haveToRest", haveToRest_.count());
  insertQuery.bindValue(":logon", loggedOnTime_.count());
  insertQuery.bindValue(":logoff", loggedOffTime_.count());
  insertQuery.bindValue(":idle", userIdlingTime_.count());
  insertQuery.bindValue(":session", currentSession_.toString("yyyy-MM-dd"));
  insertQuery.exec();

  updateQuery.bindValue(":haveToRest", haveToRest_.count());
  updateQuery.bindValue(":logon", loggedOnTime_.count());
  updateQuery.bindValue(":logoff", loggedOffTime_.count());
  updateQuery.bindValue(":idle", userIdlingTime_.count());
  updateQuery.bindValue(":session", currentSession_.toString("yyyy-MM-dd"));
  updateQuery.exec();

  appUsageModel->submitAll();
}

void TimeTracker::setRestTimer()
{
  updateDailyStats();
}
void TimeTracker::getRestTimer()
{

}
