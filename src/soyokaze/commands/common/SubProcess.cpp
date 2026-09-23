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
#include <shlwapi.h>
#include <winrt/Windows.ApplicationModel.Core.h>

#pragma comment(lib, "shlwapi.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;
using ConfiguredBrowserEnvironment = launcherapp::externaltool::webbrowser::ConfiguredBrowserEnvironment;
using namespace launcherapp::actions::core;
using json = nlohmann::json;

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
	bool PrepareRunAsAdminTarget(CString& path, CString& param);
	bool QueryAssociationString(ASSOCSTR assocStr, const CString& assocKey, CString& value);
	bool GetAssociationKey(const CString& path, CString& assocKey);
	bool IsDirectExecutable(const CString& path);
	bool ParseAssociationCommand(const CString& command, CString& path, CString& param);
	void ReplaceAssociationParameter(CString& param, LPCTSTR name, const CString& value, bool isUrl);

	bool StartWithLowerPermissions(SHELLEXECUTEINFO si, ProcessPtr& process);
	bool Start(SHELLEXECUTEINFO si, ProcessPtr& process);

	bool SetupShellExecuteInfo(CString& path, CString& param, const CString& workDir, SHELLEXECUTEINFO& si);

	Parameter* mParam{nullptr};
	int mShowType{SW_SHOW};
	bool mIsRunAsAdmin{false};
	bool mIsRunAsAdminTarget{false};
	CString mWorkingDir;
	std::map<tstring, tstring> mAdditionalEnv;
};

// 管理者権限で実行可能なファイルタイプか?
bool SubProcess::PImpl::CanRunAsAdmin(const CString& path)
{
	return IsDirectExecutable(path);
}

bool SubProcess::PImpl::GetAssociationKey(const CString& path, CString& assocKey)
{
	if (PathIsURL(path)) {
		int separatorPos = path.Find(_T(":"));
		if (separatorPos <= 0) {
			return false;
		}
		assocKey = path.Left(separatorPos);
		return true;
	}

	assocKey = PathFindExtension(path);
	return assocKey.IsEmpty() == FALSE;
}

bool SubProcess::PImpl::QueryAssociationString(ASSOCSTR assocStr, const CString& assocKey, CString& value)
{
	DWORD size = 256;
	std::vector<TCHAR> buffer(size);

	for (;;) {
		DWORD actualSize = size;
		HRESULT hr = AssocQueryString(
			ASSOCF_NONE,
			assocStr,
			assocKey,
			_T("open"),
			buffer.data(),
			&actualSize
		);
		if (hr == S_FALSE && actualSize > size) {
			size = actualSize;
			buffer.resize(size);
			continue;
		}
		if (FAILED(hr)) {
			return false;
		}

		value = buffer.data();
		return value.IsEmpty() == FALSE;
	}
}

bool SubProcess::PImpl::IsDirectExecutable(const CString& path)
{
	CString assocKey;
	if (GetAssociationKey(path, assocKey) == false) {
		return false;
	}

	CString value;
	if (QueryAssociationString(ASSOCSTR_EXECUTABLE, assocKey, value)) {
		value.Trim();
		if (value == _T("%1") || value == _T("\"%1\"")) {
			return true;
		}
	}

	// Windowsの環境によってASSOCSTR_EXECUTABLEが実行ファイルのパスを返す場合があるため、コマンドも確認する。
	if (QueryAssociationString(ASSOCSTR_COMMAND, assocKey, value) == false) {
		return false;
	}
	value.Trim();
	return value == _T("%1") || value == _T("\"%1\"") || value == _T("%1 %*") || value == _T("\"%1\" %*");
}

void SubProcess::PImpl::ReplaceAssociationParameter(CString& param, LPCTSTR name, const CString& value, bool isUrl)
{
	int pos = 0;
	while ((pos = param.Find(name, pos)) >= 0) {
		int nameLength = static_cast<int>(_tcslen(name));
		bool isQuoted = pos > 0 && pos + nameLength < param.GetLength() &&
			param[pos - 1] == _T('\"') && param[pos + nameLength] == _T('\"');
		int replacementPos = pos;
		if (isUrl && isQuoted) {
			// URLでは関連付けコマンドの引用符も引数に残さない。
			param.Delete(pos + nameLength, 1);
			param.Delete(pos - 1, 1);
			replacementPos--;
		}
		CString replacement(value);
		if (isUrl == false && isQuoted == false && _tcscmp(name, _T("%*")) != 0 && replacement.IsEmpty() == FALSE) {
			replacement = _T("\"") + replacement + _T("\"");
		}
		param.Delete(replacementPos, nameLength);
		param.Insert(replacementPos, replacement);
		pos = replacementPos + replacement.GetLength();
	}
}

