#include "pch.h"
#include "URLDirectoryIndexCommandProvider.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommand.h"
#include "commands/url_directoryindex/DirectoryIndexAdhocCommand.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommandParam.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "hotkey/CommandHotKeyManager.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "commands/core/CommandFile.h"
#include "resource.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace url_directoryindex {



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(URLDirectoryIndexCommandProvider)


URLDirectoryIndexCommandProvider::URLDirectoryIndexCommandProvider()
{
}

URLDirectoryIndexCommandProvider::~URLDirectoryIndexCommandProvider()
{
}

CString URLDirectoryIndexCommandProvider::GetName()
{
	return _T("URLDirectoryIndexCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString URLDirectoryIndexCommandProvider::GetDisplayName()
{
	return _T("DirectoryIndexコマンド");
}

// コマンドの種類の説明を示す文字列を取得
CString URLDirectoryIndexCommandProvider::GetDescription()
{
	static LPCTSTR description = _T("Webサーバーのインデックスページ上のファイルやディレクトリなどを候補として表示します");
	return description;
}

// コマンド新規作成ダイアログ
bool URLDirectoryIndexCommandProvider::NewDialog(CommandParameter* param)
{
	URLDirectoryIndexCommand* newCmd = nullptr;
	if (URLDirectoryIndexCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	constexpr bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd, isReloadHotKey);

	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t URLDirectoryIndexCommandProvider::URLDirectoryIndexCommandProvider::GetOrder() const
{
	return 400;
}

void URLDirectoryIndexCommandProvider::OnBeforeLoad()
{
}

bool URLDirectoryIndexCommandProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<URLDirectoryIndexCommand> command(new URLDirectoryIndexCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);
	*retCommand = command.release();

	return true;
}


} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

