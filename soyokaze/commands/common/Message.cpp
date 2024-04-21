#include "pch.h"
#include "Message.h"
#include "app/LauncherApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace common {

void PopupMessage(const CString& msg)
{
	auto app = (LauncherApp*)AfxGetApp();
	app->PopupMessage(msg);
}

}
}
}

