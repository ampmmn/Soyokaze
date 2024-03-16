#pragma once

#include "hotkey/HotKeyAttribute.h"
#include <regex>

namespace soyokaze {
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

	bool mIsEnableShortcut;

	// アイコンデータ(空の場合はデフォルトアイコンを使用)
	std::vector<uint8_t> mIconData;
};



}
}
}

