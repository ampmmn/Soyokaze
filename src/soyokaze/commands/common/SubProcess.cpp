#include "pch.h"
#include "SubProcess.h"
#include "commands/core/IFIDDefine.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"
#include "commands/core/CommandParameter.h"
#include "utility/LastErrorString.h"
#include "utility/Path.h"
#include "utility/DemotedProcessToken.h"
#include "setting/AppPreference.h"
#include <map>
#include <servprov.h>
#include <shobjidl_core.h>
#include <winrt/Windows.ApplicationModel.Core.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

struct AdditionalEnvVariableSite : 
	winrt::implements<AdditionalEnvVariableSite, ::IServiceProvider, ::ICreatingProcess>
{
public:

	void SetEnvironmentVariables(const std::map<tstring, tstring>& vals) {
		mEnvMap = vals;
	}


	IFACEMETHOD(QueryService)(REFGUID service, REFIID riid, void** ppv) {
		if (service != SID_ExecuteCreatingProcess) {
			*ppv = nullptr;
			return E_NOTIMPL;
		}
		return this->QueryInterface(riid, ppv);
	}

	IFACEMETHOD(OnCreating)(ICreateProcessInputs* inputs) {

		for (auto& item : mEnvMap) {
			HRESULT hr = inputs->SetEnvironmentVariable(item.first.c_str(), item.second.c_str());
			if (hr != S_OK) {
				spdlog::error("SetEnvironmentVariable failed: {:x}", hr);
				break;
			}
		}

		return S_OK;
	}

private:
	std::map<tstring, tstring> mEnvMap;
};


