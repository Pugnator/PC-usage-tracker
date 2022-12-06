#pragma once

#include <QLCDNumber>
#include <QTimer>

using namespace std::chrono_literals;

class DigitalClock : public QLCDNumber
{
  Q_OBJECT
public:
  DigitalClock(QWidget *parent = nullptr);

private slots:
    void showTime();
    void start();
    void stop();
    void reset();

};

class DigitalTimer : public QLCDNumber
{
  Q_OBJECT
public:
  DigitalTimer(std::chrono::seconds seed = 0s, QWidget *parent = nullptr);
  void start();
  void start(std::chrono::seconds time);
  void pause();
  void reset();
  void set(std::chrono::seconds time);

private slots:
    void showTime();

private:
    std::chrono::seconds timeElapsed;
    QTimer *timer;
    std::atomic_bool isRunning;
};
