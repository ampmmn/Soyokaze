#include "pch.h"
#include "Message.h"
#include "Soyokaze.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace common {

void PopupMessage(const CString& msg)
{
	auto app = (CSoyokazeApp*)AfxGetApp();
	app->PopupMessage(msg);
}

}
}
}

