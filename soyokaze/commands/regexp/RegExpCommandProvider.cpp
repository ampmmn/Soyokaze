#include "pch.h"
#include "RegExpCommandProvider.h"
#include "commands/regexp/RegExpCommand.h"
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
namespace regexp {


struct RegExpCommandProvider::PImpl
{
	uint32_t mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(RegExpCommandProvider)


RegExpCommandProvider::RegExpCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mRefCount = 1;
}

RegExpCommandProvider::~RegExpCommandProvider()
{
}

// 初回起動の初期化を行う
void RegExpCommandProvider::OnFirstBoot()
{
	// 特に何もしない
}


// コマンドの読み込み
void RegExpCommandProvider::LoadCommands(
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
		if (typeStr.IsEmpty() == FALSE && typeStr != RegExpCommand::GetType()) {
			continue;
		}

		auto command = std::make_unique<RegExpCommand>();
		if (command->Load(cmdFile, entry) == false) {
			continue;
		}

		// 登録
		cmdRepo->RegisterCommand(command.release());

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}
}

CString RegExpCommandProvider::GetName()
{
	return _T("RegExpCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString RegExpCommandProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_REGEXPCOMMAND);
}

// コマンドの種類の説明を示す文字列を取得
CString RegExpCommandProvider::GetDescription()
{
	return CString((LPCTSTR)IDS_DESCRIPTION_REGEXPCOMMAND);
}

// コマンド新規作成ダイアログ
bool RegExpCommandProvider::NewDialog(const CommandParameter* param)
{
	return RegExpCommand::NewDialog(param);
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool RegExpCommandProvider::IsPrivate() const
{
	return false;
}

// 一時的なコマンドを必要に応じて提供する
void RegExpCommandProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands)
{
	// このCommandProviderは一時的なコマンドを持たない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t RegExpCommandProvider::RegExpCommandProvider::GetOrder() const
{
	return 150;
}

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool RegExpCommandProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	// 必要に応じて実装する
	return true;
}

uint32_t RegExpCommandProvider::RegExpCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t RegExpCommandProvider::Release()
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
