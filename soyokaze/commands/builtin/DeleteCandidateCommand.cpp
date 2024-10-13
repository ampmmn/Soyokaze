#include "pch.h"
#include "framework.h"
#include "DeleteCandidateCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "commands/builtin/DeleteCommand.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using namespace launcherapp::core;
using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace builtin {

struct DeleteCandidateCommand::PImpl
{
	CString mCmdName;
};


DeleteCandidateCommand::DeleteCandidateCommand(
	const CString& cmdName
) : 
	AdhocCommandBase(cmdName, cmdName),
	in(std::make_unique<PImpl>())
{
	this->mName.Format(_T("delete %s"), (LPCTSTR)cmdName);
	this->mDescription.Format(_T("%s を削除します"), (LPCTSTR)cmdName);
	in->mCmdName = cmdName;
}

DeleteCandidateCommand::~DeleteCandidateCommand()
{
}

CString DeleteCandidateCommand::GetGuideString()
{
	return _T("Enter:コマンドを削除");
}

CString DeleteCandidateCommand::GetTypeDisplayName()
{
	return _T("システムコマンド");
}

BOOL DeleteCandidateCommand::Execute(Parameter* param)
{
	DeleteCommand cmd;
	RefPtr<CommandParameterBuilder> paramTmp(CommandParameterBuilder::Create(_T("delete")));
	paramTmp->AddArgument(in->mCmdName);
	return cmd.Execute(paramTmp.get());
}

HICON DeleteCandidateCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5383);
}

launcherapp::core::Command*
DeleteCandidateCommand::Clone()
{
	return new DeleteCandidateCommand(in->mCmdName);
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

