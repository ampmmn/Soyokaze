#include "pch.h"
#include "RunVMXAction.h"
#include "commands/common/SubProcess.h"
#include "resource.h"

namespace launcherapp { namespace actions { namespace vmware {


using namespace launcherapp::commands::common;

RunVMXAction::RunVMXAction(const CString& vmName, const CString& vmxFilePath, int showType) :
	mVMDisplayName(vmName), mVMXFilePath(vmxFilePath), mShowType(showType)
{
}

RunVMXAction::~RunVMXAction()
{
}

// Action
// アクションの内容を示す名称
CString RunVMXAction::GetDisplayName()
{
	return _T("VMを起動する");
}

// アクションを実行する
bool RunVMXAction::Perform(Parameter* param, String* errMsg)
{
	if (IsVMLocked()) {
		// ロックされている旨を表示
		CString msg;
		msg.Format(IDS_CONFIRM_VMXLOCKED, (LPCTSTR)this->mVMDisplayName);
		int sel = AfxMessageBox(msg, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
		if (sel != IDYES) {
			return true;
		}
	}

	SubProcess::ProcessPtr process;

	SubProcess exec(param);
	exec.SetShowType(mShowType);
	if (exec.Run(mVMXFilePath, process) == FALSE) {
		if (errMsg) {
			UTF2UTF(process->GetErrorMessage(), *errMsg);
		}
		return false;
	}
	return true;
}

// .lckファイルの確認
bool RunVMXAction::IsVMLocked()
{
	std::vector<TCHAR> pattern(MAX_PATH_NTFS);
	_tcscpy_s(pattern.data(), pattern.size(), mVMXFilePath);
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

}}}

