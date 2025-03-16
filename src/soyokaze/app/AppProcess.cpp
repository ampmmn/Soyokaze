#include "pch.h"
#include "AppProcess.h"
#include "setting/AppPreference.h"
#include "utility/Path.h"

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

// 管理者権限で実行されているか?
static bool IsRunningAsAdmin()
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


// 管理者権限で再起動が必要なら再起動する
bool AppProcess::RebootAsAdminIfNeeded()
{
	// 管理者権限として再起動するかどうかの判断をする
	bool isRunAsAdmin = IsRunningAsAdmin();
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

	BOOL isRun = ShellExecuteEx(&si);

	if (si.hProcess) {
		CloseHandle(si.hProcess);
	}

	return isRun != FALSE;
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

