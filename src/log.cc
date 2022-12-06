#include "timetracker.hpp"
#include "./ui_timetracker.h"
#include <QMessageBox>

void TimeTracker::error(QString text)
{
  QMessageBox::critical(this, "Ошибка", text);
}
