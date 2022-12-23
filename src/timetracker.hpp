#pragma once

#include <QMainWindow>
#include <QtSql>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QDialog>
#include <QChartView>
#include "clock/digitalclock.hpp"
#include "timer/timer.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
  class TimeTracker;
}
QT_END_NAMESPACE

using namespace std::chrono_literals;

inline constexpr const char *const DATABASE_FILENAME = "usage.db3";
inline constexpr const char *const DATABASE_BACKUP_FILENAME = "usage.db3.bak";
inline constexpr const char *const SQL_OPTIONS =
    "PRAGMA main.page_size = 4096;\
    PRAGMA main.cache_size=10000;\
PRAGMA main.synchronous=OFF;\
PRAGMA foreign_keys = ON;\
PRAGMA main.journal_mode=WAL;\
PRAGMA wal_autocheckpoint=1000;\
PRAGMA main.temp_store=MEMORY;";

class AppUsageView : public QSqlTableModel
{
public:
  AppUsageView() : QSqlTableModel(){};
  ~AppUsageView(){};

private:
  QString selectStatement() const Q_DECL_OVERRIDE;
  QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
  QString beautifyDuration(std::chrono::seconds input_seconds) const;
};

class TrayIcon : public QSystemTrayIcon
{
  Q_OBJECT

public:
  TrayIcon(QObject *parent);
  ~TrayIcon();

private slots:
  void showTrayMessage(QString);
  void deleteTrayMessage();
  void setVisible();  
  void handleActivation(QSystemTrayIcon::ActivationReason reason);

signals:  
  void toggleVisibility();

private:
  QMenu *trayIconMenu;
  bool messageShown;
};

class TimeTracker : public QMainWindow
{
  Q_OBJECT

public:
  TimeTracker(QWidget *parent = nullptr);
  ~TimeTracker();

protected:
  void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
  bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

signals:
  void showTrayMessage(QString);
  void deleteTrayMessage();

private slots:  
  void updateApplicationUsage(QString, std::chrono::seconds);
  void updateApplicationAtWorkUsage(QString name, std::chrono::seconds time);

  void resizeEvent();

  void onPieChartSliceHovered(bool);
  void on_showWorkTimeRangeOnly_clicked(bool);
  void on_stopTrackingButton_clicked(bool);  
  void on_calendarStats_selectionChanged();

  void on_resetStatsButton_clicked();
  void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;
  bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) Q_DECL_OVERRIDE;

private:
  void setRestTimer();
  void getRestTimer();
  void doSomeRestNotification();
  bool prepareDb();
  void loadUserSettings();
  void appModelSetup(QString tableName);
  void createTrayIcon();
  void createActions();
  void updateDailyStats();
  void loadSettings();
  void createDailyChart();
  void createCalendChart(QDate date);
  void createAppChart(QDate date);

  QString selectStatement() const;

  void error(QString text);

  Ui::TimeTracker *ui;

  std::unique_ptr<QObject> timeTracker_;
  std::unique_ptr<Timer> trackerWorker;
  AppUsageView *appUsageModel;
  std::unique_ptr<QSqlDatabase> db;

  TrayIcon *trayIcon;
  QChart *appDailyChart;
  QChart *calendChart;
  DigitalTimer *logonTimer;
  DigitalTimer *activityTimer;
  int timerId;

  std::atomic_bool dontWarnOnHide_;
  std::chrono::seconds maxWorkTimeInRow_;
  std::chrono::seconds timerToRest_;
  std::chrono::seconds haveToRest_;

  std::atomic_bool isHaveToRest_;
  std::atomic_bool restControlEnabled_;
  std::atomic_bool isSystemLocked_;
  std::atomic_bool trackingEnabled_;
  std::atomic_bool showWorkShiftStatsOnly_;
  std::chrono::seconds loggedOnTime_;
  std::chrono::seconds loggedOffTime_;
  std::chrono::seconds userIdlingTime_;

  std::chrono::seconds userWorkTime_;

  QTime shiftStart_;
  QTime shiftEnd_;
  QDate currentSession_;
};
