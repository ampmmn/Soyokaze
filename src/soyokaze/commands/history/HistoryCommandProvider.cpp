#include "pch.h"
#include "HistoryCommandProvider.h"
#include "commands/history/HistoryCommand.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ExecuteHistory = launcherapp::commands::common::ExecuteHistory;

namespace launcherapp {
namespace commands {
namespace history {

constexpr DWORD CHECK_INTERVAL = 1000;  // 10秒

using CommandRepository = launcherapp::core::CommandRepository;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;


struct HistoryCommandProvider::PImpl
{
	bool mQuering = false;
	uint64_t mLastCheck = 0;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(HistoryCommandProvider)


HistoryCommandProvider::HistoryCommandProvider() : in(std::make_unique<PImpl>())
{
}

HistoryCommandProvider::~HistoryCommandProvider()
{
}

CString HistoryCommandProvider::GetName()
{
	return _T("HistoryCommand");
}

// 一時的なコマンドを必要に応じて提供する
void HistoryCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	// 再入しない
	if (in->mQuering) {
		return;
	}
	struct scope_quering {
		scope_quering(bool& f) : mFlag(f) { mFlag = true; }
		~scope_quering() { mFlag = false; }
		bool& mFlag;
	} scope(in->mQuering);

	ExecuteHistory::ItemList items;
	auto history = ExecuteHistory::GetInstance();
	history->GetItems(_T("history"), items);

	if (GetTickCount64() - in->mLastCheck > CHECK_INTERVAL) {
		// 一定間隔でコマンドが存在するかどうかをチェックする

		std::set<CString> missingWords;
		std::set<CString> existingCmds;

		auto repos = CommandRepository::GetInstance();
		for (auto it = items.begin(); it != items.end(); ) {

			auto& item = *it;

			RefPtr<CommandParameterBuilder> param(CommandParameterBuilder::Create(item.mWord), false);
			auto cmdName = param->GetCommandString();

			if (existingCmds.find(item.mWord) != existingCmds.end()) {
				++it;
				continue;
			}

			RefPtr<launcherapp::core::Command> cmd(repos->QueryAsWholeMatch(cmdName, true));
			if (cmd != nullptr) {
				existingCmds.insert(cmdName);
				it++;
				continue;
			}

			missingWords.insert(item.mWord);
			it = items.erase(it);
		}

		history->EraseItems(_T("history"), missingWords);

		in->mLastCheck = GetTickCount64();
	}

	for (const auto& item : items) {

		int level = pattern->Match(item.mWord);
		if (level != Pattern::Mismatch) {
			commands.Add(CommandQueryItem(level, new HistoryCommand(item.mWord)));
		}
	}
}


} // end of namespace history
} // end of namespace commands
} // end of namespace launcherapp

