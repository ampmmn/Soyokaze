#include "pch.h"
#include "SubProcess.h"
#include "commands/core/IFIDDefine.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/core/CommandParameter.h"
#include "utility/LastErrorString.h"
#include "utility/Path.h"
#include "setting/AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {

struct SubProcess::PImpl
{
	PImpl(launcherapp::core::CommandParameter* param) : mParam(param)
	{
	}

	bool IsRunAsKeyPressed();
	bool IsOpenPathKeyPressed();
	bool IsRunningAsAdmin();
	bool CanRunAsAdmin(const CString& path);

	CommandParameter* mParam = nullptr;
	int mShowType = SW_SHOW;
	bool mIsRunAdAdmin = false;
	CString mWorkingDir;
};

bool SubProcess::PImpl::IsRunAsKeyPressed()
{
	// Ctrl-Shiftキーが押されていたら
	uint32_t state = GetModifierKeyState(mParam, MASK_ALL);
	return state == (MASK_CTRL | MASK_SHIFT);
}

bool SubProcess::PImpl::IsOpenPathKeyPressed()
{
	auto pref = AppPreference::Get();
	if (pref->IsShowFolderIfCtrlKeyIsPressed() == false) {
		return false;
	}

	// Ctrlキーのみが押されていたら
	uint32_t state = GetModifierKeyState(mParam, MASK_ALL);
	return state == MASK_CTRL;
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

// 管理者権限で実行可能なファイルタイプか?
bool SubProcess::PImpl::CanRunAsAdmin(const CString& path)
{
	CString ext = PathFindExtension(path);
	return ext.CompareNoCase(_T(".exe")) == 0 || ext.CompareNoCase(_T(".bat")) == 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


SubProcess::SubProcess(CommandParameter* param) : 
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

	int paramCount = in->mParam->GetParamCount();
	std::vector<CString> args;
	args.reserve(paramCount);
	for (int i = 0; i < paramCount; ++i) {
		args.push_back(in->mParam->GetParam(i));
	}

	// 変数置換(パス)
	ExpandArguments(path, args);
	ExpandMacros(path);

	bool isDir = Path::IsDirectory(path);
	if ((in->IsOpenPathKeyPressed() && Path::FileExists(path)) || isDir) {
		auto pref = AppPreference::Get();
		if (pref->IsUseFiler()) {
			// ファイラ経由でパスを表示する形に差し替える
			paramStr = pref->GetFilerParam();
			paramStr.Replace(_T("$target"), path);

			path = pref->GetFilerPath();
			ExpandArguments(path, args);
			ExpandMacros(path);

		}
		else {
			// 登録されたファイラーがない場合はエクスプローラで開く
			if (isDir == FALSE) {
				PathRemoveFileSpec(path.GetBuffer(MAX_PATH_NTFS));
				path.ReleaseBuffer();
			}
			paramStr = _T("open");
		}
	}
	else { 
		// 変数置換(パラメータ)
		ExpandArguments(paramStr, args);
		ExpandMacros(paramStr);
	}

	SPDLOG_DEBUG(_T("path:{} param:{}"), (LPCTSTR)path, (LPCTSTR)paramStr);


	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = in->mShowType;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = path;

	bool isRunAsAdminSpecified = in->mIsRunAdAdmin || (in->IsRunAsKeyPressed() && in->CanRunAsAdmin(path) );
	if (isRunAsAdminSpecified && in->IsRunningAsAdmin() == false) {
		si.lpVerb = _T("runas");
	}

	if (paramStr.IsEmpty() == FALSE) {
		si.lpParameters = paramStr;
	}
	

	CString workDir = in->mWorkingDir;
	if (workDir.IsEmpty() == FALSE) {
		ExpandArguments(workDir, args);
		ExpandMacros(workDir);
		StripDoubleQuate(workDir);
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

