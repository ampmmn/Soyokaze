#pragma once

#include "commands/common/UserCommandBase.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace align_window {

class AlignWindowCommand : public launcherapp::commands::common::UserCommandBase
{
public:
	AlignWindowCommand();
	virtual ~AlignWindowCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeName() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	int EditDialog(HWND parent) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	bool IsPriorityRankEnabled() override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

	static CString GetType();

	static bool NewDialog(const Parameter* param, AlignWindowCommand** newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, AlignWindowCommand** newCmdPtr);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace align_window
} // end of namespace commands
} // end of namespace launcherapp

