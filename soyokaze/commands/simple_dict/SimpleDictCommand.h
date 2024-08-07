#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/core/CommandQueryItem.h"
#include "commands/simple_dict/SimpleDictParam.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace simple_dict {

class CommandUpdateListenerIF;

class SimpleDictCommand : public launcherapp::commands::common::UserCommandBase
{
public:
	SimpleDictCommand();
	virtual ~SimpleDictCommand();

	void AddListener(CommandUpdateListenerIF* listener);
	void RemoveListener(CommandUpdateListenerIF* listener);

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
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

	static bool NewDialog(const Parameter* param, SimpleDictCommand** newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, SimpleDictCommand** newCmdPtr);

	const SimpleDictParam& GetParam();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

