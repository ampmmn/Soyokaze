#pragma once

#include "setting/Settings.h"

namespace launcherapp { namespace commands { namespace webhistory {

class CommandParam
{
public:
	CommandParam() = default;
	~CommandParam() = default;

	bool Save(Settings& settings) const;
	bool Load(Settings& settings);

public:
	CString mPrefix;
	// 検索件数上限
	int mLimit{20};
	// 検索を有効にする最小文字数
	int mMinTriggerLength{5};
	bool mIsEnable{true};
	bool mIsEnableEdge{true};
	bool mIsEnableAltBrowser{true};
	bool mIsUseMigemo{true};
	bool mIsUseURL{false};
};



}}}

