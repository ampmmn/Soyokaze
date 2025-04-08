#pragma once

namespace launcherapp { namespace commands { namespace env {

class AppSettings
{
public:
	void Load();

	CString mPrefix{_T("env")};
	bool mIsEnable = true;
};

}}} // end of namespace launcherapp::commands::env

