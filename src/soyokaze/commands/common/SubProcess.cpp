#include "pch.h"
#include "SubProcess.h"
#include "core/IFIDDefine.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"
#include "externaltool/webbrowser/ConfiguredBrowserEnvironment.h"
#include "actions/core/ActionParameter.h"
#include "utility/LastErrorString.h"
#include "utility/Path.h"
#include "utility/DemotedProcessToken.h"
#include "setting/AppPreference.h"
#include <map>
#include <servprov.h>
#include <shobjidl_core.h>
#include <winrt/Windows.ApplicationModel.Core.h>

#pragma comment(lib, "Mpr.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;
using ConfiguredBrowserEnvironment = launcherapp::externaltool::webbrowser::ConfiguredBrowserEnvironment;
using namespace launcherapp::actions::core;

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
	PImpl(Parameter* param) : mParam(param)
	{
	}

	bool CanRunAsAdmin(const CString& path);

	bool StartWithLowerPermissions(SHELLEXECUTEINFO si, ProcessPtr& process);
	bool Start(SHELLEXECUTEINFO si, ProcessPtr& process);

	bool SetupShellExecuteInfo(CString& path, CString& param, const CString& workDir, SHELLEXECUTEINFO& si);

	Parameter* mParam{nullptr};
	int mShowType{SW_SHOW};
	bool mIsRunAsAdmin{false};
	CString mWorkingDir;
	std::map<tstring, tstring> mAdditionalEnv;
};

// 管理者権限で実行可能なファイルタイプか?
bool SubProcess::PImpl::CanRunAsAdmin(const CString& path)
{
	CString ext = PathFindExtension(path);
	return ext.CompareNoCase(_T(".exe")) == 0 || ext.CompareNoCase(_T(".bat")) == 0;
}

bool SubProcess::PImpl::StartWithLowerPermissions(SHELLEXECUTEINFO si, ProcessPtr& process)
{
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	bool isRun = proxy->StartProcess(&si, mAdditionalEnv);

	process = std::move(std::make_unique<Instance>(si.hProcess));

	return isRun;
}

bool SubProcess::PImpl::Start(SHELLEXECUTEINFO si, ProcessPtr& process)
{
	// 追加の環境変数が設定されているか
	auto site = winrt::make_self<AdditionalEnvVariableSite>();
	site->SetEnvironmentVariables(mAdditionalEnv);
	if (mAdditionalEnv.empty() == false) {
		si.fMask |= SEE_MASK_FLAG_HINST_IS_SITE;
    si.hInstApp = reinterpret_cast<HINSTANCE>(site.get());
	}

	// 管理者として実行する指定がされているか?
	bool isRunAsAdminSpecified = mIsRunAsAdmin && CanRunAsAdmin(si.lpFile);
	if (IsRunningAsAdmin() == false && isRunAsAdminSpecified) {
		si.lpVerb = _T("runas");
	}



	BOOL isRun = ShellExecuteEx(&si);

	process = std::move(std::make_unique<SubProcess::Instance>(si.hProcess));

	return isRun != FALSE;
}

bool SubProcess::PImpl::SetupShellExecuteInfo(CString& path, CString& param, const CString& workDir, SHELLEXECUTEINFO& si)
{
	// Webブラウザ(外部ツール)経由で起動すべきかどうかを判断する
	auto brwsEnv = ConfiguredBrowserEnvironment::GetInstance();
	if (brwsEnv->ShouldUseThisFor(path)) {

		CString url(path);

		// Webブラウザ経由
		CString browserPath;
		CString parameter;
		if (brwsEnv->GetInstalledExePath(browserPath) == false || brwsEnv->GetCommandlineParameter(parameter) == false) {
			// 無効なパスが設定されている
			return false;
		}

		// パスに空白を含む場合はダブルクォーテーションで囲む
		if (url.Find(_T(" ")) != -1) {
			url = _T("\"") + url + _T("\"");
		}

		// 置換後のパラメータを引数paramに書き戻す
		param = parameter;
		param.Replace(_T("$target"), url);

		si.cbSize = sizeof(si);
		si.nShow = mShowType;
		si.fMask = SEE_MASK_NOCLOSEPROCESS;

		// ConfiguredBrowserEnvironmentから得たWebブラウザ(外部ツール)のパスを引数pathに戻す
		path = browserPath;
		si.lpFile = path.GetBuffer(path.GetLength() + 1);

		si.lpParameters = param;

		if (workDir.IsEmpty() == FALSE) {
			si.lpDirectory = workDir;
		}
		return true;
	}
	else {
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
		return true;
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


SubProcess::SubProcess(Parameter* param) : 
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

	if (PathIsUNC(path) && Path::FileExists(path) == false) {
		// パスがUNC形式で、かつ、接続できないときは
		// ネットワーク接続先ホストへの認証が未実施の可能性があるため、認証を試みる

		NETRESOURCE nr = { 0 };
		nr.dwType = RESOURCETYPE_DISK;
		CString buf(path);
		nr.lpRemoteName = buf.GetBuffer();
		DWORD result = WNetAddConnection2(&nr, NULL, NULL, CONNECT_INTERACTIVE);
		buf.ReleaseBuffer();
		if (result != NO_ERROR) {
			LastErrorString errStr(result);
			process = std::move(std::make_unique<SubProcess::Instance>(nullptr));
			process->SetErrorMessage((LPCTSTR)errStr);
			return false;
		}
	}

	// ディレクトリの場合はファイラで経由でパスを表示する形に差し替える
	if (Path::IsDirectory(path)) {

		bool isFilerAvailable = false;

		if (pref->IsUseFiler()) {
			// ファイラ経由でパスを表示する形に差し替える
			paramStr = pref->GetFilerParam();
			paramStr.Replace(_T("$target"), path);

			auto filerPath = pref->GetFilerPath();
			ExpandArguments(filerPath, args);
			ExpandMacros(filerPath);

			isFilerAvailable = Path::FileExists(filerPath);
			if (isFilerAvailable) {
				path = filerPath;
			}
			else {
				// ファイラーが見つからない旨をログにだす
				spdlog::warn(_T("Failed to locate the specified file manager. {}"), (LPCTSTR)filerPath);
			}
		}

		if (isFilerAvailable == false) {
			// 登録されたファイラーがない、または、利用できない場合はエクスプローラで開く
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
	bool isRunAsAdminSpecified = in->mIsRunAsAdmin && in->CanRunAsAdmin(path);

	SHELLEXECUTEINFO si = {};
	in->SetupShellExecuteInfo(path, paramStr, workDir, si);

	bool isRun = false;
	if (IsRunningAsAdmin() && isRunAsAdminSpecified == false && pref->ShouldDemotePriviledge()) {
		// ランチャーを管理者権限で実行していて、かつ、コマンドを管理者権限で起動しない場合は、
		// 降格した権限で起動する
		isRun = in->StartWithLowerPermissions(si, process);
	}
	else {
		isRun = in->Start(si, process);
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

