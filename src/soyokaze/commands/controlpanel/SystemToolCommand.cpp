#include "pch.h"
#include "framework.h"
#include "SystemToolCommand.h"
#include "actions/builtin/ExecuteAction.h"
#include "icon/IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;

namespace launcherapp {
namespace commands {
namespace controlpanel {

struct SystemToolCommand::PImpl
{
	std::vector<CString> mCommandLine;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(SystemToolCommand)

/**
  システムツールコマンドを初期化する
  @param[in] name コマンドの表示名
  @param[in] description コマンドの説明
  @param[in] commandLine 実行するコマンドラインの配列
*/
SystemToolCommand::SystemToolCommand(
	const CString& name,
	const CString& description,
	const std::vector<CString>& commandLine
) : AdhocCommandBase(name, description),
	in(std::make_unique<PImpl>())
{
	in->mCommandLine = commandLine;
}

/**
  システムツールコマンドを破棄する
*/
SystemToolCommand::~SystemToolCommand()
{
}

/**
  システムツールを実行するアクションを作成する
  @param[in] hotkeyAttr ホットキー属性
  @param[out] action 作成したアクション
  @return true:アクションを作成した  false:作成しない
*/
bool SystemToolCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() != 0 || in->mCommandLine.empty()) {
		return false;
	}

	CString parameters;
	for (size_t i = 1; i < in->mCommandLine.size(); ++i) {
		if (parameters.IsEmpty() == FALSE) {
			parameters += _T(" ");
		}

		// FIXME: ControlPanel.cppのSYSTEM_TOOL_DEFINITIONSで空白を含むパラメータが使われる要素が出できた場合は必要に応じてダブルクォーテーションで囲む処理を実装する
		parameters += in->mCommandLine[i];
	}

	*action = new ExecuteAction(in->mCommandLine[0], parameters);
	return true;
}

/**
  システムツールコマンドのアイコンを取得する
  @return システムツール用のアイコン
*/
HICON SystemToolCommand::GetIcon()
{
	return IconLoader::Get()->GetShell32Icon(-22);
}

/**
  システムツールコマンドを複製する
  @return 複製したコマンド
*/
launcherapp::core::Command* SystemToolCommand::Clone()
{
	return new SystemToolCommand(mName, mDescription, in->mCommandLine);
}

// コンテキストメニューの項目数を取得する
int SystemToolCommand::GetMenuItemCount()
{
	return 1;
}

// コンテキストメニューに対応するアクションを取得する
bool SystemToolCommand::GetMenuItem(int index, Action** action)
{
	if (index == 0) {
		return GetAction(HOTKEY_ATTR(0, VK_RETURN), action);
	}
	return false;
}

bool SystemToolCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ContextMenuSource*)this;
		return true;
	}
	return false;
}

/**
  コマンド種別の表示名を取得する
  @return コマンド種別の表示名
*/
CString SystemToolCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

/**
  システムツールのコマンド種別表示名を取得する
  @return コマンド種別の表示名
*/
CString SystemToolCommand::TypeDisplayName()
{
	return _T("システムツール");
}

} // controlpanel名前空間終了
} // commands名前空間終了
} // launcherapp名前空間終了
