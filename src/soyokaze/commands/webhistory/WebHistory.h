#pragma once

#include "externaltool/webbrowser/BrowserEnvironment.h"

namespace launcherapp {
namespace commands {
namespace webhistory {

struct HISTORY
{
public:
	externaltool::webbrowser::BrowserEnvironment* mEnv{nullptr};
	CString mDisplayName;
	CString mUrl;
	int mMatchLevel{0};
};


} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp



