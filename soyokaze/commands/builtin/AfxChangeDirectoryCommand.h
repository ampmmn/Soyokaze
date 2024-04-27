#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class AfxChangeDirectoryCommand : public BuiltinCommandBase
{
public:
	AfxChangeDirectoryCommand(LPCTSTR name = nullptr);
	virtual ~AfxChangeDirectoryCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory邨檎罰縺ｧ繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ繧堤函謌舌〒縺阪ｋ繧医≧縺ｫ縺吶ｋ縺溘ａ縺ｮ謇狗ｶ壹″
	DECLARE_BUILTINCOMMAND(AfxChangeDirectoryCommand)
};
	
}
}
}
