#pragma once

namespace launcherapp { namespace externaltool { namespace webbrowser {

class BrowserProfile
{
public:
	// Local Stateから最後に使用したプロファイル名を取得する
	static CString GetLastUsedProfileName(LPCTSTR userDataPath);
};

}}}
