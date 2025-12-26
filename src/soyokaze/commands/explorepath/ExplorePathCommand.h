#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include "commands/core/SelectionBehavior.h"
#include "commands/core/ExtraActionHotKeySettings.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace explorepath {

class ExtraActionSettings;

class ExplorePathCommand :
 	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ContextMenuSource,
	virtual public launcherapp::core::SelectionBehavior,
	virtual public launcherapp::commands::core::ExtraActionHotKeySettings
{
public:
	ExplorePathCommand(const CString& fullPath);
	ExplorePathCommand(const CString& displayName, const CString& fullPath);
	virtual ~ExplorePathCommand();

	void SetExtraActionSettings(ExtraActionSettings* settings);

	void SetCompletionText(const CString& completion);

	CString GetName() override;
	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ContextMenuSource
	// メニューの項目数を取得する
	int GetMenuItemCount() override;
	// メニューに対応するアクションを取得する
	bool GetMenuItem(int index, Action** action) override;

// SelectionBehavior
	// 選択された
	void OnSelect(Command* prior) override;
	// 選択解除された
	void OnUnselect(Command* next) override;
	// 実行後のウインドウを閉じる方法
	CloseWindowPolicy GetCloseWindowPolicy(uint32_t modifierMask) override;
	// 選択時に入力欄に設定するキーワードとキャレットを設定する
	bool CompleteKeyword(CString& keyword, int& startPos, int& endPos) override;

// ExtraActionHotKeySettings
	// ホットキー設定の数を取得
	int GetHotKeyCount() override;
	// ホットキー設定を取得
	bool GetHotKeyAttribute(int index, HOTKEY_ATTR& hotkeyAttr) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	static CString TypeDisplayName(bool isFolder);

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(ExplorePathCommand)

protected:
	bool GetExtraAction(const HOTKEY_ATTR& hotkeyAttr, Action** action);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace explorepath
} // end of namespace commands
} // end of namespace launcherapp

