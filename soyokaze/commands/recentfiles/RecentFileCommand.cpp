#include "pch.h"
#include "framework.h"
#include "RecentFileCommand.h"
#include "IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace recentfiles {


struct RecentFileCommand::PImpl
{
	CString mFullPath;
};


RecentFileCommand::RecentFileCommand(const CString& name, const CString& fullPath) : 
	AdhocCommandBase(name, fullPath),
	in(new PImpl)
{
	in->mFullPath = fullPath;
}

RecentFileCommand::~RecentFileCommand()
{
}


CString RecentFileCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_RECENTFILES);
	return TEXT_TYPE;
}

BOOL RecentFileCommand::Execute(const Parameter& param)
{
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_NORMAL;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = in->mFullPath;
	si.lpParameters = nullptr;

	ShellExecuteEx(&si);
	CloseHandle(si.hProcess);

	return TRUE;
}

HICON RecentFileCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mFullPath);
}

soyokaze::core::Command*
RecentFileCommand::Clone()
{
	auto clonedObj = new RecentFileCommand(this->mName, in->mFullPath);
	return clonedObj;
}

} // end of namespace recentfiles
} // end of namespace commands
} // end of namespace soyokaze

