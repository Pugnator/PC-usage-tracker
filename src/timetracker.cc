#include "timetracker.hpp"
#include "./ui_timetracker.h"
#include "timer/timer.hpp"
#include "Wtsapi32.h"
#include "clock/digitalclock.hpp"

TimeTracker::TimeTracker(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::TimeTracker)
  , isSystemLocked(false),
    loggedOnTime(0s),
    loggedOffTime(0s),
    userIdlingTime(0s),
    showWorkShiftStatsOnly(false),
    trackingEnabled(true),
    dontWarnOnHide(false),
    currentSession(QDate::currentDate())
{
  ui->setupUi(this);
  QSettings settings ("settings.ini", QSettings::IniFormat);
  if(QSettings::NoError != settings.status())
    {
      return;
    }
  settings.beginGroup("MainWindow");
  if(settings.contains("dontWarnOnHide"))
    {
      dontWarnOnHide = settings.value("dontWarnOnHide").toBool();
    }
  settings.endGroup();


  if(!WTSRegisterSessionNotification(reinterpret_cast<HWND>(winId()), NOTIFY_FOR_THIS_SESSION))
    {
      qDebug("Failed to register for notifications");
    }

  if(!prepareDb())
    {
      qDebug("failed to prepare DB");
      QCoreApplication::exit(1);
      return;
    }

  createTrayIcon();
  ui->appTableView->show();

  timeTracker.reset(new QObject(this));
  trackerWorker.reset(new Timer(timeTracker.get(), this, reinterpret_cast<HWND>(winId())));
  connect(trackerWorker.get(), SIGNAL(appUpdate(QString, std::chrono::seconds)), this, SLOT(updateApplicationUsage(QString, std::chrono::seconds)));

  setWindowIcon(QIcon("icon.ico"));

  logonTimer = new DigitalTimer();
  logonTimer->start();
  activityTimer = new DigitalTimer();
  activityTimer->start();
  ui->logonTimerLayout->addWidget(logonTimer);
  ui->activityTimerLayout->addWidget(activityTimer);
  createDailyChart();
  createCalendChart(QDate::currentDate());
  createAppChart(QDate::currentDate());
  timerId = startTimer(15000, Qt::VeryCoarseTimer);
  loadSettings();

  show();
}

TimeTracker::~TimeTracker()
{  
  QSettings settings ("settings.ini", QSettings::IniFormat);
  if(QSettings::NoError != settings.status())
    {
      return;
    }
  settings.beginGroup("MainWindow");
  settings.setValue("dontWarnOnHide", QVariant(dontWarnOnHide));
  settings.endGroup();

  updateDailyStats();
  killTimer(timerId);
  WTSUnRegisterSessionNotification(reinterpret_cast<HWND>(winId()));
  db->close();
  delete ui;
}


