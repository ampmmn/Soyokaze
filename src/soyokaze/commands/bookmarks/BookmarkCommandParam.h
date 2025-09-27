#pragma once

#include "setting/Settings.h"

namespace launcherapp { namespace commands { namespace bookmarks {

class CommandParam
{
public:
	CommandParam() = default;
	~CommandParam() = default;

	bool Save(Settings& settings) const;
	bool Load(Settings& settings);

public:
	CString mPrefix;
	// 検索を有効にする最小文字数
	int mMinTriggerLength{5};
	// 機能を利用するか?
	bool mIsEnable{true};
	bool mIsEnableAltBrowser{true};
	bool mIsEnableEdge{true};
	bool mIsUseURL{false};
};


}}} // end of namespace launcherapp::commands::bookmarks

