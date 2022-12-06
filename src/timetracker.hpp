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
  void setVisible();
  void handleActivation(QSystemTrayIcon::ActivationReason reason);

signals:
  void toggleVisibility();

private:
  QMenu *trayIconMenu;
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
  bool prepareDb();
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

  std::unique_ptr<QObject> timeTracker;
  std::unique_ptr<Timer> trackerWorker;
  AppUsageView *appUsageModel;
  std::unique_ptr<QSqlDatabase> db;

  TrayIcon *trayIcon;
  QChart *appDailyChart;
  QChart *calendChart;
  DigitalTimer *logonTimer;
  DigitalTimer *activityTimer;
  int timerId;

  std::atomic_bool dontWarnOnHide;
  std::atomic_bool isSystemLocked;
  std::atomic_bool trackingEnabled;
  std::atomic_bool showWorkShiftStatsOnly;
  std::chrono::seconds loggedOnTime;
  std::chrono::seconds loggedOffTime;
  std::chrono::seconds userIdlingTime;

  QTime shiftStart;
  QTime shiftEnd;
  QDate currentSession;
};
