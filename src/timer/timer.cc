#include "timer.hpp"
#include <QDebug>
#include <QTimer>
#include <QTime>
#include "Windows.h"
#include "Wtsapi32.h"
#include <QAbstractEventDispatcher>

using namespace std::chrono_literals;

Timer::Timer(QObject *thingy, QObject *parent, HWND win)
{
  timeTracker = thingy;
  parentWin = win;
  workerThread.reset(new QThread(parent));
  connect(workerThread.get(), SIGNAL(started()), this, SLOT(trackActivity()));
  connect(workerThread.get(), SIGNAL(finished()), this, SLOT(stopTimer()));
  this->moveToThread(workerThread.get());
  workerThread->start();
}

Timer::~Timer()
{
  workerThread->requestInterruption();
  workerThread->quit();
  workerThread->wait();
}

void Timer::timerEvent(QTimerEvent *event)
{
  HWND foreground = GetForegroundWindow();
  if (!foreground || parentWin == foreground)
  {
    return;
  }

  int len = GetWindowTextLength(foreground) + 1;
  std::vector<wchar_t> buf(len);
  GetWindowTextW(foreground, &buf[0], len);
  auto title = QString::fromWCharArray(std::accumulate(buf.begin(), buf.end(), std::wstring{}).c_str());
  emit appUpdate(title, 1s);
}

void Timer::trackActivity()
{
  timerId = startTimer(LOG_INTERVAL, Qt::PreciseTimer);
}

void Timer::stopTimer()
{
  killTimer(timerId);
}
