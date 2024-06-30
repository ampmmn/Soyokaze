#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/filter/FilterResult.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace filter {

class CommandParam;

class FilterCommand : public launcherapp::commands::common::UserCommandBase
{
public:
	FilterCommand();
	virtual ~FilterCommand();

	void ClearCache();
	void Query(Pattern* pattern, FilterResultList& results);

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	int EditDialog(const Parameter* param) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	bool IsPriorityRankEnabled() override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

	static CString GetType();

	static bool NewDialog(const Parameter* param, FilterCommand** newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, FilterCommand** newCmdPtr);
	
public:
	FilterCommand& SetParam(const CommandParam& param);
	FilterCommand& GetParam(CommandParam& param);


protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

