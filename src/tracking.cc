#include "timetracker.hpp"
#include "./ui_timetracker.h"
#include <winuser.h>
#include <wtsapi32.h>
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

void TimeTracker::updateAppStats(QString name, std::chrono::seconds time)
{
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
          qInfo("Failure");
        }
    }
  else
    {
      insertQuery.prepare("UPDATE ApplicationUsage SET usage = usage + :timeIncrement, last=DATETIME('now', 'localtime') WHERE day = DATE('now', 'localtime') AND name=:appName");
      insertQuery.bindValue(":appName", name);
      insertQuery.bindValue(":timeIncrement", time.count());
      insertQuery.exec();
      bool isWorkShift = false;
      QDateTime curTime = QDateTime::currentDateTime();
      if (curTime.time() >= shiftStart_ && curTime.time() <= shiftEnd_)
        {
          isWorkShift = true;
        }
      if (isWorkShift)
        {
          updateApplicationAtWorkUsage(name, time);
        }
    }

  appUsageModel->submitAll();
}

void TimeTracker::onNewDayAction()
{
  qInfo("New day, resettings stats.");
  // it's a new day
  //! FIXME: this is a shitty temp solution
  //! BUG: https://github.com/Pugnator/ApplicationTimeTracker/issues/1

  updateDailyStats();

  currentSession_ = QDate::currentDate();
  daylyLoggedOnTime_ = 0s;
  daylyLoggedOffTime_ = 0s;
  daylyIdlingTime_ = 0s;
  activityTimer->reset();
  logonTimer->reset();
  appModelSetup(showWorkShiftStatsOnly_ ? "ApplicationWorktimeUsage" : "ApplicationUsage");
}

bool TimeTracker::lockSystem()
{
  isHaveToRest_ = true;
  PWTS_SESSION_INFO pSessionInfo = NULL;
  HANDLE hServer = WTS_CURRENT_SERVER_HANDLE;
  DWORD count = 0;

  BOOL result = WTSEnumerateSessions(hServer, 0, 1, &pSessionInfo, &count);

  if (!result)
    {
      qCritical("WTSEnumerateSessions failed.");
      return false;
    }

  BOOL isAnyDiscFailed = false;

  qInfo("User sessions count: %lu.", count);
  for (auto index = 0; index < count; index++)
    {
      qInfo("[%u] SessionId: %lu State: %u.", index,
              pSessionInfo[index].SessionId,
              pSessionInfo[index].State);
      qInfo("User has to rest.");
      BOOL discRes = WTSDisconnectSession(hServer, pSessionInfo[index].SessionId, FALSE);
      if(!discRes)
        {
          isAnyDiscFailed = true;
          qCritical("Failed to disconnect user %lu.", pSessionInfo[index].SessionId);
        }
    }

  return !isAnyDiscFailed;
}

void TimeTracker::onTimerTick(QString name, std::chrono::seconds time)
{  
  if (QDate::currentDate() > currentSession_)
    {
      onNewDayAction();
    }

  if (isSystemLocked_)
    {
      daylyLoggedOffTime_++;
      if (isHaveToRest_)
        {
          timeUserHasToRest_--;
          if(timeUserHasToRest_ <= 0s)
            {
              qInfo() << "Rest finished, user can log on.";
              isHaveToRest_ = false;
              timeLeftToLock_ = maxWorkTimeInRow_;
              timeEndWarningShown_ = false;
              setRestTimer();
            }
        }
      else
        {

          if(timeLeftToLock_ < maxWorkTimeInRow_)
            {
              timeLeftToLock_++;
            }
        }
      return;
    }

  if(maxWorkPerDayTime_ > 0s && daylyLoggedOnTime_ >= maxWorkPerDayTime_)
    {
      qInfo("Dayly limit reached [%lld], logging off.", maxWorkPerDayTime_.count());
      lockSystem();
    }

  daylyLoggedOnTime_++;
  if (!trackingEnabled_)
    {
      return;
    }

  if(restControlEnabled_ && !isHaveToRest_ && timeLeftToLock_ == 0s)
    {
      lockSystem();
    }
  else if (isHaveToRest_)
    {
      qWarning("Logic error, user is logged on when it should rest.");
    }

  if(restControlEnabled_ && timeLeftToLock_ > 0s)
    {
      timeLeftToLock_--;
    }


  if(restControlEnabled_ && timeLeftToLock_  < 350s)
    {
      timeUserHasToRest_ = timerToRest_;
      setRestTimer();
      if(!timeEndWarningShown_)
        {
          showNativeMessage(WTS_CURRENT_SESSION, "Less than 5mins left.");
          qWarning("Less than %lldsecs left.", timeLeftToLock_.count());
          emit showTrayMessage("Less than 5min left.");
        }

      timeEndWarningShown_ = true;
    }

  if (name.trimmed().isEmpty())
    {
      // some system windowless apps or no actual window is in focus
      return;
    }

  updateAppStats(name, time);

  LASTINPUTINFO info;
  info.cbSize = sizeof(info);
  GetLastInputInfo(&info);
  DWORD idleSeconds = (GetTickCount() - info.dwTime) / 1000;
  if (idleSeconds > USER_IDLING_DELAY)
    {
      daylyIdlingTime_++;
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
  insertQuery.prepare("INSERT OR IGNORE INTO DailyUsage(logon, logoff, idle, day, haveToRest,timeLeftToLock) VALUES(:logon, :logoff, :idle, date(:session), :haveToRest, :timeLeftToLock)");
  updateQuery.prepare("UPDATE DailyUsage SET timeLeftToLock = :timeLeftToLock, haveToRest = :haveToRest, logon = :logon, idle = :idle, logoff = :logoff WHERE day=date(:session)");

  insertQuery.bindValue(":timeLeftToLock", timeLeftToLock_.count());
  insertQuery.bindValue(":haveToRest", timeUserHasToRest_.count());
  insertQuery.bindValue(":logon", daylyLoggedOnTime_.count());
  insertQuery.bindValue(":logoff", daylyLoggedOffTime_.count());
  insertQuery.bindValue(":idle", daylyIdlingTime_.count());
  insertQuery.bindValue(":session", currentSession_.toString("yyyy-MM-dd"));
  insertQuery.exec();

  updateQuery.bindValue(":timeLeftToLock", timeLeftToLock_.count());
  updateQuery.bindValue(":haveToRest", timeUserHasToRest_.count());
  updateQuery.bindValue(":logon", daylyLoggedOnTime_.count());
  updateQuery.bindValue(":logoff", daylyLoggedOffTime_.count());
  updateQuery.bindValue(":idle", daylyIdlingTime_.count());
  updateQuery.bindValue(":session", currentSession_.toString("yyyy-MM-dd"));
  updateQuery.exec();

  appUsageModel->submitAll();
}

void TimeTracker::setRestTimer()
{  
  updateDailyStats();
}

