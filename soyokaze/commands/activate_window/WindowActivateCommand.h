#pragma once

#include "commands/core/CommandIF.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace activate_window {

class WindowActivateCommand : public soyokaze::core::Command
{
public:
	WindowActivateCommand();
	virtual ~WindowActivateCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsEditable() override;
	int EditDialog(const Parameter* param) override;
	bool IsPriorityRankEnabled() override;
	soyokaze::core::Command* Clone() override;

	bool Save(CommandFile* cmdFile) override;

	uint32_t AddRef() override;
	uint32_t Release() override;

	static CString GetType();

	static bool NewDialog(const Parameter* param, WindowActivateCommand** newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, WindowActivateCommand** newCmdPtr);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace activate_window
} // end of namespace commands
} // end of namespace soyokaze

