#include "pch.h"
#include "framework.h"
#include "VMXFileCommand.h"
#include "core/IFIDDefine.h"
#include "commands/common/SubProcess.h"
#include "icon/IconLoader.h"
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
	bool Execute(Parameter* param, int showType);
	bool IsLocked();

	CString mName;
	CString mFullPath;
	CString mErrorMsg;
};

bool VMXFileCommand::PImpl::Execute(Parameter* param, int showType)
{
	mErrorMsg.Empty();

	// .lckファイルの確認
	if (IsLocked()) {
		CString msg;
		msg.Format(IDS_CONFIRM_VMXLOCKED, (LPCTSTR)this->mName);
		int sel = AfxMessageBox(msg, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
		if (sel != IDYES) {
			return TRUE;
		}
	}

	SubProcess::ProcessPtr process;
	SubProcess exec(param);
	exec.SetShowType(showType);
	if (exec.Run(mFullPath, process) == false) {
		mErrorMsg = process->GetErrorMessage();
		return false;
	}
	return true;
}

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

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(VMXFileCommand)


VMXFileCommand::VMXFileCommand(const CString& name, const CString& fullPath) : in(std::make_unique<PImpl>())
{
	in->mName = name;
	this->mName = name;
	this->mDescription = fullPath;
	in->mFullPath = fullPath;
}

VMXFileCommand::~VMXFileCommand()
{
}

CString VMXFileCommand::GetGuideString()
{
	return _T("⏎:ファイルを開く C-⏎:パスを開く");
}

CString VMXFileCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

BOOL VMXFileCommand::Execute(Parameter* param)
{
	return in->Execute(param, SW_SHOW) ? TRUE : FALSE;
}

CString VMXFileCommand::GetErrorString()
{
	return in->mErrorMsg;
}

HICON VMXFileCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mFullPath);
}

launcherapp::core::Command*
VMXFileCommand::Clone()
{
	return new VMXFileCommand(this->mName, in->mFullPath);
}

// メニューの項目数を取得する
int VMXFileCommand::GetMenuItemCount()
{
	return 3;
}

// メニューの表示名を取得する
bool VMXFileCommand::GetMenuItemName(int index, LPCWSTR* displayNamePtr)
{
	if (index == 0) {
		static LPCWSTR name = L"実行(&E)";
		*displayNamePtr= name;
		return true;
	}
	else if (index == 1) {
		static LPCWSTR name = L"最大化状態で実行(&X)";
		*displayNamePtr= name;
		return true;
	}
	else if (index == 2) {
		static LPCWSTR name = L"最小化状態で実行(&M)";
		*displayNamePtr= name;
		return true;
	}
	return false;
}

// メニュー選択時の処理を実行する
bool VMXFileCommand::SelectMenuItem(int index, launcherapp::core::CommandParameter* param)
{
	if (index < 0 || 2 < index) {
		return false;
	}

	if (index == 0) {
		return in->Execute(param, SW_SHOW) != FALSE;
	}
	else if (index == 1) {
		return in->Execute(param, SW_MAXIMIZE) != FALSE;
	}
	else {
		return in->Execute(param, SW_MINIMIZE) != FALSE;
	}
}

bool VMXFileCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ContextMenuSource*)this;
		return true;
	}
	return false;
}

CString VMXFileCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_VMXFILE);
	return TEXT_TYPE;
}


} // end of namespace vmware
} // end of namespace commands
} // end of namespace launcherapp

