#include "pch.h"
#include "WebHistoryProvider.h"
#include "commands/webhistory/WebHistoryCommand.h"
#include "commands/webhistory/WebHistoryAdhocCommand.h"
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
namespace webhistory {

using WebHistoryCommandPtr = std::unique_ptr<WebHistoryCommand, std::function<void(void*)> >;
using WebHistoryCommandList = std::vector<WebHistoryCommandPtr>;

using CommandRepository = launcherapp::core::CommandRepository;

struct WebHistoryProvider::PImpl : public launcherapp::core::CommandRepositoryListenerIF
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


	WebHistoryCommandList mCommands;

	uint32_t mRefCount = 1;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(WebHistoryProvider)


WebHistoryProvider::WebHistoryProvider() : in(std::make_unique<PImpl>())
{
}

WebHistoryProvider::~WebHistoryProvider()
{
}

// 初回起動の初期化を行う
void WebHistoryProvider::OnFirstBoot()
{
}

static void releaseCmd(void* p)
{
	auto ptr = (WebHistoryCommand*)p;
	if (ptr) {
		ptr->Release();
	}
}

// コマンドの読み込み
void WebHistoryProvider::LoadCommands(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto cmdRepo = CommandRepository::GetInstance();

	WebHistoryCommandList tmp;

	int entries = cmdFile->GetEntryCount();
	for (int i = 0; i < entries; ++i) {

		auto entry = cmdFile->GetEntry(i);
		if (cmdFile->IsUsedEntry(entry)) {
			// 既にロード済(使用済)のエントリ
			continue;
		}

		std::unique_ptr<WebHistoryCommand> command(new WebHistoryCommand);
		if (command->Load(entry) == false) {
			continue;
		}

		// 登録
		bool isReloadHotKey = false;
		cmdRepo->RegisterCommand(command.get(), isReloadHotKey);

		command->AddRef();  // mCommandsで保持する分の参照カウント+1
		tmp.push_back(WebHistoryCommandPtr(command.release(), releaseCmd));

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}

	in->mCommands.swap(tmp);
}


CString WebHistoryProvider::GetName()
{
	return _T("WebHistoryCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString WebHistoryProvider::GetDisplayName()
{
	return _T("ブラウザ履歴コマンド");
}

// コマンドの種類の説明を示す文字列を取得
CString WebHistoryProvider::GetDescription()
{
	return _T("ブラウザで閲覧したページを検索するコマンドです");
}

// コマンド新規作成ダイアログ
bool WebHistoryProvider::NewDialog(const CommandParameter* param)
{
	std::unique_ptr<WebHistoryCommand> newCmd;
	if (WebHistoryCommand::NewDialog(param, newCmd) == false) {
		return false;
	}

	newCmd->AddRef();  // mCommandsで保持する分の参照カウント+1
	in->mCommands.push_back(WebHistoryCommandPtr(newCmd.get(), releaseCmd));

	bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release(), isReloadHotKey);
	return true;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool WebHistoryProvider::IsPrivate() const
{
	return false;
}

// 一時的なコマンドを必要に応じて提供する
void WebHistoryProvider::QueryAdhocCommands(
	Pattern* pattern,
 	std::vector<CommandQueryItem>& commands
)
{
	// 完全一致検索の場合は検索ワード補完をしない
	if (pattern->shouldWholeMatch()) {
		return;
	}

	std::vector<HISTORY> histories;

	CString url;
	CString displayName;
	for (auto& cmd : in->mCommands) {

		if (cmd->QueryHistories(pattern, histories) == false) {
			continue;
		}

		for (auto& history : histories) {
			auto adhocCmd = new WebHistoryAdhocCommand(history);
			commands.push_back(CommandQueryItem(history.mMatchLevel, adhocCmd));
		}
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t WebHistoryProvider::GetOrder() const
{
	return 145;
}

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool WebHistoryProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	UNREFERENCED_PARAMETER(parent);
	UNREFERENCED_PARAMETER(pages);

	// 必要に応じて実装する
	return true;
}

uint32_t WebHistoryProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t WebHistoryProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

