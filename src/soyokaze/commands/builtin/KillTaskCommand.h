#pragma once

#include "commands/builtin/BuiltinCommandBase.h"
#include "commands/core/ExtraCandidateSourceIF.h"

namespace launcherapp {
namespace commands {
namespace builtin {

#pragma warning(push)
#pragma warning(disable : 4250)

class KillTaskCommand :
	virtual public BuiltinCommandBase,
	virtual public launcherapp::commands::core::ExtraCandidateSource
{
public:
	/** コマンドを生成する */
	KillTaskCommand(LPCTSTR name = nullptr);
	/** コマンドを複製する */
	KillTaskCommand(const KillTaskCommand& rhs);
	/** コマンドを破棄する */
	virtual ~KillTaskCommand();

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	int Match(Pattern* pattern) override;
	BOOL Execute(Parameter* param) override;
	launcherapp::core::Command* Clone() override;

	/** 実行中プロセスの候補を作成する */
	bool QueryCandidates(Pattern* pattern, CommandQueryItemList& commands) override;
	/** 候補作成用のキャッシュを破棄する */
	void ClearCache() override;

	CString GetType() override;
	static CString TYPE;

	DECLARE_BUILTINCOMMAND(KillTaskCommand)
};

#pragma warning(pop)

}
}
}
