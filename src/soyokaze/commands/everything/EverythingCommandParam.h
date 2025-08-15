#pragma once

#include "setting/Settings.h"

namespace launcherapp { namespace commands { namespace everything {

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam&) = default;
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	bool Save(Settings& settings) const;
	bool Load(Settings& settings);

public:
	CString mPrefix;

	// 機能を利用するか?
	bool mIsEnable{false};
	// Everythingアプリが起動していない場合に起動するか?
	bool mIsRunApp{false};
	// コマンドライン経由で使用する場合のEverything.exeのパス
	CString mEverythingExePath;

};


}}} // end of namespace launcherapp::commands::everything
