#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "setting/Settings.h"
#include <map>

namespace launcherapp { namespace commands { namespace winscp {

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	bool ResolveExecutablePath(CString& executableFilePath);

	bool Save(Settings& settings) const;
	bool Load(Settings& settings);

public:
	// 機能を利用するか?
	bool mIsEnable{true};
	// ポータブル版WinScpを利用するか?
	bool mIsUsePortable{false};
	// ポータブル版を利用する場合の実行ファイルのパス
	CString mWinScpExeFilePath;

	// 以下はキャッシュ
	CString mCachedWinExpFilePath;
};


}}} // end of namespace launcherapp::commands::winscp

