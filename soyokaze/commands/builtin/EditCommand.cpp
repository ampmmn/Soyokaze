#include "pch.h"
#include "framework.h"
#include "commands/builtin/EditCommand.h"
#include "core/CommandRepository.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {


CString EditCommand::TYPE(_T("Builtin"));

CString EditCommand::GetType()
{
	return TYPE;
}

EditCommand::EditCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("edit"))
{
	mDescription = _T("【編集】");
}

EditCommand::~EditCommand()
{
}

BOOL EditCommand::Execute(const Parameter& param)
{
	std::vector<CString> args;
	param.GetParameters(args);

	auto cmdRepoPtr =
	 	soyokaze::core::CommandRepository::GetInstance();

	if (args.empty()) {
		// キーワードマネージャを実行する
		cmdRepoPtr->ManagerDialog();
		return TRUE;
	}

	CString editName = args[0];
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
	return IconLoader::Get()->LoadDefaultIcon();
}

soyokaze::core::Command* EditCommand::Clone()
{
	return new EditCommand();
}

}
}
}
