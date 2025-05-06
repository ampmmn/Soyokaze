#include "pch.h"
#include "RemoteClientCommandProvider.h"
#include "commands/remote/RemoteClientCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/CommandFile.h"
#include "setting/AppPreference.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace remote {


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(RemoteClientCommandProvider)

IMPLEMENT_LOADFROM(RemoteClientCommandProvider, RemoteClientCommand)

RemoteClientCommandProvider::RemoteClientCommandProvider()
{
}

RemoteClientCommandProvider::~RemoteClientCommandProvider()
{
}

CString RemoteClientCommandProvider::GetName()
{
	return _T("RemoteClientCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString RemoteClientCommandProvider::GetDisplayName()
{
	return _T("リモートコマンド");
}

// コマンドの種類の説明を示す文字列を取得
CString RemoteClientCommandProvider::GetDescription()
{
	return _T("SSH経由でリモート側で動作しているランチャーのコマンドを実行します");
}

// 非公開コマンドかどうか
bool RemoteClientCommandProvider::IsPrivate() const
{
	// リモートクライアント機能が有効なときだけ、機能を利用できるようにする
	return AppPreference::Get()->IsEnableRemoteClient() == false;
}

// コマンド新規作成ダイアログ
bool RemoteClientCommandProvider::NewDialog(CommandParameter* param)
{
	return RemoteClientCommand::NewDialog(param);
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t RemoteClientCommandProvider::RemoteClientCommandProvider::GetOrder() const
{
	return 1000;
}

}}} // end of namespace launcherapp::commands::remote
