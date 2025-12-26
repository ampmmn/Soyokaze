#include "pch.h"
#include "framework.h"
#include "EditCandidateCommand.h"
#include "commands/core/CommandRepository.h"
#include "actions/builtin/CallbackAction.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using CommandRepository = launcherapp::core::CommandRepository;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;

namespace launcherapp {
namespace commands {
namespace builtin {

struct EditCandidateCommand::PImpl
{
	CString mCmdName;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(EditCandidateCommand)

EditCandidateCommand::EditCandidateCommand(
	const CString& cmdName
) : 
	AdhocCommandBase(cmdName, cmdName),
	in(std::make_unique<PImpl>())
{
	this->mName.Format(_T("edit %s"), (LPCTSTR)cmdName);
	this->mDescription.Format(_T("%s の設定変更を行います"), (LPCTSTR)cmdName);
	in->mCmdName = cmdName;
}

EditCandidateCommand::~EditCandidateCommand()
{
}

CString EditCandidateCommand::GetTypeDisplayName()
{
	return _T("システムコマンド");
}

bool EditCandidateCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}

	*action = new CallbackAction(_T("編集"), [&](Parameter*, String* errMsg) -> bool {

		auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();

		RefPtr<launcherapp::core::Command> cmd(cmdRepoPtr->QueryAsWholeMatch(in->mCmdName));
		if (cmd == nullptr) {
			CString msgStr((LPCTSTR)IDS_ERR_NAMEDOESNOTEXIST);
			msgStr += _T("\n\n");
			msgStr += in->mCmdName;
			if (errMsg) {
				UTF2UTF(msgStr, *errMsg);
			}
			return false;
		}

		constexpr bool isClone = false;
		cmdRepoPtr->EditCommandDialog(in->mCmdName, isClone);
		return true;
	});
	return true;
}

HICON EditCandidateCommand::GetIcon()
{
	return IconLoader::Get()->LoadEditIcon();
}

launcherapp::core::Command*
EditCandidateCommand::Clone()
{
	return new EditCandidateCommand(in->mCmdName);
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

