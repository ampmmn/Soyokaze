#include "pch.h"
#include "MailToCommand.h"
#include "commands/common/SubProcess.h"
#include "commands/core/CommandParameter.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace mailto {

using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;
using SubProcess = launcherapp::commands::common::SubProcess;

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

BOOL MailToCommand::Execute(Parameter* param)
{
	CString recipient;

	CString str = param->GetCommandString();
	int n = str.Find(_T("mailto:"));
	if (n != -1) {
		recipient = str.Mid(n + 7);
		recipient.Trim();
	}

	SubProcess::ProcessPtr process;
	SubProcess exec(CommandParameterBuilder::EmptyParam());

	CString arg = _T("/c start \"\" mailto:" + recipient);
	exec.SetShowType(SW_HIDE);
	exec.Run(_T("cmd.exe"), arg, process);

	return TRUE;
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

