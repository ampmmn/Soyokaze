#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/core/SelectionBehavior.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace alias {

class CommandParam;

class AliasCommand :
 virtual	public launcherapp::commands::common::UserCommandBase,
 virtual public launcherapp::core::SelectionBehavior
{
public:
	AliasCommand();
	virtual ~AliasCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;

	bool CanResolve() override;
	bool Resolve(CString& value) override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsAllowAutoExecute() override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

 // SelectionBehavior
	/** 選択されたときの処理を行う */
	void OnSelect(Command* prior) override;
	/** 選択解除されたときの処理を行う */
	void OnUnselect(Command* next) override;
	/** 実行後のウインドウを閉じる方針を取得する */
	launcherapp::core::SelectionBehavior::CloseWindowPolicy GetCloseWindowPolicy(uint32_t modifierMask) override;
	/** 入力欄へ補完するキーワードを取得する */
	bool CompleteKeyword(CString& keyword, int& startPos, int& endPos) override;
	/** IFIDに対応するインターフェースを取得する */
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

// Editable
	// コマンドを編集するためのダイアログを作成/取得する
	virtual bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	// ダイアログ上での編集結果をコマンドに適用する
	virtual bool Apply(launcherapp::core::CommandEditor* editor) override;
	// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
	virtual bool CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmd) override;

	static CString GetType();
	static CString TypeDisplayName();

	static bool NewDialog(Parameter* param);
	
public:
	void SetParam(const CommandParam& param);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

