#include "pch.h"
#include "framework.h"
#include "VMXFileCommand.h"
#include "IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace vmware {


struct VMXFileCommand::PImpl
{
	bool IsLocked();

	CString mName;
	CString mFullPath;
	CString mErrorMsg;
	uint32_t mRefCount;
};

bool VMXFileCommand::PImpl::IsLocked()
{
	TCHAR pattern[MAX_PATH_NTFS];
	_tcscpy_s(pattern, mFullPath);
	PathRemoveFileSpec(pattern);

	PathAppend(pattern, _T("*.*"));

	CString extLck(_T(".lck"));
	bool isLocked = false;

	CFileFind f;
	BOOL isLoop = f.FindFile(pattern, 0);
	while (isLoop) {
		isLoop = f.FindNextFile();
		if (f.IsDots()) {
			continue;
		}

		CString name(f.GetFileName());
		if (extLck.CompareNoCase(PathFindExtension(name)) == 0) {
			isLocked = true;
			break;
		}
	}

	f.Close();

	return isLocked;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



VMXFileCommand::VMXFileCommand(const CString& name, const CString& fullPath) : in(new PImpl)
{
	in->mName = name;
	in->mFullPath = fullPath;
	in->mRefCount = 1;
}

VMXFileCommand::~VMXFileCommand()
{
}

CString VMXFileCommand::GetName()
{
	return in->mName;
}

CString VMXFileCommand::GetDescription()
{
	return in->mFullPath;
}

CString VMXFileCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_VMXFILE);
	return TEXT_TYPE;
}

BOOL VMXFileCommand::Execute()
{
	Parameter emptyParams;
	return Execute(emptyParams);
}

BOOL VMXFileCommand::Execute(const Parameter& param)
{
	in->mErrorMsg.Empty();

	// .lckファイルの確認
	if (in->IsLocked()) {
		in->mErrorMsg.Format(IDS_ERR_VMXLOCKED, (LPCTSTR)in->mName);
		return FALSE;
	}

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

CString VMXFileCommand::GetErrorString()
{
	return in->mErrorMsg;
}

HICON VMXFileCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mFullPath);
}

int VMXFileCommand::Match(Pattern* pattern)
{
	return pattern->Match(in->mName);
}

bool VMXFileCommand::IsEditable()
{
	return false;
}

int VMXFileCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
VMXFileCommand::Clone()
{
	auto clonedObj = new VMXFileCommand(in->mName, in->mFullPath);
	return clonedObj;
}

bool VMXFileCommand::Save(CommandFile* cmdFile)
{
	// 非サポート
	return false;
}

uint32_t VMXFileCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t VMXFileCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace vmware
} // end of namespace commands
} // end of namespace soyokaze

