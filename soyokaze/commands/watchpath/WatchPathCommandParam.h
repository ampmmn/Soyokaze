#pragma once

namespace launcherapp {
namespace commands {
namespace watchpath {

class CommandParam
{
public:
	CommandParam() : mNotifyMessage(_T("更新を検知"))
	{}

	CString mName;
	CString mDescription;
	CString mPath;
	CString mNotifyMessage;
	bool mIsDisabled = false;
};


}
}
}

