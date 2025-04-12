#include "pch.h"
#include "AppProcess.h"
#include "setting/AppPreference.h"
#include "utility/Path.h"
#include "utility/DemotedProcessToken.h"  // IsRunningAsAdmin

static LPCTSTR PROCESS_MUTEX_NAME = _T("Global\\mutex_launcherapp_exist");


AppProcess::AppProcess() : m_hMutexRun(nullptr), mIsFirstProcess(true)
{
}

AppProcess::~AppProcess()
{
	if (m_hMutexRun != nullptr) {
		CloseHandle(m_hMutexRun);
	}
}

bool AppProcess::IsExist()
{
	// 多重起動検知のための名前付きミューテックスの有無により、先行プロセスの有無を調べる
	HANDLE h = OpenMutex(MUTEX_ALL_ACCESS, FALSE, PROCESS_MUTEX_NAME);
	if (h != nullptr) {
		mIsFirstProcess = false;
		CloseHandle(h);
		SPDLOG_DEBUG("isExists=true");
		return true;
	}

	m_hMutexRun = CreateMutex(NULL, TRUE, PROCESS_MUTEX_NAME);
	if (m_hMutexRun == NULL) {
		if (GetLastError() == ERROR_ACCESS_DENIED) {
			mIsFirstProcess = false;
			SPDLOG_DEBUG("isExists=true");
			return true;
		}

		if (m_hMutexRun == NULL) {
			DWORD lastErr = GetLastError();
			spdlog::error(_T("Failed to create mutex. err:{0:x}"), lastErr);
			throw exception(_T("Failed to init."));
		}
	}

	return false;
}

/**
  管理者権限で再起動が必要なら再起動する
 	@return true:実行中のプロセスを終了する(再起動をするため)  false:プロセスを終了しない
*/
bool AppProcess::RebootAsAdminIfNeeded()
{
	// 管理者権限として再起動するかどうかの判断をする
	bool isRunAsAdmin =DemotedProcessToken::IsRunningAsAdmin();
	if (isRunAsAdmin) {
		// すでに管理者権限で動いている
		return false;
	}
	if (AppPreference::Get()->ShouldRunAsAdmin() == false) {
		// 管理者権限で実行しない設定で起動された
		return false;
	}
	if (mIsFirstProcess == false) {
		// 後続プロセスとしての実行である
		return false;
	}

// 管理者権限で起動するため、自身を再起動する

	// 再起動防止のミューテックスを解除
	if (m_hMutexRun != nullptr) {
		CloseHandle(m_hMutexRun);
		m_hMutexRun = nullptr;
	}

	Path pathSelf(Path::MODULEFILEPATH);

	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_SHOW;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = pathSelf;
	si.lpVerb = _T("runas");

	ShellExecuteEx(&si);

	if (si.hProcess) {
		CloseHandle(si.hProcess);
	}

	// UACの確認画面で「いいえ」を選択した場合、ShellExecuteExの結果はfalseになるが、
	// その場合、アプリとしては起動しない(このプロセスを終了する)ので、
	// アプリの起動結果にかかわらず、trueを返す。
	return true;
}

// テスト用にオブジェクト名を上書きする
void AppProcess::SetSyncObjectName(LPCTSTR name)
{
	PROCESS_MUTEX_NAME = name;
}

HANDLE AppProcess::GetHandle()
{
	return m_hMutexRun;
}

