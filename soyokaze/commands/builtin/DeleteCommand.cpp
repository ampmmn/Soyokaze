#include "pch.h"
#include "framework.h"
#include "commands/builtin/DeleteCommand.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "core/CommandRepository.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

CString DeleteCommand::GetType() { return _T("Builtin-Delete"); }

DeleteCommand::DeleteCommand(LPCTSTR name) : mRefCount(1)
{
	mName = name ? name : _T("delete");
}

DeleteCommand::~DeleteCommand()
{
}

CString DeleteCommand::GetName()
{
	return mName;
}

CString DeleteCommand::GetDescription()
{
	return _T("【削除】");
}

CString DeleteCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_BUILTIN);
	return TEXT_TYPE;
}

BOOL DeleteCommand::Execute()
{
	Parameter param;
	return Execute(param);
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
	 	soyokaze::core::CommandRepository::GetInstance();

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

	cmdRepoPtr->DeleteCommand(delName);
	return TRUE;
}

CString DeleteCommand::GetErrorString()
{
	return _T("");
}

HICON DeleteCommand::GetIcon()
{
	return IconLoader::Get()->LoadDefaultIcon();
}

int DeleteCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool DeleteCommand::IsEditable()
{
	return false;
}

int DeleteCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command* DeleteCommand::Clone()
{
	return new DeleteCommand();
}

bool DeleteCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
}

uint32_t DeleteCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t DeleteCommand::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

}
}
}
