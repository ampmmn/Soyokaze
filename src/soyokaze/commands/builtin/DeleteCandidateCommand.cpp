#include "pch.h"
#include "framework.h"
#include "DeleteCandidateCommand.h"
#include "commands/core/CommandRepository.h"
#include "actions/core/ActionParameter.h"
#include "actions/builtin/CallbackAction.h"
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
using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;

namespace launcherapp {
namespace commands {
namespace builtin {

struct DeleteCandidateCommand::PImpl
{
	CString mCmdName;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(DeleteCandidateCommand)

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
	return _T("⏎:コマンドを削除");
}

CString DeleteCandidateCommand::GetTypeDisplayName()
{
	return _T("システムコマンド");
}

bool DeleteCandidateCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	*action = new CallbackAction(_T("コマンドを削除"), [&](Parameter*, String*) -> bool {
		DeleteCommand cmd;
		RefPtr<ParameterBuilder> paramTmp(ParameterBuilder::Create(_T("delete")));
		paramTmp->AddArgument(in->mCmdName);
		return cmd.Execute(paramTmp.get());
	});
	return true;
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

