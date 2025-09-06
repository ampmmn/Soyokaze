#pragma once

#include "commands/common/UserCommandBase.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace activate_window {

class CommandParam;

class WindowActivateCommand : 
	virtual public launcherapp::commands::common::UserCommandBase
{
public:
	WindowActivateCommand();
	virtual ~WindowActivateCommand();

	void SetParam(const CommandParam& param);

// Command
	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(Parameter* param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsAllowAutoExecute() override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

// Editable
	// $B%3%^%s%I$rJT=8$9$k$?$a$N%@%$%"%m%0$r:n@.(B/$B<hF@$9$k(B
	bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	// $B%@%$%"%m%0>e$G$NJT=87k2L$r%3%^%s%I$KE,MQ$9$k(B
	bool Apply(launcherapp::core::CommandEditor* editor) override;
	// $B%@%$%"%m%0>e$G$NJT=87k2L$K4p$E$-!"?7$7$$%3%^%s%I$r:n@.(B($BJ#@=(B)$B$9$k(B
	bool CreateNewInstanceFrom(launcherapp::core::CommandEditor*editor, Command** newCmd) override;


	static CString GetType();
	static CString TypeDisplayName();

	static bool NewInstance(launcherapp::core::CommandEditor* editor, Command** newCmdPtr);
	static bool NewDialog(Parameter* param, WindowActivateCommand** newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, WindowActivateCommand** newCmdPtr);


protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

