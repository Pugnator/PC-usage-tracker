#pragma once

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QThread>

#include <winternl.h>

#include <chrono>

#define LOG_INTERVAL 1000

class Timer : public QObject
{
  Q_OBJECT
public:
  Timer(QObject *thingy, QObject *parent, HWND win);
  ~Timer();

private:
  int timerId;
  QObject *timeTracker;
  HWND parentWin;
  std::unique_ptr<QThread> workerThread;

private slots:
  void timerEvent(QTimerEvent *event);
  void stopTimer();
  void trackActivity();

signals:
  void appUpdate(QString, std::chrono::seconds);
};
