#include "pch.h"
#include "GroupCommandProvider.h"
#include "commands/group/GroupCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "hotkey/CommandHotKeyManager.h"
#include "commands/group/GroupEditDialog.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace group {



struct GroupCommandProvider::PImpl
{
	uint32_t mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(GroupCommandProvider)


GroupCommandProvider::GroupCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mRefCount = 1;
}

GroupCommandProvider::~GroupCommandProvider()
{
}

// 初回起動の初期化を行う
void GroupCommandProvider::OnFirstBoot()
{
	// 特に何もしない
}


// コマンドの読み込み
void GroupCommandProvider::LoadCommands(
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

		auto command = std::make_unique<GroupCommand>();
		if (command->Load(entry) == false) {
			continue;
		}
		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);

		// 登録
		constexpr bool isReloadHotKey = false;
		cmdRepo->RegisterCommand(command.release(), isReloadHotKey);
	}
}

CString GroupCommandProvider::GetName()
{
	return _T("GroupCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString GroupCommandProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_GROUPCOMMAND);
}

// コマンドの種類の説明を示す文字列を取得
CString GroupCommandProvider::GetDescription()
{
	return CString((LPCTSTR)IDS_DESCRIPTION_GROUPCOMMAND);
}

// コマンド新規作成ダイアログ
bool GroupCommandProvider::NewDialog(const CommandParameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// グループ作成ダイアログを表示
	GroupEditDialog dlg;
	if (dlg.DoModal() != IDOK) {
		return false;
	}


	auto& paramNew = dlg.GetParam();

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = std::make_unique<GroupCommand>();
	newCmd->SetParam(paramNew);

	constexpr bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release(), isReloadHotKey);

	return true;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool GroupCommandProvider::IsPrivate() const
{
	return false;
}

// 一時的なコマンドを必要に応じて提供する
void GroupCommandProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands)
{
	UNREFERENCED_PARAMETER(pattern);
	UNREFERENCED_PARAMETER(comands);

	// このCommandProviderは一時的なコマンドを持たない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t GroupCommandProvider::GroupCommandProvider::GetOrder() const
{
	return 300;
}

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool GroupCommandProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	UNREFERENCED_PARAMETER(parent);
	UNREFERENCED_PARAMETER(pages);
	// 必要に応じて実装する
	return true;
}

uint32_t GroupCommandProvider::GroupCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t GroupCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

