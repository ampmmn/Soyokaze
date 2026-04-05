#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/SelectionBehavior.h"
#include "commands/place_window_in_region/PlaceWindowInRegionParameter.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace place_window_in_region {

class PlaceWindowInRegionAdhocCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::core::SelectionBehavior


{
public:
	PlaceWindowInRegionAdhocCommand(HWND hwnd, LPCTSTR name, const CommandParam& param);
	virtual ~PlaceWindowInRegionAdhocCommand();

	CString GetTypeDisplayName() override;
	// 修飾キー押下状態に対応した実行アクションを取得する
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// SelectionBehavior
	// 選択された
	void OnSelect(Command* prior) override;
	// 選択解除された
	void OnUnselect(Command* next) override;
	// 実行後のウインドウを閉じる方法を決定する
	CloseWindowPolicy GetCloseWindowPolicy(uint32_t modifierMask) override;
	// 選択時に入力欄に設定するキーワードとキャレットを設定する
	bool CompleteKeyword(CString& keyword, int& startPos, int& endPos) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(PlaceWindowInRegionAdhocCommand)

//
public:
	static CString TypeDisplayName();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace place_window_in_region
} // end of namespace commands
} // end of namespace launcherapp

