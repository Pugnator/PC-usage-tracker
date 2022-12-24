#include "timetracker.hpp"
#include "./ui_timetracker.h"

// QProcess::startDetached("shutdown -s -f -t 00");
#include <QMenu>
#include <QMainWindow>

void TimeTracker::createTrayIcon()
{
  trayIcon = new TrayIcon(this);
  connect(trayIcon, SIGNAL(toggleVisibility()), this, SLOT(resizeEvent()));
  connect(this, SIGNAL(showTrayMessage(QString)), trayIcon, SLOT(showTrayMessage(QString)));  
}

TrayIcon::TrayIcon(QObject *parent) :
  QSystemTrayIcon(parent),
  messageShown(false)
{
  setIcon(QIcon("icon.ico"));
  setToolTip("TimeTracker");
  installEventFilter(this);

  connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(handleActivation(QSystemTrayIcon::ActivationReason)));
  trayIconMenu = new QMenu("TimeTracker");
  trayIconMenu->addAction(tr("Restore"), this, SLOT(setVisible()));
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(tr("Quit"), parent, &QCoreApplication::quit, Qt::QueuedConnection);
  setContextMenu(trayIconMenu);
  show();
}

TrayIcon::~TrayIcon()
{
}

void TrayIcon::setVisible()
{
  emit toggleVisibility();
}

void TrayIcon::showTrayMessage(QString message)
{  
  showMessage("Warning", message, QSystemTrayIcon::Warning);
}

void TrayIcon::handleActivation(QSystemTrayIcon::ActivationReason reason)
{
  switch (reason)
  {
  case TrayIcon::Context:
  {
    break;
  };

  case TrayIcon::MiddleClick:
  {
    break;
  };

  case TrayIcon::Trigger:
  {
    break;
  }

  case TrayIcon::DoubleClick:
  {
    emit toggleVisibility();
    break;
  };

  default:
  {
    break;
  };
  };
}