namespace launcherapp {
namespace commands {
namespace common {



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct SubProcess::PImpl
{
	PImpl(launcherapp::core::CommandParameter* param) : mParam(param)
	{
	}

	bool IsRunAsKeyPressed();
	bool IsOpenPathKeyPressed();
	bool CanRunAsAdmin(const CString& path);

	bool StartWithLowerPermissions(CString& path, CString& param, const CString& workDir, ProcessPtr& process);
	bool Start(CString& path, CString& param, const CString& workDir, ProcessPtr& process);

	CommandParameter* mParam{nullptr};
	int mShowType{SW_SHOW};
	bool mIsRunAsAdmin{false};
	CString mWorkingDir;
	std::map<tstring, tstring> mAdditionalEnv;
};

bool SubProcess::PImpl::IsRunAsKeyPressed()
{
	// Ctrl-Shiftキーが押されていたら
	uint32_t state = GetModifierKeyState(mParam, MASK_ALL);
	return state == (MASK_CTRL | MASK_SHIFT);
}

bool SubProcess::PImpl::IsOpenPathKeyPressed()
{
	// Ctrlキーのみが押されていたら
	uint32_t state = GetModifierKeyState(mParam, MASK_ALL);
	return state == MASK_CTRL;
}


// 管理者権限で実行可能なファイルタイプか?
bool SubProcess::PImpl::CanRunAsAdmin(const CString& path)
{
	CString ext = PathFindExtension(path);
	return ext.CompareNoCase(_T(".exe")) == 0 || ext.CompareNoCase(_T(".bat")) == 0;
}

bool SubProcess::PImpl::StartWithLowerPermissions(CString& path, CString& param, const CString& workDir, ProcessPtr& process)
{
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = mShowType;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = path;
	if (param.IsEmpty() == FALSE) {
		si.lpParameters = param;
	}
	if (workDir.IsEmpty() == FALSE) {
		si.lpDirectory = workDir;
	}

	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	bool isRun = proxy->StartProcess(&si, mAdditionalEnv);

	process = std::move(std::make_unique<Instance>(si.hProcess));

	return isRun;
}

bool SubProcess::PImpl::Start(CString& path, CString& param, const CString& workDir, ProcessPtr& process)
{
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = mShowType;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = path;

	// 追加の環境変数が設定されているか
	auto site = winrt::make_self<AdditionalEnvVariableSite>();
	site->SetEnvironmentVariables(mAdditionalEnv);
	if (mAdditionalEnv.empty() == false) {
		si.fMask |= SEE_MASK_FLAG_HINST_IS_SITE;
    si.hInstApp = reinterpret_cast<HINSTANCE>(site.get());
	}

	// 管理者として実行する指定がされているか?
	bool isRunAsAdminSpecified = mIsRunAsAdmin || (IsRunAsKeyPressed() && CanRunAsAdmin(path) );
	if (IsRunningAsAdmin() == false && isRunAsAdminSpecified) {
		si.lpVerb = _T("runas");
	}

	if (param.IsEmpty() == FALSE) {
		si.lpParameters = param;
	}

	if (workDir.IsEmpty() == FALSE) {
		si.lpDirectory = workDir;
	}

	BOOL isRun = ShellExecuteEx(&si);

	process = std::move(std::make_unique<SubProcess::Instance>(si.hProcess));

	return isRun != FALSE;
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
	in->mIsRunAsAdmin = true;
}

void SubProcess::SetWorkDirectory(const CString& dir)
{
	in->mWorkingDir = dir;
}

// 追加登録する環境変数
bool SubProcess::SetAdditionalEnvironment(const CString& name, const CString& value)
{
	if (name.FindOneOf(_T(" =")) != -1) {
		spdlog::warn(_T("Invali env name {}"), (LPCTSTR)name);
		return false;
	}
	in->mAdditionalEnv[tstring(name)] = tstring(value);
	return true;
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
	if (path_.IsEmpty()) {
		return false;
	}

	CString path = path_;
	CString paramStr = paramStr_;

	// 与えられた実行時引数を配列にコピー
	int paramCount = in->mParam->GetParamCount();
	std::vector<CString> args;
	args.reserve(paramCount);
	for (int i = 0; i < paramCount; ++i) {
		args.push_back(in->mParam->GetParam(i));
	}

	// 変数を展開(パス)
	ExpandArguments(path, args);
	ExpandMacros(path);

	auto pref = AppPreference::Get();

	// 「パスを開く」指定の場合はファイラで経由でパスを表示する形に差し替える
	bool isDir = Path::IsDirectory(path);
	if ((in->IsOpenPathKeyPressed() && Path::FileExists(path)) || isDir) {
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

	// 作業ディレクトリ
	CString workDir = in->mWorkingDir;
	if (workDir.IsEmpty() == FALSE) {
		ExpandArguments(workDir, args);
		ExpandMacros(workDir);
		StripDoubleQuate(workDir);
	}

	// 管理者として実行する指定がされているか?
	bool isRunAsAdminSpecified = in->mIsRunAsAdmin || (in->IsRunAsKeyPressed() && in->CanRunAsAdmin(path));

	bool isRun = false;
	if (IsRunningAsAdmin() && isRunAsAdminSpecified == false && pref->ShouldDemotePriviledge()) {
		// ランチャーを管理者権限で実行していて、かつ、コマンドを管理者権限で起動しない場合は、
		// 降格した権限で起動する
		isRun = in->StartWithLowerPermissions(path, paramStr, workDir, process);
	}
	else {
		isRun = in->Start(path, paramStr, workDir, process);
	}

	if (isRun == false) {
		LastErrorString errStr(GetLastError());
		process->SetErrorMessage((LPCTSTR)errStr);
	}
	return isRun;
}

// 管理者権限で実行されているか?
bool SubProcess::IsRunningAsAdmin()
{
	return DemotedProcessToken::IsRunningAsAdmin();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct SubProcess::Instance::PImpl
{
	HANDLE mProcess{nullptr};
	CString mErrMsg;
};

SubProcess::Instance::Instance(HANDLE hProcess) : in(new PImpl)
{
	in->mProcess = hProcess;
}

SubProcess::Instance::~Instance()
{
	if (in->mProcess) {
		CloseHandle(in->mProcess);
	}
}

bool SubProcess::Instance::Wait(DWORD timeout)
{
	if (in->mProcess == nullptr) {
		return false;
	}
	return WaitForSingleObject(in->mProcess, timeout) == WAIT_OBJECT_0;
}

void SubProcess::Instance::SetErrorMessage(const CString& msg)
{
	in->mErrMsg = msg;
}

CString SubProcess::Instance::GetErrorMessage()
{
	return in->mErrMsg;
}

}
}
}

