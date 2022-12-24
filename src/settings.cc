#include "timetracker.hpp"
#include "./ui_timetracker.h"
#include "Windows.h"
#include "Wtsapi32.h"

void TimeTracker::loadSettings()
{
  shiftStart_ = ui->timeShiftStart->time();
  shiftEnd_ = ui->timeShiftEnd->time();
  if (shiftStart_ > shiftEnd_)
  {
    shiftStart_ = shiftEnd_;
  }
  QSqlQuery usageQuery;
  usageQuery.prepare("SELECT logon, idle, logoff, haveToRest, timeLeftToLock FROM DailyUsage WHERE day=DATE('now', 'localtime') LIMIT 1");
  if (!usageQuery.exec())
  {
    return;
  }

  while (usageQuery.next())
  {
    std::chrono::seconds logon(usageQuery.value(0).toInt());
    std::chrono::seconds idle(usageQuery.value(1).toInt());
    std::chrono::seconds logoff(usageQuery.value(2).toInt());
    std::chrono::seconds rest(usageQuery.value(3).toInt());
    std::chrono::seconds timeLeft(usageQuery.value(4).toInt());
    daylyLoggedOnTime_ = logon;
    daylyLoggedOffTime_ = logoff;
    daylyIdlingTime_ = idle;
    timeUserHaveToRest_ = rest;
    timeLeftToLock_ = timeLeft;
    if(restControlEnabled_ && timeLeftToLock_ <= 0s && timeUserHaveToRest_ > 0s)
      {
        lockSystem();
      }
    qDebug("Loaded: logon %lld idle %lld logoff %lld haveToRest %lld timeLeftToLock_ %lld", logon.count(), idle.count(), logoff.count(), rest.count(), timeLeftToLock_.count());
    logonTimer->set(logon);
    std::chrono::seconds activity(usageQuery.value(0).toInt() - usageQuery.value(1).toInt());
    activityTimer->set(activity);
    break;
  }
}

void TimeTracker::loadUserSettings()
{
  QSettings settings("settings.ini", QSettings::IniFormat);
  if (QSettings::NoError != settings.status())
  {
    return;
  }
  settings.beginGroup("MainWindow");
  if (settings.contains("dontWarnOnHide"))
  {
    dontWarnOnHide_ = settings.value("dontWarnOnHide").toBool();
  }
  if (settings.contains("startMinimized"))
  {
    startHiddenInTray_ = settings.value("startMinimized").toBool();
  }

  settings.endGroup();
  settings.beginGroup("RestControl");
  if (settings.contains("maxLogOnTime"))
  {
    maxWorkTimeInRow_ = std::chrono::seconds(settings.value("maxLogOnTime").toUInt());
  }
  if (settings.contains("restTime"))
  {
    timerToRest_ = std::chrono::seconds(settings.value("restTime").toUInt());
  }
  if(maxWorkTimeInRow_ > 0s && timerToRest_ > 0s)
    {
      qDebug("Rest control enabled");
      restControlEnabled_ = true;
      timeLeftToLock_ = maxWorkTimeInRow_;
    }
  settings.endGroup();  
}
