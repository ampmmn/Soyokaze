#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include <regex>

namespace launcherapp {
namespace commands {
namespace websearch {

class CommandParam
{
public:
	CommandParam();
	~CommandParam();

	bool IsEnableShortcutSearch() const;

public:
	CString mName;
	CString mDescription;

	CString mURL;

	CommandHotKeyAttribute mHotKeyAttr;

	bool mIsEnableShortcut;

	// アイコンデータ(空の場合はデフォルトアイコンを使用)
	std::vector<uint8_t> mIconData;
};



}
}
}

