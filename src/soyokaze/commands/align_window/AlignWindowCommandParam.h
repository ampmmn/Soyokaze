#pragma once

#include "commands/align_window/AlignWindowCommandParamItem.h"
#include "commands/core/CommandEntryIF.h"
#include "hotkey/CommandHotKeyAttribute.h"
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

	bool Save(CommandEntryIF* entry);
	bool Load(CommandEntryIF* entry);

public:
	std::vector<ITEM> mItems;

	CString mName;
	CString mDescription;

	CommandHotKeyAttribute mHotKeyAttr;
	// ウインドウが見つからなかった場合に通知
	bool mIsNotifyIfWindowNotFound;
	// アクティブなウインドウを実行後も変えない
	bool mIsKeepActiveWindow;
	// 自動実行を許可するか?
	bool mIsAllowAutoExecute;
};



}
}
}

