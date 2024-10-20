#pragma once

#include "commands/builtin/BuiltinCommandBase.h"
#include "commands/core/ExtraCandidateSourceIF.h"

namespace launcherapp {
namespace commands {
namespace builtin {

#pragma warning(push)
#pragma warning(disable : 4250)

class EditCommand : 
	virtual public BuiltinCommandBase,
	virtual public launcherapp::commands::core::ExtraCandidateSource
{
public:
	EditCommand(LPCTSTR name = nullptr);
	EditCommand(const EditCommand& rhs);
	virtual ~EditCommand();

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	int Match(Pattern* pattern) override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ExtraCandidateSource
	bool QueryCandidates(Pattern* pattern, CommandQueryItemList& commands) override;
	void ClearCache() override;

	CString GetType() override;
	static CString TYPE;

	DECLARE_BUILTINCOMMAND(EditCommand)
};

#pragma warning(pop)

}
}
}
