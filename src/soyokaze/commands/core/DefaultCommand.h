#pragma once

#include "commands/core/CommandIF.h"
#include "commands/core/SelectionBehavior.h"

#include <memory>

namespace launcherapp {
namespace commands {
namespace core {

// 未登録キーワードだったときのアクションを実装したコマンド
// このコマンドは特殊で、CommandRepositoryが直接保持する
// Providerを持たない
class DefaultCommand :
 	public launcherapp::core::Command,
	public launcherapp::core::SelectionBehavior
{
public:
	DefaultCommand();
	virtual ~DefaultCommand();

	void SetName(const CString& word);

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;
	bool CanExecute(String*) override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsAllowAutoExecute() override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	Command* Clone() override;
	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

	// 選択された
	void OnSelect(Command* prior) override;
	// 選択解除された
	void OnUnselect(Command* next) override;
	// 実行後のウインドウを閉じる方法
	CloseWindowPolicy GetCloseWindowPolicy(uint32_t modifierMask) override;
	// 選択時に入力欄に設定するキーワードとキャレットを設定する
	bool CompleteKeyword(CString& keyword, int& startPos, int& endPos) override;

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
