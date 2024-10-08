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
	mDescription = _T("【コマンドを削除】");
}

DeleteCommand::DeleteCommand(const DeleteCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

DeleteCommand::~DeleteCommand()
{
}

BOOL DeleteCommand::Execute(Parameter* param)
{
	if (param->HasParameter() == false) {
		AfxMessageBox(IDS_ERR_NODELETECOMMAND);
		return TRUE;
	}

	LPCTSTR delName = param->GetParam(0);

	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();

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

HICON DeleteCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5383);
}

launcherapp::core::Command* DeleteCommand::Clone()
{
	return new DeleteCommand(*this);
}

}
}
}
