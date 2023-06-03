#include "pch.h"
#include "framework.h"
#include "VersionCommand.h"
#include "gui/AboutDlg.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString VersionCommand::GetType() { return _T("Builtin-Version"); }

VersionCommand::VersionCommand(LPCTSTR name) : 
	mRefCount(1),
	mIsExecuting(false)
{
	mName = name ? name : _T("version");
}

VersionCommand::~VersionCommand()
{
}

CString VersionCommand::GetName()
{
	return mName;
}

CString VersionCommand::GetDescription()
{
	return _T("【バージョン情報】");
}

BOOL VersionCommand::Execute()
{
	if (mIsExecuting) {
		return TRUE;
	}

	CAboutDlg dlg;

	mIsExecuting = true;
	dlg.DoModal();
	mIsExecuting = false;
	return TRUE;
}

BOOL VersionCommand::Execute(const Parameter& param)
{
	// 引数指定しても動作はかわらない
	return Execute();
}

CString VersionCommand::GetErrorString()
{
	return _T("");
}

HICON VersionCommand::GetIcon()
{
	return IconLoader::Get()->LoadVersionIcon();
}


int VersionCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool VersionCommand::IsEditable()
{
	return false;
}

int VersionCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
VersionCommand::Clone()
{
	return new VersionCommand();
}

bool VersionCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
}

uint32_t VersionCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t VersionCommand::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

