#pragma once

#include "setting/Settings.h"

namespace launcherapp { namespace commands { namespace onenote {

class CommandParam
{
public:
	CommandParam() = default;
	CommandParam(const CommandParam&) = default;
	~CommandParam() = default;

	CommandParam& operator = (const CommandParam& rhs) = default;

	bool Save(Settings& settings) const;
	bool Load(Settings& settings);

public:
	// 検索を有効化するためのプレフィックス
	CString mPrefix;
	// 検索を有効にする最小文字数
	int mMinTriggerLength{5};
	// 機能を利用するか?
	bool mIsEnable{true};
};


}}} // end of namespace launcherapp::commands::onenote
