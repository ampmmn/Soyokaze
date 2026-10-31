#include "pch.h"
#include "SubProcess.h"
#include "core/IFIDDefine.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"
#include "externaltool/webbrowser/ConfiguredBrowserEnvironment.h"
#include "actions/core/ActionParameter.h"
#include "utility/LastErrorString.h"
#include "utility/FilerParameter.h"
#include "utility/Path.h"
#include "utility/CommandLineArgument.h"
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

namespace {

// 環境変数名として 사용할ことができる文字か?
bool IsValidEnvironmentVariableName(const CString& name)
{
	if (name.IsEmpty()) {
		return false;
	}

	for (int i = 0; i < name.GetLength(); ++i) {
		TCHAR c = name[i];
		if (c < 0x20 || c == _T('=') || c == _T(' ') || c == _T('\t') ||
			c == _T('\r') || c == _T('\n') || c == _T('"') || c == _T('\'')) {
			return false;
		}
	}

	return true;
}

} // namespace



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
	bool IsCmdExecuted(const CString& path);

	bool StartWithLowerPermissions(SHELLEXECUTEINFO si, ProcessPtr& process);
	bool Start(SHELLEXECUTEINFO si, ProcessPtr& process);

	bool SetupShellExecuteInfo(CString& path, CString& param, const CString& workDir, SHELLEXECUTEINFO& si);

	Parameter* mParam{nullptr};
	int mShowType{SW_SHOW};
	bool mIsRunAsAdmin{false};
	bool mIsRunAsAdminTarget{false};
	bool mIsExpanded{false};
	bool mIsExpandMacros{true};
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
		if (hr == S_FALSE) {
			// バッファが小さい場合は、必要なサイズまで拡張して再試行する
			if (actualSize <= size) {
				return false;
			}
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

		// 値をコマンドラインに組み込むため、引用符やバックスラッシュをエスケープする
		CString replacement;
		if (isUrl) {
			// URLは関連付けコマンドの引用符の内側にそのまま渡す
			replacement = launcherapp::utility::EscapeArgumentValue(value);
		}
		else if (isQuoted) {
			// 既に引用符が付いている場合は内側だけエスケープする
			replacement = launcherapp::utility::EscapeArgumentValue(value);
		}
		else if (_tcscmp(name, _T("%*")) != 0 && value.IsEmpty() == FALSE) {
			// 引用符が付いていない場合は、引用符を付けて値を渡す
			replacement = launcherapp::utility::QuoteArgument(value);
		}
		else {
			replacement = value;
		}

		param.Delete(replacementPos, nameLength);
		param.Insert(replacementPos, replacement);
		pos = replacementPos + replacement.GetLength();
	}
}

