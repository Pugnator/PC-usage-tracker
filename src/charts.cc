#include "timetracker.hpp"
#include "ui_timetracker.h"
#include <QPieSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QLegend>
#include <QBarCategoryAxis>
#include <QHorizontalPercentBarSeries>
#include <QValueAxis>

#include <QPieSlice>
#include <QLegend>
#include <QCryptographicHash>

void TimeTracker::createDailyChart()
{
  QLayoutItem *item;
  while ((item = ui->statsLayout->layout()->takeAt(0)) != NULL)
  {
    delete item->widget();
    delete item;
  }
  appDailyChart = new QChart();
  QSqlQuery usageQuery;
  QString queryStr = QString(
                         "SELECT name, usage FROM %1 WHERE last >= DATE('now', 'localtime') AND last < DATE('now', 'localtime', '+1 day') ORDER BY usage DESC LIMIT 10")
                         .arg(showWorkShiftStatsOnly ? "ApplicationWorktimeUsage" : "ApplicationUsage");
  usageQuery.prepare(queryStr);
  if (!usageQuery.exec())
  {
    return;
  }

  QPieSeries *series = new QPieSeries();
  while (usageQuery.next())
  {
    series->append(usageQuery.value(0).toString(), usageQuery.value(1).toInt());
  }

  series->setHoleSize(0.3);
  series->setLabelsVisible(false);
  if (!series->slices().empty())
  {
    for (const auto &s : series->slices())
    {
      std::hash<std::string> hasher;
      auto hashed = hasher(s->label().toStdString());
      QColor color(hashed & 0xFF, (hashed >> 8) & 0xFF, (hashed >> 16) & 0xFF);
      s->setPen(QPen(color, 2));
      s->setBrush(color);
      connect(s, SIGNAL(hovered(bool)), this, SLOT(onPieChartSliceHovered(bool)));
    }
  }
  appDailyChart->addSeries(series);
  appDailyChart->setBackgroundRoundness(0);
  appDailyChart->legend()->setVisible(true);
  appDailyChart->legend()->detachFromChart();
  appDailyChart->legend()->setBackgroundVisible(false);
  appDailyChart->legend()->setBrush(QBrush(QColor(128, 128, 128, 128)));
  appDailyChart->legend()->setPen(QPen(QColor(192, 192, 192, 192)));
  appDailyChart->legend()->setGeometry(QRectF(20, 400, 350, 450));

  appDailyChart->setContentsMargins(-10, -10, -10, -10);
  appDailyChart->legend()->update();
  QChartView *chartView = new QChartView(appDailyChart);
  chartView->setRenderHint(QPainter::Antialiasing);
  ui->statsLayout->addWidget(chartView);
}

void TimeTracker::onPieChartSliceHovered(bool hovered)
{
  QObject *obj = sender();
  QPieSlice *slice = qobject_cast<QPieSlice *>(obj);
  slice->setExploded(hovered);
}

void TimeTracker::createCalendChart(QDate date)
{
  QLayoutItem *item;
  while ((item = ui->statsLayoutCalend->layout()->takeAt(0)) != NULL)
  {
    delete item->widget();
    delete item;
  }
  QChart *calendChart = new QChart();
  QSqlQuery usageQuery;
  QString queryStr = QString(
                         "SELECT name, usage FROM %1 WHERE last >= DATE('%2') AND last < DATE('%2', '+1 day') ORDER BY usage DESC LIMIT 10")
                         .arg(showWorkShiftStatsOnly ? "ApplicationWorktimeUsage" : "ApplicationUsage", date.toString("yyyy-MM-dd"));
  usageQuery.prepare(queryStr);
  if (!usageQuery.exec())
  {
    return;
  }

  QPieSeries *series = new QPieSeries();
  while (usageQuery.next())
  {
    series->append(usageQuery.value(0).toString(), usageQuery.value(1).toInt());
  }

  series->setHoleSize(0.3);
  series->setLabelsVisible(false);
  if (!series->slices().empty())
  {
    for (const auto &s : series->slices())
    {
      std::hash<std::string> hasher;
      auto hashed = hasher(s->label().toStdString());
      QColor color(hashed & 0xFF, (hashed >> 8) & 0xFF, (hashed >> 16) & 0xFF);
      s->setPen(QPen(color, 2));
      s->setBrush(color);
      connect(s, SIGNAL(hovered(bool)), this, SLOT(onPieChartSliceHovered(bool)));
    }
  }

  calendChart->addSeries(series);
  calendChart->setBackgroundRoundness(0);
  calendChart->legend()->setVisible(true);
  calendChart->legend()->detachFromChart();
  calendChart->legend()->setBackgroundVisible(false);
  calendChart->legend()->setBrush(QBrush(QColor(128, 128, 128, 128)));
  calendChart->legend()->setPen(QPen(QColor(192, 192, 192, 192)));
  calendChart->legend()->setGeometry(QRectF(20, 400, 350, 450));

  calendChart->setContentsMargins(-10, -10, -10, -10);
  calendChart->legend()->update();
  QChartView *chartView = new QChartView(calendChart);
  chartView->setRenderHint(QPainter::Antialiasing);
  ui->statsLayoutCalend->addWidget(chartView);
}

void TimeTracker::createAppChart(QDate date)
{

  QSqlQuery usageQuery;
  QString queryStr = QString(
                         "SELECT name, usage FROM ApplicationUsage WHERE last >= DATE('%2', '-7 days') AND last < DATE('%2', '+1 day') ORDER BY usage DESC LIMIT 10")
                         .arg(date.toString("yyyy-MM-dd"));
  usageQuery.prepare(queryStr);
  if (!usageQuery.exec())
  {
    return;
  }

  QHorizontalPercentBarSeries *series = new QHorizontalPercentBarSeries();
  while (usageQuery.next())
  {
    auto *set = new QBarSet(usageQuery.value(0).toString());
    *set << usageQuery.value(1).toInt();
    series->append(set);
  }
  QChart *appChart = new QChart();
  appChart->setTitle("Weekly stats");
  appChart->setAnimationOptions(QChart::SeriesAnimations);
  appChart->addSeries(series);
  QStringList categories;
  categories << "Jan"
             << "Feb"
             << "Mar"
             << "Apr"
             << "May"
             << "Jun";
  QBarCategoryAxis *axisY = new QBarCategoryAxis();
  axisY->append(categories);
  appChart->addAxis(axisY, Qt::AlignLeft);
  series->attachAxis(axisY);
  QValueAxis *axisX = new QValueAxis();
  appChart->addAxis(axisX, Qt::AlignBottom);
  series->attachAxis(axisX);
  appChart->legend()->setVisible(true);
  appChart->legend()->setAlignment(Qt::AlignBottom);
  QChartView *chartView = new QChartView(appChart);
  chartView->setRenderHint(QPainter::Antialiasing);
  ui->appWeekChartLayout->addWidget(chartView);
}

void TimeTracker::on_calendarStats_selectionChanged()
{
  QDate date = ui->calendarStats->selectedDate();
  createCalendChart(date);
}