bool SubProcess::PImpl::ParseAssociationCommand(const CString& command, CString& path, CString& param)
{
	CString commandLine(command);
	commandLine.Trim();
	if (commandLine.IsEmpty()) {
		return false;
	}

	if (commandLine[0] == _T('\"')) {
		int endQuote = commandLine.Find(_T('\"'), 1);
		if (endQuote < 0) {
			return false;
		}
		path = commandLine.Mid(1, endQuote - 1);
		param = commandLine.Mid(endQuote + 1);
	}
	else {
		int separator = commandLine.FindOneOf(_T(" \t"));
		if (separator < 0) {
			path = commandLine;
			param.Empty();
		}
		else {
			path = commandLine.Left(separator);
			param = commandLine.Mid(separator);
		}
	}
	param.TrimLeft();
	return path.IsEmpty() == FALSE;
}

bool SubProcess::PImpl::PrepareRunAsAdminTarget(CString& path, CString& param)
{
	mIsRunAsAdminTarget = false;
	if (CanRunAsAdmin(path)) {
		mIsRunAsAdminTarget = true;
		return true;
	}

	CString assocKey;
	CString command;
	CString associatedPath;
	CString associatedParam;
	if (GetAssociationKey(path, assocKey) == false ||
		QueryAssociationString(ASSOCSTR_COMMAND, assocKey, command) == false ||
		ParseAssociationCommand(command, associatedPath, associatedParam) == false) {
		return false;
	}

	bool isUrl = PathIsURL(path) != FALSE;
	ReplaceAssociationParameter(associatedParam, _T("%1"), path, isUrl);
	ReplaceAssociationParameter(associatedParam, _T("%L"), path, isUrl);
	ReplaceAssociationParameter(associatedParam, _T("%*"), param, false);
	path = associatedPath;
	param = associatedParam;
	mIsRunAsAdminTarget = true;
	return true;
}

bool SubProcess::PImpl::StartWithLowerPermissions(SHELLEXECUTEINFO si, ProcessPtr& process)
{
	const std::map<std::wstring, std::wstring>& envMap = mAdditionalEnv;

	std::string dst;

	json json_req;
	json_req["command"] = "shellexecute";
	json_req["show_type"] = (int)si.nShow;
	json_req["mask"] = si.fMask;
	json_req["file"] = UTF2UTF(CString(si.lpFile), dst);
	if (si.lpParameters) {
		json_req["parameters"] = UTF2UTF(CString(si.lpParameters), dst);
	}
	if (si.lpDirectory) {
		json_req["directory"] = UTF2UTF(CString(si.lpDirectory), dst);
	}

	std::map<std::string, std::string> env_map;
	std::string dst_key;
	std::string dst_val;
	for (const auto& item : envMap) {
		env_map[UTF2UTF(item.first, dst_key)] = UTF2UTF(item.second, dst_val);
	}
	json_req["environment"] = env_map;

	// リクエストを送信する
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();

	json json_res;
	if (proxy->SendRequest(json_req, json_res) == false) {
		process = std::move(std::make_unique<Instance>(nullptr));
		return false;
	}

	// 結果を取得する
	if (json_res.find("pid") == json_res.end()) {
		spdlog::error("unexpected response.");
		process = std::move(std::make_unique<Instance>(nullptr));
		return false;
	}

	int pid = json_res["pid"];
	si.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)pid);
	process = std::move(std::make_unique<Instance>(si.hProcess));

	return true;
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
	bool isRunAsAdminSpecified = mIsRunAsAdmin && mIsRunAsAdminTarget;
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
		path.ReleaseBuffer();
		// Note: lpFileにpathの内部バッファを設定しているので、path変数の中身をこの後変えてはいけない..

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

    // UNC パスに対する事前接続(WNetAddConnection2)は行わない
    // 到達できない場合は以降の ShellExecuteEx でエラーとなる

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
	bool isRunAsAdminSpecified = false;
	if (in->mIsRunAsAdmin) {
		isRunAsAdminSpecified = in->PrepareRunAsAdminTarget(path, paramStr);
	}

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

