#include "timetracker.hpp"
#include "./ui_timetracker.h"
#include "Windows.h"
#include "Wtsapi32.h"
#include <QMessageBox>

void TimeTracker::closeEvent(QCloseEvent *event)
{  
  if (!event->spontaneous() || !isVisible())
    return;

  if (trayIcon->isVisible())
  {
    if (!dontWarnOnHide_)
    {
      QMessageBox msgBox;
      msgBox.setWindowIcon(QIcon("icon.ico"));
      msgBox.setText(tr("The application will continue to work in backgroud.<br>To close the app click \"Quit\" in the system tray's icon menu."));
      msgBox.setIcon(QMessageBox::Icon::Warning);
      msgBox.addButton(QMessageBox::Ok);
      QCheckBox *cb = new QCheckBox("Don't show again");
      msgBox.setCheckBox(cb);
      QObject::connect(cb, &QCheckBox::stateChanged, [this](int state)
                       {
              if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
                  this->dontWarnOnHide_ = true;
                } });
      msgBox.exec();
    }
    hide();
    event->ignore();
  }
}

bool TimeTracker::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
  Q_UNUSED(result);
  Q_UNUSED(eventType);
  MSG *msg = reinterpret_cast<MSG *>(message);
  switch (msg->message)
  {
  case WM_WTSSESSION_CHANGE:
    if (WTS_SESSION_LOCK == msg->wParam)
    {
      isSystemLocked_ = true;
      logonTimer->pause();
      activityTimer->pause();
    }
    if (WTS_SESSION_UNLOCK == msg->wParam)
    {
        if(isHaveToRest_)
          {
            qDebug("You have to rest!");
            if(LockWorkStation())
              {
                qDebug("Locked the workstation");
                return false;
              }
          }
      isSystemLocked_ = false;
      logonTimer->start();
      activityTimer->start();
    }
    break;
  }
  return false;
}

void TimeTracker::resizeEvent()
{
  QMainWindow::setVisible(true);
}

void TimeTracker::timerEvent(QTimerEvent *event)
{
  createDailyChart();
}

void TimeTracker::on_showWorkTimeRangeOnly_clicked(bool clicked)
{
  showWorkShiftStatsOnly_ = clicked;
  createDailyChart();
  appModelSetup(showWorkShiftStatsOnly_ ? "ApplicationWorktimeUsage" : "ApplicationUsage");
}

void TimeTracker::on_stopTrackingButton_clicked(bool clicked)
{
  trackingEnabled_ = !trackingEnabled_;
  if (trackingEnabled_)
  {
    ui->stopTrackingButton->setText(tr("Stop tracking"));
  }
  else
  {
    ui->stopTrackingButton->setText(tr("Start tracking"));
  }
}

bool TimeTracker::eventFilter(QObject *obj, QEvent *event)
{
  qDebug("event!");
  if (obj == trayIcon)
  {
    if (event->type() == QEvent::MouseButtonDblClick)
    {
      qDebug("Bingo!");
      return true;
    }
    else
    {
      qDebug("Something else!");
      return false;
    }
  }
  else
  {

    return QMainWindow::eventFilter(obj, event);
  }
}
