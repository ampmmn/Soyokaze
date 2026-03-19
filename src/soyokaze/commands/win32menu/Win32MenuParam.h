#pragma once

#include "setting/Settings.h"
#include <map>

namespace launcherapp { namespace commands { namespace win32menu {

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	// 画面構成の都合上、UIAutomationと設定画面を共有する。その関係で、uiautomation::CommandParam側で設定値を保存するのでこちらはSaveを実装しない
	//bool Save(Settings& settings) const;
	bool Load(Settings& settings);

public:
	// 機能を利用するか?
	bool mIsEnable{false};
	// 全てのウインドウのメニューを検索するか?
	bool mIsFindAllMenu{false};
	// プレフィックス
	CString mPrefix;
};


}}} // end of namespace launcherapp::commands::win32menu

