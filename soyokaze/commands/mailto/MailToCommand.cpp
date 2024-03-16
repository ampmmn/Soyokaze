#include "pch.h"
#include "MailToCommand.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace soyokaze {
namespace commands {
namespace mailto {

using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;


MailToCommand::MailToCommand() : 
	AdhocCommandBase(_T("mailto:"), _T("あて先を指定してメール"))
{
}

MailToCommand::~MailToCommand()
{
}

CString MailToCommand::GetGuideString()
{
	return _T("Enter:開く");
}

CString MailToCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_MAILTO);
	return TEXT_TYPE;
}

BOOL MailToCommand::Execute(const Parameter& param)
{
	CString recipient;

	CString str = param.GetCommandString();
	int n = str.Find(_T("mailto:"));
	if (n != -1) {
		recipient = str.Mid(n + 7);
		recipient.Trim();
	}

	ShellExecCommand::ATTRIBUTE attr;
	attr.mPath = _T("cmd.exe");
	attr.mParam = _T("/c start \"\" mailto:" + recipient);
	attr.mShowType = SW_HIDE;

	ShellExecCommand cmd;
	cmd.SetAttribute(attr);

	Parameter paramEmpty;
	return cmd.Execute(paramEmpty);
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

soyokaze::core::Command*
MailToCommand::Clone()
{
	return new MailToCommand();
}

} // end of namespace mailto
} // end of namespace commands
} // end of namespace soyokaze

