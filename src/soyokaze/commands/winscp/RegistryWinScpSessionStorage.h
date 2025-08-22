#pragma once

#include "commands/winscp/WinScpSessionStorage.h"

namespace launcherapp { namespace commands { namespace winscp {

// $B%l%8%9%H%j$+$i$N@_Dj>pJsFI$_9~$_(B
class RegistrySessionStorage : public SessionStrage
{
public:
	RegistrySessionStorage();
	~RegistrySessionStorage();

	bool HasUpdate() override;
	bool LoadSessions(std::vector<CString>& sessionNames) override;

	bool OpenSubKeyIfNotOpened();
	bool RegisterNotifyChangeKeyValue();

private:
	HKEY mSubKeyForWatch{nullptr};
	HANDLE mEventForNotify{nullptr};
};

}}} // end of namespace launcherapp::commands::winscp

