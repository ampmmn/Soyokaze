#pragma once

#include "commands/align_window/AlignWindowCommandParamItem.h"
#include "hotkey/HotKeyAttribute.h"
#include <regex>
#include <vector>

namespace launcherapp {
namespace commands {
namespace align_window {

class CommandParam
{
public:
	using ACTION = launcherapp::commands::align_window::ACTION;
	using ITEM = launcherapp::commands::align_window::ITEM;

public:
	CommandParam();
	~CommandParam();

public:
	std::vector<ITEM> mItems;

	CString mName;
	CString mDescription;

	HOTKEY_ATTR mHotKeyAttr;

	bool mIsGlobal;
	// ウインドウが見つからなかった場合に通知
	bool mIsNotifyIfWindowNotFound;
	// アクティブなウインドウを実行後も変えない
	bool mIsKeepActiveWindow;
};



}
}
}

