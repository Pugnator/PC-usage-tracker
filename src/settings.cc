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
  usageQuery.prepare("SELECT logon, idle, logoff, haveToRest FROM DailyUsage WHERE day=DATE('now', 'localtime') LIMIT 1");
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
    loggedOnTime_ = logon;
    loggedOffTime_ = logoff;
    userIdlingTime_ = idle;
    haveToRest_ = rest;
    if(haveToRest_ > 0s)
      {
        isHaveToRest_ = true;
        qDebug("have to rest!");
        if(LockWorkStation())
          {
            qDebug("Locked the workstation");
          }
      }
    qDebug("Loaded: logon %lld idle %lld logoff %lld haveToRest_ %lld", logon.count(), idle.count(), logoff.count(), rest.count());
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
      restControlEnabled_ = true;
    }
  settings.endGroup();  
}
