#include "pch.h"
#include "WebSearchProvider.h"
#include "commands/websearch/WebSearchCommand.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace websearch {

using CommandRepository = soyokaze::core::CommandRepository;

struct WebSearchProvider::PImpl
{
	PImpl()
	{
	}
	virtual ~PImpl()
	{
	}

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

// コマンドの読み込み
void WebSearchProvider::LoadCommands(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto cmdRepo = CommandRepository::GetInstance();

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
		cmdRepo->RegisterCommand(command.release());

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
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
	// サポートしない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t WebSearchProvider::GetOrder() const
{
	return 140;
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

