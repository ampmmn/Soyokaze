#include "pch.h"
#include "framework.h"
#include "ControlPanelCommand.h"
#include "IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace controlpanel {


struct ControlPanelCommand::PImpl
{
	CString mName;
	CString mIconPath;
	CString mAppName;
	CString mDescription;

	uint32_t mRefCount;
};


ControlPanelCommand::ControlPanelCommand(
	const CString& name,
	const CString& iconPath,
	const CString& appName,
	const CString& description
) : in(new PImpl)
{
	in->mRefCount = 1;
	in->mName = name;
	in->mIconPath = iconPath;
	in->mAppName = appName;
	in->mDescription = description.IsEmpty() ? name : description;
}

ControlPanelCommand::~ControlPanelCommand()
{
	delete in;
}

CString ControlPanelCommand::GetName()
{
	return in->mName;
}

CString ControlPanelCommand::GetDescription()
{
	return in->mDescription;
}

CString ControlPanelCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_CONTROLPANEL);
	return TEXT_TYPE;
}

BOOL ControlPanelCommand::Execute()
{
	Parameter param;
	return Execute(param);
}

BOOL ControlPanelCommand::Execute(const Parameter& param)
{
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_NORMAL;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = _T("control.exe");

	CString paramStr(_T("/name ") + in->mAppName);
	
	si.lpParameters = paramStr;

	ShellExecuteEx(&si);
	CloseHandle(si.hProcess);

	return TRUE;
}

CString ControlPanelCommand::GetErrorString()
{
	return _T("");
}

HICON ControlPanelCommand::GetIcon()
{
	if (in->mIconPath.IsEmpty()) {
		return IconLoader::Get()->LoadUnknownIcon();
	}
	return IconLoader::Get()->GetDefaultIcon(in->mIconPath);
}

int ControlPanelCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool ControlPanelCommand::IsEditable()
{
	return false;
}

int ControlPanelCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
ControlPanelCommand::Clone()
{
	return new ControlPanelCommand(in->mName, in->mIconPath, in->mAppName, in->mDescription);
}

bool ControlPanelCommand::Save(CommandFile* cmdFile)
{
	// 非サポート
	return false;
}

uint32_t ControlPanelCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t ControlPanelCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace controlpanel
} // end of namespace commands
} // end of namespace soyokaze

