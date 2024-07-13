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


	WebSearchCommandList mCommands;

	uint32_t mRefCount = 1;
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

// 初回起動の初期化を行う
void WebSearchProvider::OnFirstBoot()
{
}

static void releaseCmd(void* p)
{
	auto ptr = (WebSearchCommand*)p;
	if (ptr) {
		ptr->Release();
	}
}

// コマンドの読み込み
void WebSearchProvider::LoadCommands(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto cmdRepo = CommandRepository::GetInstance();

	WebSearchCommandList tmp;

	int entries = cmdFile->GetEntryCount();
	for (int i = 0; i < entries; ++i) {

		auto entry = cmdFile->GetEntry(i);
		if (cmdFile->IsUsedEntry(entry)) {
			// 既にロード済(使用済)のエントリ
			continue;
		}

		std::unique_ptr<WebSearchCommand> command(new WebSearchCommand);
		if (command->Load(entry) == false) {
			continue;
		}

		// 登録
		bool isReloadHotKey = false;
		cmdRepo->RegisterCommand(command.get(), isReloadHotKey);

		command->AddRef();  // mCommandsで保持する分の参照カウント+1
		tmp.push_back(WebSearchCommandPtr(command.release(), releaseCmd));

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}

	in->mCommands.swap(tmp);
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
	CString description((LPCTSTR)IDS_DESCRIPTION_WEBSEARCH);
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

// 非公開コマンドかどうか(新規作成対象にしない)
bool WebSearchProvider::IsPrivate() const
{
	return false;
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

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool WebSearchProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	UNREFERENCED_PARAMETER(parent);
	UNREFERENCED_PARAMETER(pages);

	// 必要に応じて実装する
	return true;
}

uint32_t WebSearchProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t WebSearchProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace websearch
} // end of namespace commands
} // end of namespace launcherapp

