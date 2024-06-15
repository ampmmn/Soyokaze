#include "pch.h"
#include "SnippetCommandProvider.h"
#include "commands/snippet/SnippetCommand.h"
#include "commands/snippet/RegisterSnippetCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace snippet {


struct SnippetCommandProvider::PImpl
{
	uint32_t mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(SnippetCommandProvider)


SnippetCommandProvider::SnippetCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mRefCount = 1;
}

SnippetCommandProvider::~SnippetCommandProvider()
{
}

// 初回起動の初期化を行う
void SnippetCommandProvider::OnFirstBoot()
{
}

// コマンドの読み込み
void SnippetCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	ASSERT(cmdFile);

	auto cmdRepo = CommandRepository::GetInstance();

	// スニペット登録コマンドがなければここで登録
	if (cmdRepo->HasCommand(RegisterSnippetCommand::DEFAULT_NAME) == false) {
		cmdRepo->RegisterCommand(new RegisterSnippetCommand(nullptr));
	}

	int entries = cmdFile->GetEntryCount();
	for (int i = 0; i < entries; ++i) {

		auto entry = cmdFile->GetEntry(i);
		if (cmdFile->IsUsedEntry(entry)) {
			// 既にロード済(使用済)のエントリ
			continue;
		}

		auto command = std::make_unique<SnippetCommand>();
		if (command->Load(entry) == false) {
			continue;
		}

		// 登録
		cmdRepo->RegisterCommand(command.release());

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}
}

CString SnippetCommandProvider::GetName()
{
	return _T("SnippetCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString SnippetCommandProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_SNIPPETCOMMAND);
}

// コマンドの種類の説明を示す文字列を取得
CString SnippetCommandProvider::GetDescription()
{
	return CString((LPCTSTR)IDS_DESCRIPTION_SNIPPETCOMMAND);
}

// コマンド新規作成ダイアログ
bool SnippetCommandProvider::NewDialog(const CommandParameter* param)
{
	return SnippetCommand::NewDialog(param);
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool SnippetCommandProvider::IsPrivate() const
{
	return false;
}

// 一時的なコマンドを必要に応じて提供する
void SnippetCommandProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands)
{
	// このCommandProviderは一時的なコマンドを持たない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t SnippetCommandProvider::SnippetCommandProvider::GetOrder() const
{
	return 120;
}

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool SnippetCommandProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	// 必要に応じて実装する
	return true;
}

uint32_t SnippetCommandProvider::SnippetCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t SnippetCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

}
}
}
