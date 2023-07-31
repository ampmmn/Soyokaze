#include "pch.h"
#include "framework.h"
#include "commands/builtin/EditCommand.h"
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


CString EditCommand::GetType() { return _T("Builtin-Edit"); }

EditCommand::EditCommand(LPCTSTR name) : mRefCount(1)
{
	mName = name ? name : _T("edit");
}

EditCommand::~EditCommand()
{
}

CString EditCommand::GetName()
{
	return mName;
}

CString EditCommand::GetDescription()
{
	return _T("【編集】");
}

CString EditCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_BUILTIN);
	return TEXT_TYPE;
}

BOOL EditCommand::Execute()
{
	Parameter param;
	return Execute(param);
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

CString EditCommand::GetErrorString()
{
	return _T("");
}

HICON EditCommand::GetIcon()
{
	return IconLoader::Get()->LoadNewIcon();
}

int EditCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool EditCommand::IsEditable()
{
	return false;
}

int EditCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command* EditCommand::Clone()
{
	return new EditCommand();
}

bool EditCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
}

uint32_t EditCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t EditCommand::Release()
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
