#pragma once

#include "setting/Settings.h"
#include <map>

namespace launcherapp { namespace commands { namespace uiautomation {

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	bool Save(Settings& settings) const;
	bool Load(Settings& settings);

public:
	// 機能を利用するか?
	bool mIsEnable{false};
	// プレフィックス
	CString mPrefix;
	// メニュー検索を使うか
	bool mIsEnableMenuItem{false};
	// タブページ検索を使うか(非アクティブなタブのUI要素検索)
	bool mIsEnableTabPages{false};
	// デバッグ出力コマンドを有効にする
	bool mIsDebugDumpEnabled{false};
};


}}} // end of namespace launcherapp::commands::uiautomation

