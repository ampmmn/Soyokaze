#pragma once

#include "commands/core/CommandIF.h"
#include "commands/core/EditableIF.h"
#include "commands/builtin/BuiltinCommandFactory.h"
#include "utility/RefPtr.h"

namespace launcherapp {
namespace commands {
namespace builtin {

class BuiltinCommandBase :
 	virtual public launcherapp::core::Command,
	virtual public launcherapp::core::Editable
{
public:
	using Entry = BuiltinCommandFactory::Entry;
public:
	BuiltinCommandBase(LPCTSTR name = nullptr);
	BuiltinCommandBase(const BuiltinCommandBase& rhs);
	virtual ~BuiltinCommandBase();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	//BOOL Execute(const Parameter* param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

// Editable
	// コマンドは編集可能か?
	bool IsEditable() override;
	// コマンドは削除可能か?
	bool IsDeletable() override;
	// コマンドを編集するためのダイアログを作成/取得する
	bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	// ダイアログ上での編集結果をコマンドに適用する
	bool Apply(launcherapp::core::CommandEditor* editor) override;
	// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
	bool CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, launcherapp::core::Command** newCmd) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;
	uint32_t AddRef() override;
	uint32_t Release() override;


	virtual CString GetType() = 0;
	virtual void LoadFrom(Entry* entry);

protected:
	CString mName;
	CString mDescription;
	CString mError;
	uint32_t mRefCount;

	// 実行前に確認するか?
	bool mIsConfirmBeforeRun = false;
	// 実行前の確認有無を選択可能か?
	bool mCanSetConfirm = false;

	// 機能は有効か?
	bool mIsEnable = true;
	// 機能を無効化できるか?
	bool mCanDisable = false;
};

}
}
}

