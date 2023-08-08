#include "pch.h"
#include "MailToCommand.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace soyokaze {
namespace commands {
namespace mailto {


using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;


struct MailToCommand::PImpl
{
	uint32_t mRefCount;
};


MailToCommand::MailToCommand() : in(new PImpl)
{
	in->mRefCount = 1;
}

MailToCommand::~MailToCommand()
{
	delete in;
}

CString MailToCommand::GetName()
{
	return _T("mailto:");
}

CString MailToCommand::GetDescription()
{
	return _T("あて先を指定してメール");
}

CString MailToCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_MAILTO);
	return TEXT_TYPE;
}

BOOL MailToCommand::Execute()
{
	ShellExecCommand::ATTRIBUTE attr;
	attr.mPath = _T("cmd.exe");
	attr.mParam = _T("/c start \"\" mailto:");
	attr.mShowType = SW_HIDE;

	ShellExecCommand cmd;
	cmd.SetAttribute(attr);
	return cmd.Execute();
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
	return cmd.Execute();
}

CString MailToCommand::GetErrorString()
{
	return _T("");
}

HICON MailToCommand::GetIcon()
{
	HICON h =IconLoader::Get()->GetImageResIcon(15);
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

bool MailToCommand::IsEditable()
{
	return false;
}

int MailToCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
MailToCommand::Clone()
{
	return new MailToCommand();
}

bool MailToCommand::Save(CommandFile* cmdFile)
{
	// 非サポート
	return false;
}

uint32_t MailToCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t MailToCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace mailto
} // end of namespace commands
} // end of namespace soyokaze

