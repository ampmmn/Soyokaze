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

protected:
	CString mIniFilePath;
	FILETIME mLastUpdateFt{0,0};

};

}}} // end of namespace launcherapp::commands::winscp

