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
	CString mName;
	CString mFullPath;
	uint32_t mRefCount;
};


RecentFileCommand::RecentFileCommand(const CString& name, const CString& fullPath) : in(new PImpl)
{
	in->mName = name;
	in->mFullPath = fullPath;
	in->mRefCount = 1;
}

RecentFileCommand::~RecentFileCommand()
{
}

CString RecentFileCommand::GetName()
{
	return in->mName;
}

CString RecentFileCommand::GetDescription()
{
	return in->mFullPath;
}

CString RecentFileCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_RECENTFILES);
	return TEXT_TYPE;
}

BOOL RecentFileCommand::Execute()
{
	Parameter emptyParams;
	return Execute(emptyParams);
}

BOOL RecentFileCommand::Execute(const Parameter& param)
{
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_NORMAL;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = in->mFullPath;;
	si.lpParameters = nullptr;

	ShellExecuteEx(&si);
	CloseHandle(si.hProcess);

	return TRUE;
}

CString RecentFileCommand::GetErrorString()
{
	return _T("");
}

HICON RecentFileCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mFullPath);
}

int RecentFileCommand::Match(Pattern* pattern)
{
	// サポートしない
	return Pattern::WholeMatch;
}

bool RecentFileCommand::IsEditable()
{
	return false;
}

int RecentFileCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
RecentFileCommand::Clone()
{
	auto clonedObj = new RecentFileCommand(in->mName, in->mFullPath);
	return clonedObj;
}

bool RecentFileCommand::Save(CommandFile* cmdFile)
{
	// 非サポート
	return false;
}

uint32_t RecentFileCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t RecentFileCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace recentfiles
} // end of namespace commands
} // end of namespace soyokaze

