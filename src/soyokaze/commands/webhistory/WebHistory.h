#pragma once
namespace launcherapp {
namespace commands {
namespace webhistory {

struct HISTORY
{
public:
	CString mBrowserName;
	CString mDisplayName;
	CString mUrl;
	int mMatchLevel{0};
};


} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp



