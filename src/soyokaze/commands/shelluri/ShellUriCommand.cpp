#include "pch.h"
#include "ShellUriCommand.h"
#include "commands/common/SubProcess.h"
#include "actions/builtin/CallbackAction.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "utility/Regex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace shelluri {

using CallbackAction = launcherapp::actions::builtin::CallbackAction;
using Parameter = launcherapp::actions::core::Parameter;
using SubProcess = launcherapp::commands::common::SubProcess;

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(ShellUriCommand)

/**
  Shell URIコマンドを初期化する
*/
ShellUriCommand::ShellUriCommand() :
	AdhocCommandBase(_T(""), _T(""))
{
}

/**
  Shell URIコマンドを破棄する
*/
ShellUriCommand::~ShellUriCommand()
{
}

/**
  入力されたShell URIをコマンド名として取得する
  @return 入力されたShell URI
*/
CString ShellUriCommand::GetName()
{
	return mDescription;
}

/**
  入力されたShell URIをコマンドの説明として取得する
  @return 入力されたShell URI
*/
CString ShellUriCommand::GetDescription()
{
	return mDescription;
}

/**
  コマンド種別の表示名を取得する
  @return コマンド種別の表示名
*/
CString ShellUriCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

/**
  Shell URIを実行するアクションを作成する
  @param[in] hotkeyAttr ホットキー属性
  @param[out] action 作成したアクション
  @return true:アクションを作成した  false:作成しない
*/
bool ShellUriCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}

	// 実行時に候補の内容が変わっても、選択時点のURIを実行する。
	const CString uri = mDescription;
	*action = new CallbackAction(_T("ファイル名を指定して実行"), [uri](Parameter* param, String* errMsg) -> bool {
		SubProcess::ProcessPtr process;
		SubProcess exec(param);
		if (exec.Run(uri, process)) {
			return true;
		}
		if (errMsg) {
			*errMsg = _T("Shell URIを実行できませんでした");
		}
		return false;
	});
	return true;
}

/**
  Shell URIコマンドのアイコンを取得する
  @return コマンドのアイコン
*/
HICON ShellUriCommand::GetIcon()
{
	return IconLoader::Get()->GetShell32Icon(-328);
}

/**
  入力パターンがShell URIに一致するか判定する
  @param[in] pattern 入力パターン
  @return 一致度
*/
int ShellUriCommand::Match(Pattern* pattern)
{
	// shell:スキームだけを受け付け、URI本体には空白や制御文字を許可しない。
	static const launcherapp::utility::Regex uriRegex(
		_T("^shell:([^ \\t\\r\\n\\f\\v]+)$"),
		false
	);

	CString uri = pattern->GetWholeString();
	if (uriRegex.FullMatch(uri) == false) {
		return Pattern::Mismatch;
	}

	mDescription = uri;
	return Pattern::WholeMatch;
}

/**
  Shell URIコマンドを複製する
  @return 複製したコマンド
*/
launcherapp::core::Command* ShellUriCommand::Clone()
{
	return new ShellUriCommand();
}

/**
  コマンド種別の表示名を取得する
  @return コマンド種別の表示名
*/
CString ShellUriCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_PATHEXEC);
	return TEXT_TYPE;
}

} // shelluri
} // commands
} // launcherapp
