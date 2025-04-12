#include "pch.h"
#include "DemotedProcessToken.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/**
 * @brief デフォルトコンストラクタ
 */
DemotedProcessToken::DemotedProcessToken() : 
	mShProcTokenHandle(nullptr),
	mPrimaryToken(nullptr),
	mShellProcessHandle(nullptr)
{
}

/**
 * @brief デストラクタ
 */
DemotedProcessToken::~DemotedProcessToken()
{
	// ハンドルを閉じる
	CloseHandle(mShProcTokenHandle);
	CloseHandle(mPrimaryToken);
	CloseHandle(mShellProcessHandle);
}

/**
 * @brief プライマリトークンを取得する
 * @return プライマリトークンのハンドル、失敗した場合は nullptr
 */
HANDLE DemotedProcessToken::FetchPrimaryToken()
{
	if (mPrimaryToken) {
		return mPrimaryToken;
	}	

	// このプロセスでのSeIncreaseQuotaPrivilegeを有効にする。(権限昇格していない場合は機能しない)
	HANDLE token = nullptr;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token) == FALSE) {
		spdlog::error("OpenProcessToken failed: {}", GetLastError());
		return nullptr;
	}

	// トークンの特権を調整
	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount = 1;
	LookupPrivilegeValueW(nullptr, SE_INCREASE_QUOTA_NAME, &tkp.Privileges[0].Luid);
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(token, FALSE, &tkp, 0, NULL, NULL);
	DWORD dwLastErr = GetLastError();
	CloseHandle(token);
	if (dwLastErr != ERROR_SUCCESS) {
		// 権限昇格していない場合はここでエラーになる
		spdlog::error("OpenProcessToken failed: {}", dwLastErr);
		return nullptr;
	}

	// デスクトップシェルウィンドウのハンドルを取得
	HWND hwnd = GetShellWindow();
	if (hwnd == nullptr) {
		spdlog::error("No desktop shell is present");
		return nullptr;
	}

	// デスクトップシェルプロセスのプロセスIDを取得
	DWORD dwPID = 0;
	GetWindowThreadProcessId(hwnd, &dwPID);
	if (dwPID == 0) {
		spdlog::error("Unable to get PID of desktop shell.");
		return nullptr;
	}

	// デスクトップシェルプロセスのハンドルを開く
	mShellProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPID);
	if (mShellProcessHandle == nullptr) {
		spdlog::error("Can't open desktop shell process:  {}", GetLastError());
		return nullptr;
	}

	// デスクトップシェルプロセスのトークンを取得
	BOOL ret = OpenProcessToken(mShellProcessHandle, TOKEN_DUPLICATE, &mShProcTokenHandle);
	if (ret == FALSE) {
		spdlog::error("Can't get process token of desktop shell: {}", GetLastError());
		return nullptr;
	}

	// トークンを複製してプライマリトークンを作成
	const DWORD dwTokenRights = TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID;
	ret = DuplicateTokenEx(mShProcTokenHandle, dwTokenRights, NULL, SecurityImpersonation, TokenPrimary, &mPrimaryToken);
	if (ret == FALSE) {
		spdlog::error("Can't get primary token: {} ", GetLastError());
		return nullptr;
	}
	return mPrimaryToken;
}


// 管理者権限で実行されているか?
bool DemotedProcessToken::IsRunningAsAdmin()
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
