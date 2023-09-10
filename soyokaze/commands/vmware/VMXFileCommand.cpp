#include "pch.h"
#include "framework.h"
#include "VMXFileCommand.h"
#include "commands/common/SubProcess.h"
#include "IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace soyokaze::commands::common;

namespace soyokaze {
namespace commands {
namespace vmware {


struct VMXFileCommand::PImpl
{
	bool IsLocked();

	CString mFullPath;
	CString mErrorMsg;
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



VMXFileCommand::VMXFileCommand(const CString& name, const CString& fullPath) : in(std::make_unique<PImpl>())
{
	this->mName = name;
	this->mDescription = fullPath;
	in->mFullPath = fullPath;
}

VMXFileCommand::~VMXFileCommand()
{
}

CString VMXFileCommand::GetGuideString()
{
	return _T("Enter:ファイルを開く Ctrl-Enter:パスを開く");
}

CString VMXFileCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_VMXFILE);
	return TEXT_TYPE;
}

BOOL VMXFileCommand::Execute(const Parameter& param)
{
	in->mErrorMsg.Empty();

	// .lckファイルの確認
	if (in->IsLocked()) {
		in->mErrorMsg.Format(IDS_ERR_VMXLOCKED, (LPCTSTR)this->mName);
		return FALSE;
	}

	SubProcess::ProcessPtr process;
	SubProcess exec(param);
	if (exec.Run(in->mFullPath, process) == false) {
		in->mErrorMsg = process->GetErrorMessage();
		return FALSE;
	}
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

soyokaze::core::Command*
VMXFileCommand::Clone()
{
	return new VMXFileCommand(this->mName, in->mFullPath);
}

} // end of namespace vmware
} // end of namespace commands
} // end of namespace soyokaze

