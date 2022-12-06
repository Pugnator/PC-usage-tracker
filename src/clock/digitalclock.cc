#include "digitalclock.hpp"
#include <QTime>
#include <QColor>

DigitalClock::DigitalClock(QWidget *parent)
  : QLCDNumber(parent)
{  
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &DigitalClock::showTime);
  timer->start(1000);

  showTime();
  resize(150, 60);
}

void DigitalClock::showTime()
{
  QTime time = QTime::currentTime();
  QString text = time.toString("hh:mm");
  if ((time.second() % 2) == 0)
    text[2] = ' ';
  display(text);
}

void DigitalClock::start()
{

}
void DigitalClock::stop()
{

}
void DigitalClock::reset()
{

}


DigitalTimer::DigitalTimer(std::chrono::seconds seed, QWidget *parent)
  : QLCDNumber(parent)
{
  timeElapsed = seed;
  isRunning = false;
  display("00 00");
  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &DigitalTimer::showTime);
  timer->start(1000);
  showTime();
  resize(150, 60);
}

void DigitalTimer::showTime()
{
  if(!isRunning)
    {
      return;
    }
  timeElapsed++;
  QTime _seed(0, 0);
  QTime time = _seed.addSecs(timeElapsed.count());
  QString text = time.toString("hh:mm");
  if ((timeElapsed.count() % 2) == 0)
    text[2] = ' ';
  display(text);
}

void DigitalTimer::start(std::chrono::seconds time)
{
  if(isRunning)
    {
      return;
    }
  isRunning = true;
  timeElapsed = time;
  auto pal = palette();
  pal.setColor(pal.WindowText, QColor(0, 0, 0));
  setPalette(pal);
}

void DigitalTimer::start()
{
  isRunning = true;
  auto pal = palette();
  pal.setColor(pal.WindowText, QColor(0, 0, 0));
  setPalette(pal);
}

void DigitalTimer::pause()
{
  isRunning = false;
  auto pal = palette();
  pal.setColor(pal.WindowText, QColor(204, 12, 12));
  setPalette(pal);
}
void DigitalTimer::reset()
{
  timeElapsed = 0s;  
}
void DigitalTimer::set(std::chrono::seconds time)
{
  timeElapsed = time;
}

