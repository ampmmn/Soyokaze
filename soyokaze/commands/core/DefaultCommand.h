#pragma once

#include "commands/core/CommandIF.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace core {

// 未登録キーワードだったときのアクションを実装したコマンド
// このコマンドは特殊で、CommandRepositoryが直接保持する
// Providerを持たない
class DefaultCommand : public launcherapp::core::Command
{
public:
	DefaultCommand();
	virtual ~DefaultCommand();

	void SetName(const CString& word);

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeName() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsEditable() override;
	bool IsDeletable() override;
	int EditDialog(HWND parent) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	bool IsPriorityRankEnabled() override;
	Command* Clone() override;
	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
