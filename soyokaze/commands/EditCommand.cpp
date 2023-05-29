#include "pch.h"
#include "framework.h"
#include "commands/EditCommand.h"
#include "core/CommandRepository.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

EditCommand::EditCommand() : mRefCount(1)
{
}

EditCommand::~EditCommand()
{
}

CString EditCommand::GetName()
{
	return _T("edit");
}

CString EditCommand::GetDescription()
{
	return _T("【編集】");
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
	if (cmdRepoPtr->QueryAsWholeMatch(editName) == nullptr) {
		CString msgStr((LPCTSTR)IDS_ERR_NAMEDOESNOTEXIST);
		msgStr += _T("\n\n");
		msgStr += editName;
		AfxMessageBox(msgStr);
		return TRUE;
	}

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

soyokaze::core::Command* EditCommand::Clone()
{
	return new EditCommand();
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

