#include "pch.h"
#include "framework.h"
#include "commands/builtin/EditCommand.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {


CString EditCommand::TYPE(_T("Builtin"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(EditCommand)

CString EditCommand::GetType()
{
	return TYPE;
}

EditCommand::EditCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("edit"))
{
	mDescription = _T("【編集】");
}

EditCommand::EditCommand(const EditCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

EditCommand::~EditCommand()
{
}

BOOL EditCommand::Execute(Parameter* param)
{
	auto cmdRepoPtr =
	 	launcherapp::core::CommandRepository::GetInstance();

	if (param->HasParameter() == false) {
		// キーワードマネージャを実行する
		cmdRepoPtr->ManagerDialog();
		return TRUE;
	}

	LPCTSTR editName = param->GetParam(0);
	auto cmd = cmdRepoPtr->QueryAsWholeMatch(editName);

	if (cmd == nullptr) {
		CString msgStr((LPCTSTR)IDS_ERR_NAMEDOESNOTEXIST);
		msgStr += _T("\n\n");
		msgStr += editName;
		AfxMessageBox(msgStr);
		return TRUE;
	}
	cmd->Release();

	cmdRepoPtr->EditCommandDialog(editName);
	return TRUE;

	return TRUE;
}

HICON EditCommand::GetIcon()
{
	return IconLoader::Get()->LoadEditIcon();
}

launcherapp::core::Command* EditCommand::Clone()
{
	return new EditCommand(*this);
}

}
}
}
