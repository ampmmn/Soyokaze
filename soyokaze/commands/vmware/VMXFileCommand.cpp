#include "pch.h"
#include "framework.h"
#include "VMXFileCommand.h"
#include "commands/common/SubProcess.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
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
	std::vector<TCHAR> pattern(MAX_PATH_NTFS);
	_tcscpy_s(pattern.data(), pattern.size(), mFullPath);
	PathRemoveFileSpec(pattern.data());

	PathAppend(pattern.data(), _T("*.*"));

	CString extLck(_T(".lck"));
	bool isLocked = false;

	CFileFind f;
	BOOL isLoop = f.FindFile(pattern.data(), 0);
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

BOOL VMXFileCommand::Execute(Parameter* param)
{
	in->mErrorMsg.Empty();

	// .lckファイルの確認
	if (in->IsLocked()) {
		CString msg;
		msg.Format(IDS_CONFIRM_VMXLOCKED, (LPCTSTR)this->mName);
		int sel = AfxMessageBox(msg, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
		if (sel != IDYES) {
			return TRUE;
		}
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

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool VMXFileCommand::IsPriorityRankEnabled()
{
	// コマンド登録順序と候補表示の順序を維持するため、重みづけを無効化する
	return false;
}

launcherapp::core::Command*
VMXFileCommand::Clone()
{
	return new VMXFileCommand(this->mName, in->mFullPath);
}

} // end of namespace vmware
} // end of namespace commands
} // end of namespace launcherapp

