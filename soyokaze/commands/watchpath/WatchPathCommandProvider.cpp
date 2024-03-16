#include "pch.h"
#include "WatchPathCommandProvider.h"
#include "commands/watchpath/WatchPathCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = soyokaze::core::CommandRepository;

namespace soyokaze {
namespace commands {
namespace watchpath {


struct WatchPathCommandProvider::PImpl
{
	uint32_t mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(WatchPathCommandProvider)


WatchPathCommandProvider::WatchPathCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mRefCount = 1;
}

WatchPathCommandProvider::~WatchPathCommandProvider()
{
}

// 初回起動の初期化を行う
void WatchPathCommandProvider::OnFirstBoot()
{
}

// コマンドの読み込み
void WatchPathCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
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

		CString typeStr = cmdFile->Get(entry, _T("Type"), _T(""));
		if (typeStr.IsEmpty() == FALSE && typeStr != WatchPathCommand::GetType()) {
			continue;
		}

		auto command = std::make_unique<WatchPathCommand>();
		if (command->Load(cmdFile, entry) == false) {
			continue;
		}

		// 登録
		cmdRepo->RegisterCommand(command.release());

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}
}

CString WatchPathCommandProvider::GetName()
{
	return _T("WatchPathCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString WatchPathCommandProvider::GetDisplayName()
{
	return CString(_T("フォルダ更新検知"));
}

// コマンドの種類の説明を示す文字列を取得
CString WatchPathCommandProvider::GetDescription()
{
	return CString(_T("【試作】フォルダ内の更新を検知するコマンドです。"));
}

// コマンド新規作成ダイアログ
bool WatchPathCommandProvider::NewDialog(const CommandParameter* param)
{
	return WatchPathCommand::NewDialog(param);
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool WatchPathCommandProvider::IsPrivate() const
{
	return false;
}

// 一時的なコマンドを必要に応じて提供する
void WatchPathCommandProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands)
{
	// このCommandProviderは一時的なコマンドを持たない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t WatchPathCommandProvider::WatchPathCommandProvider::GetOrder() const
{
	return 2100;
}

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool WatchPathCommandProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	// 必要に応じて実装する
	return true;
}

uint32_t WatchPathCommandProvider::WatchPathCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t WatchPathCommandProvider::Release()
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
