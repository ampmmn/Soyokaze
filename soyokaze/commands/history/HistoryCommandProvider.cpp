#include "pch.h"
#include "HistoryCommandProvider.h"
#include "commands/history/HistoryCommand.h"
#include "commands/common/ExecuteHistory.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ExecuteHistory = soyokaze::commands::common::ExecuteHistory;

namespace soyokaze {
namespace commands {
namespace history {

constexpr DWORD CHECK_INTERVAL = 1000;  // 10秒

using CommandRepository = soyokaze::core::CommandRepository;

struct HistoryCommandProvider::PImpl
{
	bool mQuering = false;
	DWORD mLastCheck = 0;
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

	if (GetTickCount() - in->mLastCheck > CHECK_INTERVAL) {
		// 一定間隔でコマンドが存在するかどうかをチェックする

		std::set<CString> missingWords;
		std::set<CString> existingCmds;

		auto repos = CommandRepository::GetInstance();
		for (auto it = items.begin(); it != items.end(); ) {

			auto& item = *it;

			CommandParameter param(item.mWord);
			auto cmdName = param.GetCommandString();

			if (existingCmds.find(item.mWord) != existingCmds.end()) {
				++it;
				continue;
			}

			auto cmd = repos->QueryAsWholeMatch(cmdName, true);
			if (cmd != nullptr) {
				cmd->Release();
				existingCmds.insert(cmdName);
				it++;
				continue;
			}

			missingWords.insert(item.mWord);
			it = items.erase(it);
		}

		history->EraseItems(_T("history"), missingWords);

		in->mLastCheck = GetTickCount();
	}

	for (const auto& item : items) {

		int level = pattern->Match(item.mWord);
		if (level != Pattern::Mismatch) {
			commands.push_back(CommandQueryItem(level, new HistoryCommand(item.mWord)));
		}
	}
}


} // end of namespace history
} // end of namespace commands
} // end of namespace soyokaze

