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
    timeUserHasToRest_ = rest;
    timeLeftToLock_ = timeLeft;
    if(restControlEnabled_ && timeLeftToLock_ <= 0s && timeUserHasToRest_ > 0s)
      {
        lockSystem();
      }
    qInfo("Dayly stats loaded:\nTotal logon %lld\nTotal idle %lld\nTotal logoff %lld\nTime to rest left %lld\nTime before lock left %lld", logon.count(), idle.count(), logoff.count(), rest.count(), timeLeftToLock_.count());
    logonTimer->set(logon);
    std::chrono::seconds activity(usageQuery.value(0).toInt() - usageQuery.value(1).toInt());
    activityTimer->set(activity);
    break;
  }
}

void TimeTracker::loadUserSettings()
{
  qInfo("Loading user defined settings");
  QSettings settings("settings.ini", QSettings::IniFormat);
  if (QSettings::NoError != settings.status())
  {
    qCritical("Error loading user settings.");
    return;
  }
  settings.beginGroup("Global");
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
  if (settings.contains("maxWorkInARowTime"))
  {
    maxWorkTimeInRow_ = std::chrono::seconds(settings.value("maxWorkInARowTime").toUInt());
    qInfo("maxWorkTimeInRow=%lld", maxWorkTimeInRow_.count());
  }
  if (settings.contains("maxWorkPerDayTime"))
  {
    maxWorkPerDayTime_ = std::chrono::seconds(settings.value("maxWorkPerDayTime").toUInt());
    qInfo("maxWorkPerDayTime=%lld", maxWorkPerDayTime_.count());
  }
  if (settings.contains("restTime"))
  {
    timerToRest_ = std::chrono::seconds(settings.value("restTime").toUInt());
    qInfo("timerToRest=%lld", timerToRest_.count());
  }
  if(maxWorkTimeInRow_ > 0s && timerToRest_ > 0s)
    {
      qInfo("Rest control enabled");
      restControlEnabled_ = true;
      timeLeftToLock_ = maxWorkTimeInRow_;
    }
  settings.endGroup();  
}
