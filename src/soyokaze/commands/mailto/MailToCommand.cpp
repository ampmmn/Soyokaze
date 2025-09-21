#include "pch.h"
#include "MailToCommand.h"
#include "commands/common/SubProcess.h"
#include "actions/core/ActionParameter.h"
#include "actions/builtin/CallbackAction.h"
#include "utility/Path.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace mailto {

using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;
using SubProcess = launcherapp::commands::common::SubProcess;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(MailToCommand)

MailToCommand::MailToCommand() : 
	AdhocCommandBase(_T("mailto:"), _T("あて先を指定してメール"))
{
}

MailToCommand::~MailToCommand()
{
}

CString MailToCommand::GetGuideString()
{
	return _T("⏎:開く");
}

CString MailToCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool MailToCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	UNREFERENCED_PARAMETER(modifierFlags);

	*action = new CallbackAction(_T("あて先を指定してメール"), [&](Parameter* param, String*) -> bool {

		CString recipient;

		CString str = param->GetCommandString();
		int n = str.Find(_T("mailto:"));
		if (n != -1) {
			recipient = str.Mid(n + 7);
			recipient.Trim();
		}

		Path cmdExePath(Path::SYSTEMDIR, _T("cmd.exe"));

		SubProcess::ProcessPtr process;
		SubProcess exec(ParameterBuilder::EmptyParam());


		CString arg = _T("/c start \"\" mailto:" + recipient);
		exec.SetShowType(SW_HIDE);
		exec.Run((LPCTSTR)cmdExePath, arg, process);
		return true;
	});

	return true;
}

HICON MailToCommand::GetIcon()
{
	HICON h =IconLoader::Get()->GetImageResIcon(-20);
	return h;
}

int MailToCommand::Match(Pattern* pattern)
{
	LPCTSTR keyword = _T("mailto:");

	// mailto:～ 以降に入力があった場合でも完全一致扱いと扱うため、
	// 別途比較する
	CString word = pattern->GetWholeString();
	if (word.Find(keyword) == 0) {
		return Pattern::WholeMatch;
	}

	return pattern->Match(keyword);
}

launcherapp::core::Command*
MailToCommand::Clone()
{
	return new MailToCommand();
}

CString MailToCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_MAILTO);
	return TEXT_TYPE;
}

} // end of namespace mailto
} // end of namespace commands
} // end of namespace launcherapp

