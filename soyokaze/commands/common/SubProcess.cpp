#include "pch.h"
#include "SubProcess.h"
#include "commands/common/ExpandFunctions.h"
#include "core/CommandParameter.h"
#include "utility/LastErrorString.h"
#include "AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace common {

struct SubProcess::PImpl
{
	PImpl(const soyokaze::core::CommandParameter& param) : mParam(param)
	{
	}

	bool IsRunAsKeyPressed();
	bool IsOpenPathKeyPressed();
	bool IsRunningAsAdmin();

	const CommandParameter& mParam;
	int mShowType = SW_SHOW;
	bool mIsRunAdAdmin = false;
	CString mWorkingDir;
};

bool SubProcess::PImpl::IsRunAsKeyPressed()
{
	// Ctrl-Shiftキーが押されていたら
	return mParam.GetNamedParamBool(_T("CtrlKeyPressed")) && 
	       mParam.GetNamedParamBool(_T("ShiftKeyPressed")) &&
	       mParam.GetNamedParamBool(_T("AltKeyPressed")) == false &&
	       mParam.GetNamedParamBool(_T("WinKeyPressed")) == false;
}

bool SubProcess::PImpl::IsOpenPathKeyPressed()
{
	auto pref = AppPreference::Get();
	if (pref->IsShowFolderIfCtrlKeyIsPressed() == false) {
		return false;
	}

	return mParam.GetNamedParamBool(_T("CtrlKeyPressed")) &&
	       mParam.GetNamedParamBool(_T("ShiftKeyPressed")) == false &&
	       mParam.GetNamedParamBool(_T("AltKeyPressed")) == false &&
	       mParam.GetNamedParamBool(_T("WinKeyPressed")) == false;
}

// 管理者権限で実行されているか?
bool SubProcess::PImpl::IsRunningAsAdmin()
{
	static bool isRunAsAdmin = []() {
		PSID grp;
		SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
		BOOL result = AllocateAndInitializeSid(&authority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &grp);
		if (result == FALSE) {
			return false;
		}

		BOOL isMember = FALSE;
		result = CheckTokenMembership(nullptr, grp, &isMember);
		FreeSid(grp);

		return result && isMember;
	}();
	return isRunAsAdmin;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


SubProcess::SubProcess(const CommandParameter& param) : 
	in(std::make_unique<PImpl>(param))
{
}

SubProcess::~SubProcess()
{
}

void SubProcess::SetShowType(int showType)
{
	in->mShowType = showType;
}

void SubProcess::SetRunAsAdmin()
{
	in->mIsRunAdAdmin = true;
}

void SubProcess::SetWorkDirectory(const CString& dir)
{
	in->mWorkingDir = dir;
}

bool SubProcess::Run(const CString& path, ProcessPtr& process)
{
	return Run(path, _T(""), process);
}

bool SubProcess::Run(
		const CString& path_,
	 	const CString& paramStr_,
	 	ProcessPtr& process
)
{
	CString path = path_;
	CString paramStr = paramStr_;

	std::vector<CString> args;
	in->mParam.GetParameters(args);

	// 変数置換(パス)
	ExpandArguments(path, args);
	ExpandEnv(path);
	ExpandClipboard(path);
	ExpandAfxCurrentDir(path);

	if ((in->IsOpenPathKeyPressed() && PathFileExists(path)) || PathIsDirectory(path)) {
		auto pref = AppPreference::Get();
		if (pref->IsUseFiler()) {
			// ファイラ経由でパスを表示する形に差し替える
			paramStr = pref->GetFilerParam();
			paramStr.Replace(_T("$target"), path);

			path = pref->GetFilerPath();
			ExpandArguments(path, args);
			ExpandEnv(path);
			ExpandClipboard(path);
			ExpandAfxCurrentDir(path);

		}
		else {
			// 登録されたファイラーがない場合はエクスプローラで開く
			if (PathIsDirectory(path) == FALSE) {
				PathRemoveFileSpec(path.GetBuffer(MAX_PATH_NTFS));
				path.ReleaseBuffer();
			}
			paramStr = _T("open");
		}
	}
	else { 
		// 変数置換(パラメータ)
		ExpandArguments(paramStr, args);
		ExpandEnv(path);
		ExpandClipboard(paramStr);
		ExpandAfxCurrentDir(paramStr);
	}

	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = in->mShowType;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = path;

	bool isRunAsAdminSpecified = (in->mIsRunAdAdmin || in->IsRunAsKeyPressed() );
	if (isRunAsAdminSpecified && in->IsRunningAsAdmin() == false) {
		si.lpVerb = _T("runas");
	}

	if (paramStr.IsEmpty() == FALSE) {
		si.lpParameters = paramStr;
	}
	

	CString workDir = in->mWorkingDir;
	if (workDir.IsEmpty() == FALSE) {
		ExpandAfxCurrentDir(workDir);
		si.lpDirectory = workDir;
	}

	BOOL bRun = ShellExecuteEx(&si);

	process = std::move(std::make_unique<Instance>(si));

	if (bRun == FALSE) {
		LastErrorString errStr(GetLastError());
		process->SetErrorMessage((LPCTSTR)errStr);
		return false;
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

SubProcess::Instance::Instance(SHELLEXECUTEINFO si) :
	mShellExecuteInfo(si)
{
}

SubProcess::Instance::~Instance()
{
	if (mShellExecuteInfo.hProcess) {
		CloseHandle(mShellExecuteInfo.hProcess);
	}
}

bool SubProcess::Instance::Wait(DWORD timeout)
{
	auto& si = mShellExecuteInfo;
	if (si.hProcess == nullptr) {
		return false;
	}
	return WaitForSingleObject(si.hProcess, timeout) == WAIT_OBJECT_0;
}

void SubProcess::Instance::SetErrorMessage(const CString& msg)
{
	mErrMsg = msg;
}

CString SubProcess::Instance::GetErrorMessage()
{
	return mErrMsg;
}

}
}
}

