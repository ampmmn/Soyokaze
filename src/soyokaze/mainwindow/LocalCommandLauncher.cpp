#include "pch.h"
#include "LocalCommandLauncher.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "utility/ProcessPath.h"
#include "resource.h"
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace mainwindow {

using CommandRepository = launcherapp::core::CommandRepository;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;


LocalCommandLauncher::LocalCommandLauncher()
{
}

LocalCommandLauncher::~LocalCommandLauncher()
{
}

// 読み込み
bool LocalCommandLauncher::Load()
{
	return CommandRepository::GetInstance()->Load() != FALSE;
}

// 検索リクエスト実施
void LocalCommandLauncher::Query(const launcherapp::commands::core::CommandQueryRequest& req)
{
	CommandRepository::GetInstance()->Query(req);
}

// コマンド実行
bool LocalCommandLauncher::Execute(const CString& str)
{
	return CallExecute(str);
}

// ファイルがドロップされた
void LocalCommandLauncher::DropFiles(const std::vector<CString>& files)
{
	CallDropFiles(files);
}

// URLがドロップされた
void LocalCommandLauncher::DropURL(const CString& urlString)
{
	CallDropURL(urlString);
}

// ウインドウ
void LocalCommandLauncher::CaptureWindow(HWND hwnd)
{
	CallCaptureWindow(hwnd);
}

bool LocalCommandLauncher::CallExecute(const CString& str)
{
	SPDLOG_DEBUG(_T("args str:{}"), (LPCTSTR)str);

	auto commandParam = CommandParameterBuilder::Create(str);

	auto cmd = CommandRepository::GetInstance()->QueryAsWholeMatch(commandParam->GetCommandString(), true);
	if (cmd == nullptr) {
		SPDLOG_ERROR(_T("Command does not exist. name:{}"), (LPCTSTR)commandParam->GetCommandString());

		commandParam->Release();
		return false;
	}

	std::thread th([cmd, commandParam]() {
		cmd->Execute(commandParam);
		commandParam->Release();
		cmd->Release();
	});
	th.detach();

	return true;
}

void LocalCommandLauncher::CallDropFiles(const std::vector<CString>& files)
{
	CommandRepository::GetInstance()->RegisterCommandFromFiles(files);
}

void LocalCommandLauncher::CallDropURL(const CString& urlString)
{
	// URL登録
	auto param = CommandParameterBuilder::Create();
	param->SetNamedParamString(_T("TYPE"), _T("ShellExecCommand"));
	param->SetNamedParamString(_T("PATH"), urlString);

	CommandRepository::GetInstance()->NewCommandDialog(param);

	param->Release();
}

void LocalCommandLauncher::CallCaptureWindow(HWND hwnd)
{
	if (IsWindow(hwnd) == FALSE) {
		spdlog::warn("window handle is invalid.");
		return;
	}

	ProcessPath processPath(hwnd);

	// 自プロセスのウインドウなら何もしない
	if (GetCurrentProcessId() == processPath.GetProcessId()) {
		spdlog::warn("window {} is my own self.", (void*)hwnd);
		return ;
	}

	try {
		auto param = CommandParameterBuilder::Create();
		param->SetNamedParamString(_T("TYPE"), _T("ShellExecuteCommand"));
		param->SetNamedParamString(_T("COMMAND"), processPath.GetProcessName());
		param->SetNamedParamString(_T("PATH"), processPath.GetProcessPath());
		param->SetNamedParamString(_T("DESCRIPTION"), processPath.GetCaption());
		param->SetNamedParamString(_T("ARGUMENT"), processPath.GetCommandLine());

		CommandRepository::GetInstance()->NewCommandDialog(param);

		param->Release();
		return;
	}
	catch(ProcessPath::Exception& e) {
		CString errMsg((LPCTSTR)IDS_ERR_QUERYPROCESSINFO);
		CString pid;
		pid.Format(_T(" (PID:%d)"), e.GetPID());
		errMsg += pid;

		AfxMessageBox(errMsg);
		SPDLOG_ERROR((LPCTSTR)errMsg);
		return;
	}
}



}} // end of namespace launcherapp::mainwindow


