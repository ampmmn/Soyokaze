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
	// $B8!:w$rM-8z$K$9$k:G>.J8;z?t(B
	int mMinTriggerLength{5};
	// $B5!G=$rMxMQ$9$k$+(B?
	bool mIsEnable{true};
	bool mIsEnableChrome{true};
	bool mIsEnableEdge{true};
	bool mIsUseURL{false};
};


}}} // end of namespace launcherapp::commands::bookmarks

