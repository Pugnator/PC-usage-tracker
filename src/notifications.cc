#include "timetracker.hpp"
#include <QMessageBox>
#include "Wtsapi32.h"
#include "Winuser.h"
#include <string.h>

void TimeTracker::showNativeMessage(DWORD userSession, QString message)
{
  DWORD result;
  QString title = "Warning";
  LPWSTR title_ = const_cast<wchar_t *>(title.toStdWString().c_str());
  LPWSTR message_ = const_cast<wchar_t *>(message.toStdWString().c_str());
  if(!WTSSendMessage(WTS_CURRENT_SERVER_HANDLE, userSession,
                 title_, wcslen (title_)*2+1,
                 message_, wcslen (message_)*2+1,
                 MB_OK, 3 , &result,TRUE))
    {

    }
}

