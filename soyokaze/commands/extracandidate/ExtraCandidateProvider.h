#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace extracandidate {


// ToDo: repository?がLoadCommandsを呼ぶ前にBeforeLoadイベントを発生させる(BeforeLoadでコマンドを消す実装を入れる->でないとreloadで増殖する問題が起こる)
class ExtraCandidateProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	ExtraCandidateProvider();
	~ExtraCandidateProvider() override;

public:
	CString GetName() override;

	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;

	DECLARE_COMMANDPROVIDER(ExtraCandidateProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace extracandidate
} // end of namespace commands
} // end of namespace launcherapp

