#include "pch.h"
#include "WebSearchProvider.h"
#include "commands/websearch/WebSearchCommand.h"
#include "commands/websearch/WebSearchAdhocCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "resource.h"
#include <list>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace websearch {

using WebSearchCommandPtr = std::unique_ptr<WebSearchCommand, std::function<void(void*)> >;
using WebSearchCommandList = std::vector<WebSearchCommandPtr>;

using CommandRepository = launcherapp::core::CommandRepository;

struct WebSearchProvider::PImpl : public launcherapp::core::CommandRepositoryListenerIF
{
	PImpl()
	{
		auto cmdRepo = CommandRepository::GetInstance();
		cmdRepo->RegisterListener(this);

	}
	virtual ~PImpl()
	{
		auto cmdRepo = CommandRepository::GetInstance();
		cmdRepo->UnregisterListener(this);
	}

	void OnDeleteCommand(launcherapp::core::Command* cmd) override
 	{
		for (auto it = mCommands.begin(); it != mCommands.end(); ++it) {
			if ((*it).get() != cmd) {
				continue;
			}

			mCommands.erase(it);
			break;
		}
	}
	void OnLancuherActivate() override {}
	void OnLancuherUnactivate() override {}

	void ClearCommands()
	{
		mCommands.clear();
	}

	WebSearchCommandList mCommands;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(WebSearchProvider)


WebSearchProvider::WebSearchProvider() : in(std::make_unique<PImpl>())
{
}

WebSearchProvider::~WebSearchProvider()
{
}

static void releaseCmd(void* p)
{
	auto ptr = (WebSearchCommand*)p;
	if (ptr) {
		ptr->Release();
	}
}

CString WebSearchProvider::GetName()
{
	return _T("WebSearchCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString WebSearchProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_COMMANDNAME_WEBSEARCH);
}

// コマンドの種類の説明を示す文字列を取得
CString WebSearchProvider::GetDescription()
{
	static LPCTSTR description = _T("Web検索を定義するコマンドです\n")
		_T("コマンド名とWebの検索条件を登録することにより、\n")
		_T("「コマンド名 (検索キーワード)」の形で任意のWeb検索を行うことができます\n")
		_T("例: ggl→Google検索  tw→Twitter検索");
	return description;
}

// コマンド新規作成ダイアログ
bool WebSearchProvider::NewDialog(const CommandParameter* param)
{
	std::unique_ptr<WebSearchCommand> newCmd;
	if (WebSearchCommand::NewDialog(param, newCmd) == false) {
		return false;
	}

	newCmd->AddRef();  // mCommandsで保持する分の参照カウント+1
	in->mCommands.push_back(WebSearchCommandPtr(newCmd.get(), releaseCmd));

	bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release(), isReloadHotKey);
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void WebSearchProvider::QueryAdhocCommands(
	Pattern* pattern,
 	std::vector<CommandQueryItem>& commands
)
{
	// 完全一致検索の場合は検索ワード補完をしない
	if (pattern->shouldWholeMatch()) {
		return;
	}

	std::vector<std::unique_ptr<WebSearchAdhocCommand> > shortcutsCandidates;

	bool isHit = false;

	CString url;
	CString displayName;
	for (auto& cmd : in->mCommands) {

		int level = cmd->BuildSearchUrlString(pattern, displayName, url);
		if (level == Pattern::Mismatch) {
			continue;
		}

		auto adhocCmd = new WebSearchAdhocCommand(cmd.get(), displayName, url);
		if (level == Pattern::WeakMatch) {
			// 他のWeb検索コマンドでヒットするものがなければ表示するのでいったん貯めておく
			shortcutsCandidates.emplace_back(std::unique_ptr<WebSearchAdhocCommand>(adhocCmd));
			continue;
		}
		commands.push_back(CommandQueryItem(level, adhocCmd));
		isHit = true;
	}

	if (isHit == false) {
		// WeakMatchでヒットしたものは、他にヒットしたものがなかったら候補として表示する
		for (auto& adhocCmd : shortcutsCandidates) {
			commands.push_back(CommandQueryItem(Pattern::WeakMatch, adhocCmd.release()));
		}
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t WebSearchProvider::GetOrder() const
{
	return 140;
}

void WebSearchProvider::OnBeforeLoad()
{
	in->ClearCommands();
}

bool WebSearchProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<WebSearchCommand> command(new WebSearchCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);
	*retCommand = command.get();

	command->AddRef();
	in->mCommands.push_back(WebSearchCommandPtr(command.release(), releaseCmd));

	return true;
}


} // end of namespace websearch
} // end of namespace commands
} // end of namespace launcherapp

