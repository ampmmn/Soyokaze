#pragma once

#include "commands/core/CommandIF.h"
#include "commands/builtin/BuiltinCommandFactory.h"

namespace launcherapp {
namespace commands {
namespace builtin {

class BuiltinCommandBase : public launcherapp::core::Command
{
public:
	using Entry = BuiltinCommandFactory::Entry;
public:
	BuiltinCommandBase(LPCTSTR name = nullptr);
	BuiltinCommandBase(const BuiltinCommandBase& rhs);
	virtual ~BuiltinCommandBase();

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeName() override;
	CString GetTypeDisplayName() override;
	//BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsEditable() override;
	bool IsDeletable() override;
	int EditDialog(HWND parent) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	bool IsPriorityRankEnabled() override;
	//launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

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

