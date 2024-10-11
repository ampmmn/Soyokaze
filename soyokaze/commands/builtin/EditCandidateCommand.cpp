#include "pch.h"
#include "framework.h"
#include "EditCandidateCommand.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace builtin {

struct EditCandidateCommand::PImpl
{
	CString mName;
};


EditCandidateCommand::EditCandidateCommand(
	const CString& displayName
) : 
	AdhocCommandBase(displayName, displayName),
	in(std::make_unique<PImpl>())
{
	in->mName = displayName;
}

EditCandidateCommand::~EditCandidateCommand()
{
}

CString EditCandidateCommand::GetGuideString()
{
	return _T("Enter:編集");
}

CString EditCandidateCommand::GetTypeDisplayName()
{
	return _T("システムコマンド");
}

BOOL EditCandidateCommand::Execute(Parameter* param)
{
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();

	auto cmd = cmdRepoPtr->QueryAsWholeMatch(in->mName);
	if (cmd == nullptr) {
		CString msgStr((LPCTSTR)IDS_ERR_NAMEDOESNOTEXIST);
		msgStr += _T("\n\n");
		msgStr += in->mName;
		AfxMessageBox(msgStr);
		return TRUE;
	}
	cmd->Release();

	cmdRepoPtr->EditCommandDialog(in->mName);
	return TRUE;
}

HICON EditCandidateCommand::GetIcon()
{
	return IconLoader::Get()->LoadEditIcon();
}

launcherapp::core::Command*
EditCandidateCommand::Clone()
{
	return new EditCandidateCommand(in->mName);
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