// cmd.exe経由で実行されるか?
bool SubProcess::PImpl::IsCmdExecuted(const CString& path)
{
	// 拡張子がバッチファイルの場合は、関連付け処理においてcmd.exe経由で実行される
	CString ext = PathFindExtension(path);
	ext.MakeLower();
	if (ext == _T(".bat") || ext == _T(".cmd") || ext == _T(".btm")) {
		return true;
	}

	// 実行ファイルがcmd.exeの場合も、cmd.exe経由で実行される
	CString filename = PathFindFileName(path);
	if (filename.CompareNoCase(_T("cmd.exe")) == 0) {
		return true;
	}

	return false;
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

	// 関連付けコマンドの実行ファイルが対象ファイル自身("%1")を指している場合は、
	// 対象ファイルに対して管理者権限での実行を要求する
	// (バッチファイルのように"cmd.exe /c "%1""ではなく""%1" %*"という
	//  関連付けになっている場合に対応する)
	if (associatedPath == _T("%1") || associatedPath == _T("%L")) {
		ReplaceAssociationParameter(associatedParam, _T("%*"), param, false);
		param = associatedParam;
		mIsRunAsAdminTarget = true;
		return true;
	}

	// 関連付けコマンドの実行ファイル部分にも%1が含まれる場合を考慮して置換する
	ReplaceAssociationParameter(associatedPath, _T("%1"), path, false);
	ReplaceAssociationParameter(associatedPath, _T("%L"), path, false);

	// 関連付けコマンドの引数部分を置換する
	bool hasAllParameter = associatedParam.Find(_T("%*")) >= 0;
	ReplaceAssociationParameter(associatedParam, _T("%1"), path, isUrl);
	ReplaceAssociationParameter(associatedParam, _T("%L"), path, isUrl);
	ReplaceAssociationParameter(associatedParam, _T("%*"), param, false);

	// 関連付けコマンドに%*が含まれない場合は、実行時引数を末尾に追加する
	if (hasAllParameter == false && param.IsEmpty() == FALSE) {
		if (associatedParam.IsEmpty() == FALSE) {
			associatedParam += _T(" ");
		}
		associatedParam += param;
	}

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

	DWORD pid = static_cast<DWORD>(json_res["pid"].get<int>());
	if (pid == 0xFFFFFFFF) {
		// 起動は成功しているがプロセスハンドルを取得できなかった場合は、待機なしで成功とする
		process = std::move(std::make_unique<Instance>(nullptr));
		return true;
	}

	si.hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (si.hProcess == nullptr) {
		spdlog::error("Failed to open the process. err:{}", GetLastError());
		process = std::move(std::make_unique<Instance>(nullptr));
		return false;
	}
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

		// 置換後のパラメータを引数paramに書き戻す
		// (URLはコマンドラインとして渡すため、引用符を付けて渡す)
		param = parameter;
		launcherapp::utility::ReplaceQuotedPlaceholder(param, _T("$target"), url);

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

void SubProcess::SetExpanded(bool expanded)
{
	in->mIsExpanded = expanded;
}

void SubProcess::SetExpandMacros(bool expandMacros)
{
	in->mIsExpandMacros = expandMacros;
}

// 追加登録する環境変数
bool SubProcess::SetAdditionalEnvironment(const CString& name, const CString& value)
{
	if (IsValidEnvironmentVariableName(name) == false) {
		spdlog::warn(_T("Invalid env name {}"), (LPCTSTR)name);
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
		process = std::make_unique<SubProcess::Instance>(nullptr);
		return false;
	}
	if (in->mParam == nullptr) {
		spdlog::error("Parameter is not set.");
		process = std::make_unique<SubProcess::Instance>(nullptr);
		return false;
	}

	// 失敗時にエラーメッセージを返せるようにしておく
	process = std::make_unique<SubProcess::Instance>(nullptr);

	CString path = path_;
	CString paramStr = paramStr_;

	// 与えられた実行時引数を配列にコピー
	int paramCount = in->mParam->GetParamCount();
	std::vector<CString> args;
	args.reserve(paramCount);
	for (int i = 0; i < paramCount; ++i) {
		args.push_back(in->mParam->GetParam(i));
	}

	// 変数を展開(パス・パラメータ・作業ディレクトリ)
	// (既に展開済みの場合は何もしない。
	//  展開済みの値には$nなどが含まれる可能性があるため、二重に展開してはいけない)
	CString workDir = in->mWorkingDir;
	if (in->mIsExpanded == false) {
		// パスおよび作業ディレクトリはパスであるためクォートしない
		ExpandArgumentsAndMacros(path, args, ArgumentQuoting::None, in->mIsExpandMacros);
		ExpandArgumentsAndMacros(workDir, args, ArgumentQuoting::None, in->mIsExpandMacros);
		// パラメータはコマンドラインであるため、常にクォートして展開する
		ExpandArgumentsAndMacros(paramStr, args, ArgumentQuoting::Always, in->mIsExpandMacros);
	}

	auto pref = AppPreference::Get();

    // UNC パスに対する事前接続(WNetAddConnection2)は行わない
    // 到達できない場合は以降の ShellExecuteEx でエラーとなる

	// ディレクトリの場合はファイラで経由でパスを表示する形に差し替える
	if (Path::IsDirectory(path)) {

		bool isFilerAvailable = false;

		if (pref->IsUseFiler()) {
			// ファイラ経由でパスを表示する形に差し替える
			paramStr = pref->GetFilerParam();
			launcherapp::utility::ExpandFilerParameter(paramStr, path);

			auto filerPath = pref->GetFilerPath();
			if (in->mIsExpanded == false) {
				ExpandArgumentsAndMacros(filerPath, args, ArgumentQuoting::None, in->mIsExpandMacros);
			}

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

	SPDLOG_DEBUG(_T("path:{} param:{}"), (LPCTSTR)path, (LPCTSTR)paramStr);

	if (workDir.IsEmpty() == FALSE) {
		StripDoubleQuate(workDir);
	}

	// 管理者として実行する指定がされているか?
	bool isRunAsAdminSpecified = false;
	if (in->mIsRunAsAdmin) {
		isRunAsAdminSpecified = in->PrepareRunAsAdminTarget(path, paramStr);
	}

	// cmd.exe経由で実行される場合、%NAME%形式の環境変数参照が含まれていると
	// 環境変数の展開によって別のコマンドが実行される可能性があるため、起動しない
	if (in->IsCmdExecuted(path) && launcherapp::utility::ContainsExpandableEnvReference(paramStr)) {
		spdlog::error(_T("The parameter contains an environment variable reference. {}"), (LPCTSTR)paramStr);
		process->SetErrorMessage(_T("環境変数を参照する引数は、バッチファイル経由で実行できません。"));
		return false;
	}

	SHELLEXECUTEINFO si = {};
	if (in->SetupShellExecuteInfo(path, paramStr, workDir, si) == false) {
		spdlog::error(_T("Failed to setup the shell execute info."));
		process->SetErrorMessage(_T("外部ツール(ブラウザ)の設定が不正です。"));
		return false;
	}

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

