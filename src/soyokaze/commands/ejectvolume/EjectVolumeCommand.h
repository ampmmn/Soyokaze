#pragma once

#include "commands/common/UserCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace ejectvolume {

	class CommandParam;

class EjectVolumeCommand :
 	virtual public launcherapp::commands::common::UserCommandBase	
{
public:
	EjectVolumeCommand();
	virtual ~EjectVolumeCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;

	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

// Editable
	// $B%3%^%s%I$rJT=8$9$k$?$a$N%@%$%"%m%0$r:n@.(B/$B<hF@$9$k(B
	virtual bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	// $B%@%$%"%m%0>e$G$NJT=87k2L$r%3%^%s%I$KE,MQ$9$k(B
	virtual bool Apply(launcherapp::core::CommandEditor* editor) override;
	// $B%@%$%"%m%0>e$G$NJT=87k2L$K4p$E$-!"?7$7$$%3%^%s%I$r:n@.(B($BJ#@=(B)$B$9$k(B
	virtual bool CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmd) override;

	static CString GetType();

	static bool NewDialog(Parameter* param);
	
public:
	void SetParam(const CommandParam& param);

	static CString TypeDisplayName();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

