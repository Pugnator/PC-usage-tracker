#include "timetracker.hpp"
#include "./ui_timetracker.h"

// QProcess::startDetached("shutdown -s -f -t 00");
#include <QMenu>
#include <QMainWindow>

void TimeTracker::createTrayIcon()
{
  trayIcon = new TrayIcon(this);
  connect(trayIcon, SIGNAL(toggleVisibility()), this, SLOT(resizeEvent()));
}

TrayIcon::TrayIcon(QObject *parent) : QSystemTrayIcon(parent)
{
  setIcon(QIcon("icon.ico"));
  setToolTip("TimeTracker");
  installEventFilter(this);

  connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(handleActivation(QSystemTrayIcon::ActivationReason)));
  trayIconMenu = new QMenu("TimeTracker");
  trayIconMenu->addAction(tr("Restore"), this, SLOT(setVisible()));
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(tr("Quit"), this, &QCoreApplication::quit);
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
