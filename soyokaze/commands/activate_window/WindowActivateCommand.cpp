#include "pch.h"
#include "framework.h"
#include "WindowActivateCommand.h"
#include "utility/ScopeAttachThreadInput.h"
#include "IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace activate_window {


struct WindowActivateCommand::PImpl
{
	CString mName;
	CString mDescription;
	HWND mHwnd;

	uint32_t mRefCount;
};


WindowActivateCommand::WindowActivateCommand(
	HWND hwnd
) : in(new PImpl)
{
	in->mRefCount = 1;
	in->mHwnd = hwnd;

	TCHAR caption[256];
	GetWindowText(hwnd, caption, 256);
	in->mName = caption;
	in->mDescription = CString("[Window]") + caption;
}

WindowActivateCommand::~WindowActivateCommand()
{
	delete in;
}

CString WindowActivateCommand::GetName()
{
	return in->mName;
}

CString WindowActivateCommand::GetDescription()
{
	return in->mDescription;
}

CString WindowActivateCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_WINDOWACTIVATE);
	return TEXT_TYPE;
}

BOOL WindowActivateCommand::Execute()
{
	Parameter param;
	return Execute(param);
}

BOOL WindowActivateCommand::Execute(const Parameter& param)
{
	ScopeAttachThreadInput scope;
	SetForegroundWindow(in->mHwnd);

	return TRUE;
}

CString WindowActivateCommand::GetErrorString()
{
	return _T("");
}

HICON WindowActivateCommand::GetIcon()
{
	HICON icon = (HICON)GetClassLongPtr(in->mHwnd, GCLP_HICON);
	if (icon) {
		return icon;
	}
	return (HICON)GetClassLongPtr(in->mHwnd, GCLP_HICONSM);
}

int WindowActivateCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool WindowActivateCommand::IsEditable()
{
	return false;
}

int WindowActivateCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
WindowActivateCommand::Clone()
{
	return new WindowActivateCommand(in->mHwnd);
}

bool WindowActivateCommand::Save(CommandFile* cmdFile)
{
	// 非サポート
	return false;
}

uint32_t WindowActivateCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t WindowActivateCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace activate_window
} // end of namespace commands
} // end of namespace soyokaze

