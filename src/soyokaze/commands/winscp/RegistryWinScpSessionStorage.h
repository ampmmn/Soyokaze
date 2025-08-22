#pragma once

#include "commands/winscp/WinScpSessionStorage.h"

namespace launcherapp { namespace commands { namespace winscp {

// レジストリからの設定情報読み込み
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

