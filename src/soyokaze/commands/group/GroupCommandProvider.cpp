#include "pch.h"
#include "GroupCommandProvider.h"
#include "commands/group/GroupCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "hotkey/CommandHotKeyManager.h"
#include "commands/group/GroupCommandEditor.h"
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(GroupCommandProvider)

IMPLEMENT_LOADFROM(GroupCommandProvider, GroupCommand)

GroupCommandProvider::GroupCommandProvider()
{
}

GroupCommandProvider::~GroupCommandProvider()
{
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
bool GroupCommandProvider::NewDialog(CommandParameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// グループ作成ダイアログを表示
	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<GroupCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());

	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t GroupCommandProvider::GroupCommandProvider::GetOrder() const
{
	return 300;
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t GroupCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(GroupCommand::TypeDisplayName());
	return 1;
}

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

