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

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	bool CanExecute() override;
	BOOL Execute(Parameter* param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsAllowAutoExecute() override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
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
