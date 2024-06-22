#pragma once

#include "hotkey/HotKeyAttribute.h"
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

	HOTKEY_ATTR mHotKeyAttr;
	bool mIsGlobal;

	bool mIsEnableShortcut;

	// アイコンデータ(空の場合はデフォルトアイコンを使用)
	std::vector<uint8_t> mIconData;
};



}
}
}

