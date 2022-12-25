#include "timetracker.hpp"
#include "./ui_timetracker.h"

QStringList SQL_SCHEME =
    {
        "CREATE TABLE IF NOT EXISTS ApplicationUsage (ID INTEGER PRIMARY KEY AUTOINCREMENT, day DATETIME DEFAULT CURRENT_DATE, name TEXT, usage INTEGER, last DATETIME DEFAULT CURRENT_TIMESTAMP)",
        "CREATE TABLE IF NOT EXISTS ApplicationWorktimeUsage (ID INTEGER PRIMARY KEY AUTOINCREMENT, day DATETIME DEFAULT CURRENT_DATE, name TEXT, usage INTEGER, last DATETIME DEFAULT CURRENT_TIMESTAMP)",
        "CREATE TABLE IF NOT EXISTS ApplicationFilter (ID INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE)",
        "CREATE TABLE IF NOT EXISTS DailyUsage (ID INTEGER PRIMARY KEY AUTOINCREMENT, day DATETIME DEFAULT CURRENT_DATE UNIQUE, logon INTEGER, logoff INTEGER, idle INTEGER, timeLeftToLock INTEGER, haveToRest INTEGER)",
        "CREATE TABLE IF NOT EXISTS DailyRest (ID INTEGER PRIMARY KEY AUTOINCREMENT, day DATETIME DEFAULT CURRENT_DATE UNIQUE, restTime INTEGER)"
};

#define APP_VIEW_SHOW_LIMIT " LIMIT 15"

void TimeTracker::appModelSetup(QString tableName)
{
  appUsageModel->setTable(tableName);
  appUsageModel->setFilter("day=DATE('now', 'localtime')");
  appUsageModel->setHeaderData(2, Qt::Orientation::Horizontal, "Applications");
  appUsageModel->setHeaderData(3, Qt::Orientation::Horizontal, "Using time");
  appUsageModel->setSort(3, Qt::DescendingOrder);
  ui->appTableView->setSortingEnabled(false);
  ui->appTableView->setColumnHidden(0, true);
  ui->appTableView->setColumnHidden(1, true);
  ui->appTableView->setColumnHidden(4, true);
  ui->appTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  appUsageModel->select();
}

bool TimeTracker::prepareDb()
{
  db = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE"));
  db->setDatabaseName(DATABASE_FILENAME);
  if (!db->open())
  {
    qInfo("Failed to open database");
    return false;
  }
  QSqlQuery query(*db.get());
  // It's safe to ignore errors here.
  query.exec(SQL_OPTIONS);
  query.finish();
  foreach (QString queryTxt, SQL_SCHEME)
  {
    query.exec(queryTxt);
    query.finish();
  }

  appUsageModel = new AppUsageView();
  ui->appTableView->setModel(appUsageModel);
  appModelSetup("ApplicationUsage");
  return true;
}

QString AppUsageView::selectStatement() const
{
  QString query = QSqlTableModel::selectStatement();
  query += APP_VIEW_SHOW_LIMIT;
  return query;
}

QVariant AppUsageView::data(const QModelIndex &index, int role) const
{
  auto value = QSqlTableModel::data(index, role);
  if (role == Qt::DisplayRole && index.column() == 3)
  {
    std::chrono::seconds seconds(value.toUInt());
    QString prettySeconds = beautifyDuration(seconds);
    return QVariant(prettySeconds);
  }
  return value;
}

QString AppUsageView::beautifyDuration(std::chrono::seconds input_seconds) const
{
  using namespace std::chrono;
  typedef duration<int, std::ratio<86400>> days;
  auto d = duration_cast<days>(input_seconds);
  input_seconds -= d;
  auto h = duration_cast<hours>(input_seconds);
  input_seconds -= h;
  auto m = duration_cast<minutes>(input_seconds);
  input_seconds -= m;
  auto s = duration_cast<seconds>(input_seconds);

  auto dc = d.count();
  auto hc = h.count();
  auto mc = m.count();
  auto sc = s.count();

  std::stringstream ss;
  ss.fill('0');
  if (dc)
  {
    ss << d.count() << "d ";
  }
  if (dc || hc)
  {
    if (dc)
    {
      ss << std::setw(2);
    } // pad if second set of numbers
    ss << h.count() << "h ";
  }
  if (dc || hc || mc)
  {
    if (dc || hc)
    {
      ss << std::setw(2);
    }
    ss << m.count() << "min ";
  }
  if (dc || hc || mc || sc)
  {
    if (dc || hc || mc)
    {
      ss << std::setw(2);
    }
    ss << s.count() << "sec";
  }

  return QString::fromStdString(ss.str());
}

void TimeTracker::on_resetStatsButton_clicked()
{
  appUsageModel->select();
  QSqlQuery clearQuery;
  clearQuery.prepare("DELETE FROM ApplicationUsage");
  clearQuery.exec();
  clearQuery.finish();
  clearQuery.prepare("DELETE FROM ApplicationWorktimeUsage");
  clearQuery.exec();
  appUsageModel->submitAll();
}


