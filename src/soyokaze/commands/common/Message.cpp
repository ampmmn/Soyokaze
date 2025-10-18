#include "pch.h"
#include "Message.h"
#include "app/LauncherApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {

void PopupMessage(const CString& msg)
{
	auto app = (LauncherApp*)AfxGetApp();
	app->PopupMessage(msg);
}

void PopupMessage(const String& msg)
{
	auto app = (LauncherApp*)AfxGetApp();

	CString tmpStr;
	app->PopupMessage(UTF2UTF(msg, tmpStr));
}

}
}
}

