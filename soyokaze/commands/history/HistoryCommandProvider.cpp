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


using CommandRepository = soyokaze::core::CommandRepository;

struct HistoryCommandProvider::PImpl
{
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(HistoryCommandProvider)


HistoryCommandProvider::HistoryCommandProvider() : in(new PImpl)
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
 	std::vector<CommandQueryItem>& commands
)
{
	ExecuteHistory::ItemList items;
	ExecuteHistory::GetInstance()->GetItems(_T("history"), items);

	for (const auto& item : items) {

		int level = pattern->Match(item.mWord);
		if (level != Pattern::Mismatch) {
			auto command = new HistoryCommand(item.mWord);
			commands.push_back(CommandQueryItem(level, command));
		}
	}
}


} // end of namespace history
} // end of namespace commands
} // end of namespace soyokaze

