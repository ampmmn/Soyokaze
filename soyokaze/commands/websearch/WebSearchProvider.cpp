#include "pch.h"
#include "WebSearchProvider.h"
#include "commands/websearch/WebSearchCommand.h"
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

namespace soyokaze {
namespace commands {
namespace websearch {

using WebSearchCommandPtr = std::unique_ptr<WebSearchCommand, std::function<void(void*)> >;
using WebSearchCommandList = std::vector<WebSearchCommandPtr>;

using CommandRepository = soyokaze::core::CommandRepository;

struct WebSearchProvider::PImpl : public soyokaze::core::CommandRepositoryListenerIF
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

	void OnDeleteCommand(soyokaze::core::Command* cmd) override
 	{
		for (auto it = mCommands.begin(); it != mCommands.end(); ++it) {
			if ((*it).get() != cmd) {
				continue;
			}

			mCommands.erase(it);
			break;
		}
	}

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

		std::unique_ptr<WebSearchCommand> command;
		if (WebSearchCommand::LoadFrom(cmdFile, entry, command) == false) {
			continue;
		}

		// 登録
		cmdRepo->RegisterCommand(command.get());

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

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());
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

	for (auto& cmd : in->mCommands) {
		if (cmd->IsEnableShortcut() == false) {
			continue;
		}

		// コマンド名に一致する場合は表示しない
		// (一時的なcommandてはなく、通常のWeb検索コマンドとして候補に表示されるため)
		int matchLevel = pattern->Match(cmd->GetName());
		if (matchLevel == Pattern::WholeMatch) {
			continue;
		}

		commands.push_back(CommandQueryItem(Pattern::WeakMatch, cmd->CloneAsAdhocCommand(pattern->GetWholeString())));
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
} // end of namespace soyokaze

