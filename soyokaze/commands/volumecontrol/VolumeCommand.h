#pragma once

#include "commands/common/UserCommandBase.h"
#include <memory>

class HOTKEY_ATTR;

namespace launcherapp {
namespace commands {
namespace volumecontrol {

	class CommandParam;

class VolumeCommand : public launcherapp::commands::common::UserCommandBase
{
public:
	VolumeCommand();
	virtual ~VolumeCommand();

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

	bool Save(CommandFile* cmdFile) override;

	bool Load(CommandFile* cmdFile, void* entry_);

	static CString GetType();

	static bool NewDialog(const Parameter* param);
	
public:
	void SetParam(const CommandParam& param);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

