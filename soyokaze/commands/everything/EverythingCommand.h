#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/everything/EverythingCommandParam.h"
#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace everything {

class EverythingResult;

class EverythingCommand : public launcherapp::commands::common::UserCommandBase
{
public:
	EverythingCommand();
	virtual ~EverythingCommand();

	void Query(Pattern* pattern, std::vector<EverythingResult>& results);

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	int EditDialog(const Parameter* param) override;
	bool IsPriorityRankEnabled() override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

	static CString GetType();

	static bool NewDialog(const Parameter* param, EverythingCommand** newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, EverythingCommand** newCmdPtr);

	const CommandParam& GetParam();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

