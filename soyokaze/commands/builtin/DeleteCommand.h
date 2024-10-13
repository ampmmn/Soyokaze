#pragma once

#include "commands/builtin/BuiltinCommandBase.h"
#include "commands/core/ExtraCandidateSourceIF.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class DeleteCommand :
	virtual public BuiltinCommandBase,
	virtual public launcherapp::commands::core::ExtraCandidateSource
{
public:
	DeleteCommand(LPCTSTR name = nullptr);
	DeleteCommand(const DeleteCommand& rhs);
	virtual ~DeleteCommand();

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

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(DeleteCommand)
};

}
}
}
