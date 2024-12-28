#include "pch.h"
#include "AppProcess.h"

static LPCTSTR PROCESS_MUTEX_NAME = _T("Global\\mutex_launcherapp_exist");

AppProcess::AppProcess() : m_hMutexRun(nullptr)
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
		CloseHandle(h);
		SPDLOG_DEBUG("isExists=true");
		return true;
	}

	m_hMutexRun = CreateMutex(NULL, TRUE, PROCESS_MUTEX_NAME);
	if (m_hMutexRun == NULL) {
		DWORD lastErr = GetLastError(); 
		spdlog::error(_T("Failed to create mutex. err:{0:x}"), lastErr);
		if (lastErr == ERROR_ACCESS_DENIED) {
			throw exception(_T("起動に失敗しました。\n管理者権限で既に起動されている可能性があります。"));
		}
		else {
			throw exception(_T("Failed to init."));
		}
	}

	return false;
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

