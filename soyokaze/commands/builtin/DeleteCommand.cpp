#include "pch.h"
#include "framework.h"
#include "commands/builtin/DeleteCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString DeleteCommand::TYPE(_T("Builtin-Delete"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(DeleteCommand)

CString DeleteCommand::GetType()
{
	return TYPE;
}

DeleteCommand::DeleteCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("delete"))
{
	mDescription = _T("【削除】");
}

DeleteCommand::~DeleteCommand()
{
}

BOOL DeleteCommand::Execute(const Parameter& param)
{
	// Ctrlキーがおされていた場合はカレントディレクトリをファイラで表示
	std::vector<CString> args;
	param.GetParameters(args);

	if (args.empty()) {
		AfxMessageBox(IDS_ERR_NODELETECOMMAND);
		return TRUE;
	}


	auto cmdRepoPtr =
	 	launcherapp::core::CommandRepository::GetInstance();

	auto delName = args[0];

	auto cmd = cmdRepoPtr->QueryAsWholeMatch(delName);
	if (cmd == nullptr) {
		CString msg;
		msg.Format(IDS_ERR_COMMANDDOESNOTEXIST);
		msg += _T("\n");
		msg+= delName;
		AfxMessageBox(msg);
		return TRUE;
	}

	// Note: 現状はシステムコマンド作成をサポートしていないので削除を許可しない
	if (cmd->IsEditable() == false) {
		return TRUE;
	}

	// 削除前の確認
	CString confirmMsg((LPCTSTR)IDS_CONFIRM_DELETE);
	confirmMsg += _T("\n");
	confirmMsg += _T("\n");
	confirmMsg += delName;

	int sel = AfxMessageBox(confirmMsg, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
	if (sel != IDYES) {
		return TRUE;
	}

	cmdRepoPtr->UnregisterCommand(cmd);
	return TRUE;
}

launcherapp::core::Command* DeleteCommand::Clone()
{
	return new DeleteCommand();
}

}
}
}
