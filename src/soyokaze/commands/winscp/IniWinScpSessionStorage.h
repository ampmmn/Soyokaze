#pragma once

#include "commands/winscp/WinScpSessionStorage.h"

namespace launcherapp { namespace commands { namespace winscp {

// INIファイルからの設定情報読み込み
class IniSessionStorage : public SessionStrage
{
public:
	IniSessionStorage();
	~IniSessionStorage();

	bool HasUpdate() override;
	bool LoadSessions(std::vector<CString>& sessionNames) override;

	static bool Exists();

protected:
	static bool FindIniFilePath(CString& iniFilePath);
	static bool FindAutomaticIniFilePath(CString& iniFilePath);
	static bool FindCustomIniFilePath(CString& iniFilePath);

	static void OnReload(void* p);

protected:
	CString mIniFilePath;
	uint64_t mLastUpdate{0};
	uint32_t mListenerId{0};
	bool mHasUpdate{false};


};

}}} // end of namespace launcherapp::commands::winscp

