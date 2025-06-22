#include "pch.h"
#include "HistoryCommandProvider.h"
#include "commands/history/HistoryCommand.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
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


struct HistoryCommandProvider::PImpl : public AppPreferenceListenerIF
{
	void Load()
	{
		auto pref = AppPreference::Get();
		mIsEnable = pref->IsUseInputHistory();
	}

	// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override 
	{
		Load();
	}
	void OnAppExit()
	{
		ExecuteHistory::GetInstance()->Save();
		AppPreference::Get()->UnregisterListener(this);
	}

	bool mQuering{false};
	bool mIsEnable{false};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(HistoryCommandProvider)


HistoryCommandProvider::HistoryCommandProvider() : in(std::make_unique<PImpl>())
{
	AppPreference::Get()->RegisterListener(in.get());
}

HistoryCommandProvider::~HistoryCommandProvider()
{
}

CString HistoryCommandProvider::GetName()
{
	return _T("HistoryCommand");
}

// 一時的なコマンドの準備を行うための初期化。初回のQueryAdhocCommand前に呼ばれる。
void HistoryCommandProvider::PrepareAdhocCommands()
{
	ExecuteHistory::GetInstance()->Load();
	in->Load();
}

// 一時的なコマンドを必要に応じて提供する
void HistoryCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (in->mIsEnable == false) {
		return;
	}

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

	for (const auto& item : items) {

		int level = pattern->Match(item.mWholeWord);
		if (level != Pattern::Mismatch) {
			commands.Add(CommandQueryItem(level, new HistoryCommand(item.mWholeWord)));
		}
	}
}


} // end of namespace history
} // end of namespace commands
} // end of namespace launcherapp

