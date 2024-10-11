// あ
#pragma once

#include "commands/builtin/BuiltinCommandBase.h"
#include "commands/core/ExtraCandidateSourceIF.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class EditCommand : 
	virtual public BuiltinCommandBase,
	virtual public launcherapp::commands::core::ExtraCandidateSource
{
public:
	EditCommand(LPCTSTR name = nullptr);
	EditCommand(const EditCommand& rhs);
	virtual ~EditCommand();

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ExtraCandidateSource
	bool QueryCandidates(Pattern* pattern, CommandQueryItemList& commands) override;
	void ClearCache() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory邨檎罰縺ｧ繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ繧堤函謌舌〒縺阪ｋ繧医≧縺ｫ縺吶ｋ縺溘ａ縺ｮ謇狗ｶ壹″
	DECLARE_BUILTINCOMMAND(EditCommand)
};

}
}
}
