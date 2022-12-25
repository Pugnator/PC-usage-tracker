#include "timetracker.hpp"
#include "./ui_timetracker.h"
#include "timer/timer.hpp"
#include "Wtsapi32.h"
#include "clock/digitalclock.hpp"

TimeTracker::TimeTracker(QWidget *parent)
  : QMainWindow(parent),
    ui(new Ui::TimeTracker),
    isSystemLocked_(false),
    daylyLoggedOnTime_(0s),
    isHaveToRest_(false),
    restControlEnabled_(false),
    daylyLoggedOffTime_(0s),
    daylyIdlingTime_(0s),
    timeLeftToLock_(0s),
    maxWorkPerDayTime_(0s),
    startHiddenInTray_(false),
    timeEndWarningShown_(false),
    showWorkShiftStatsOnly_(false),
    trackingEnabled_(true),
    dontWarnOnHide_(false),
    currentSession_(QDate::currentDate())
{
  ui->setupUi(this);
  qInfo("Application started.");

  loadUserSettings();

  if (!WTSRegisterSessionNotification(reinterpret_cast<HWND>(winId()), NOTIFY_FOR_ALL_SESSIONS))
    {
      qCritical("Failed to register for notifications.");
    }

  if (!prepareDb())
    {
      qCritical("failed to prepare DB.");
      QCoreApplication::exit(1);
      return;
    }

  createTrayIcon();
  ui->appTableView->show();

  timeTracker_.reset(new QObject(this));
  trackerWorker.reset(new Timer(timeTracker_.get(), this, reinterpret_cast<HWND>(winId())));
  connect(trackerWorker.get(), SIGNAL(appUpdate(QString, std::chrono::seconds)), this, SLOT(onTimerTick(QString, std::chrono::seconds)));

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
  updateDailyStats();
  if(!startHiddenInTray_)
    {
      show();
    }
}

TimeTracker::~TimeTracker()
{
  qInfo("Exiting.");
  QSettings settings("settings.ini", QSettings::IniFormat);
  if (QSettings::NoError != settings.status())
    {
      qCritical("Error saving user settings.");
      return;
    }
  settings.beginGroup("Global");
  settings.setValue("dontWarnOnHide", QVariant(dontWarnOnHide_));
  settings.endGroup();

  updateDailyStats();
  killTimer(timerId);
  WTSUnRegisterSessionNotification(reinterpret_cast<HWND>(winId()));
  db->close();
  delete ui;
}
