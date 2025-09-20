#include "pch.h"
#include "EverythingProxy.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#define EVERYTHINGUSERAPI
#include "commands/everything/Everything-SDK/include/Everything.h"
#include "matcher/Pattern.h"
#include "icon/IconLoader.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/ManualEvent.h"
#include "utility/ScopedResetEvent.h"
#include "mainwindow/MainWindowDeactivateBlocker.h"
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace everything {

constexpr DWORD REPLY_ID = 0xDEAD;

struct EverythingProxy::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
		Load();
		mQueryEvent.Set();
	}
	virtual ~PImpl()
	{
	}

// AppPreferenceListenerIF
	void OnAppFirstBoot() override
	{
	}
	void OnAppNormalBoot() override
	{
	}

	void OnAppPreferenceUpdated() override
	{
		Load();
	}

	void Load()
	{
		auto pref = AppPreference::Get();
		mIsRunApp = pref->IsRunEverythingApp();
		mAppPath = pref->GetEverythingExePath();
	}
	void OnAppExit() override
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	bool IsEverythingActive();
	bool RunApp();


	// EverythingのQuery完了通知を受け取るためのウインドウを取得する(なければ作成する)
	HWND GetReplyWindowHandle()
 	{
		DWORD tid = GetCurrentThreadId();
		spdlog::debug("tid is {}", tid);

		if (mReceiverWindow) {
			return mReceiverWindow;
		}
		// 未作成の場合は作成する。
		HINSTANCE hInst = GetModuleHandle(nullptr);
		HWND hwnd = CreateWindowEx(0, _T("STATIC"), _T("LncrEveryingEventReceive"), 0, 
				                       0, 0, 1, 1,
				                       NULL, NULL, hInst, NULL);
		ASSERT(hwnd);

		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

		mReceiverWindow = hwnd;
		return hwnd;
	}
	static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	std::mutex mMutex;
	bool mIsRunApp{false};
	bool mIsResultReceived{false};
	CString mAppPath;
	HICON mAppIcon{nullptr};
	HWND mReceiverWindow{nullptr};
	ManualEvent mQueryEvent;
};


bool EverythingProxy::PImpl::IsEverythingActive()
{
	return Everything_GetMajorVersion() != 0;
}

bool EverythingProxy::PImpl::RunApp()
{
	if (mIsRunApp == false) {
		return false;
	}

	CString param(_T("-minimized"));

	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(SHELLEXECUTEINFO);
	si.lpFile = mAppPath;
	si.lpParameters = param;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	if (ShellExecuteEx(&si)) {
		if (si.hProcess != nullptr) {
			CloseHandle(si.hProcess);
		}
		return true;
	}
	return false;
}


/**
 * @brief ウィンドウプロシージャ
 * @param hwnd ウィンドウハンドル
 * @param msg メッセージ
 * @param wparam WPARAM
 * @param lparam LPARAM
 * @return メッセージの処理結果
 */
LRESULT CALLBACK EverythingProxy::PImpl::OnWindowProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
)
{
	if (Everything_IsQueryReply(msg, wparam, lparam, REPLY_ID) == false) {
		return DefWindowProc(hwnd, msg, wparam ,lparam);
	}

	// 受け取った旨のフラグを立てる
	auto thisPtr = (EverythingProxy::PImpl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	thisPtr->mIsResultReceived = true;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


 
EverythingProxy::EverythingProxy() : in(new PImpl)
{
}

EverythingProxy::~EverythingProxy()
{
}

EverythingProxy* EverythingProxy::Get()
{
	static EverythingProxy inst;
	return &inst;
}

/**
  Everything検索を実行する
 	@return true:成功 false:失敗
 	@param[in]  queryStr    検索ワード
 	@param[in]  cancelToken キャンセル
 	@param[out] results     県が
*/
bool EverythingProxy::Query(
	const CString& queryStr,
	CancellationToken* cancelToken,
	std::vector<EverythingResult>& results
)
{
	// ## 前回のクエリの完了を待機する
	if (in->mQueryEvent.WaitFor(2500) == false) {
		spdlog::error("Failed to wait for the previous query to complete.");
		return false;
	}
	ScopedResetEvent scoped_event(in->mQueryEvent, true);  // スコープを抜けるときにSetEventを呼ぶ

	PERFLOG("EverythingProxy::Query Start");
	spdlog::stopwatch sw;

	// ## Everythingが起動していない場合は起動する
	if (in->IsEverythingActive() == false) {
		if (in->RunApp() == false) {
			return false;
		}
	}
	PERFLOG("EverythingProxy::Query RunCheck:{0:.6f} s.", sw); sw.reset();

	// ## 検索条件の設定
	// 検索文言
	Everything_SetSearch(queryStr);
	// 日付降順で得る
	Everything_SetSort(EVERYTHING_SORT_DATE_MODIFIED_DESCENDING);
	// 取得数を限定する
	Everything_SetMax(32);

	PERFLOG("EverythingProxy::Query Everything_SetSearch:{0:.6f} s.", sw); sw.reset();

	// ## クエリ結果を受け取るための準備
	in->mIsResultReceived = false;
	Everything_SetReplyWindow(in->GetReplyWindowHandle());
	Everything_SetReplyID(REPLY_ID);

	// ## Everything検索を実行する
 	if (Everything_Query(FALSE) == FALSE) {
		return false;
	}

	// ## 検索完了を待つ
	MSG msg;
	while(in->mIsResultReceived == false) {

		if (cancelToken->IsCancellationRequested()) {
			// 検索キャンセルが発生した
			spdlog::debug("Everything_Query cancel.");
			return false;
		}

		// メッセージ処理
		if (PeekMessage(&msg, nullptr, 0, 0, 0) == FALSE) {
			continue;
		}
		GetMessage(&msg, nullptr, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// ## 検索結果を取り出す
	DWORD dwNumResults = Everything_GetNumResults();

	PERFLOG("EverythingProxy::Query Everything_GetNumResults:{0:.6f} s.", sw); sw.reset();

	std::vector<EverythingResult> tmp;

	std::vector<TCHAR> path(MAX_PATH_NTFS);

	for (DWORD i = 0; i < dwNumResults; ++i) {

		if (Everything_GetResultFullPathName(i, path.data(), (DWORD)path.size()) == EVERYTHING_ERROR_INVALIDCALL) {
			break;
		}
		tmp.emplace_back(path.data());
	}
	results.swap(tmp);
	PERFLOG("EverythingProxy::Query GetResult:{0:.6f} s.", sw); sw.reset();

	return true;
}

HICON EverythingProxy::GetIcon()
{
	if (in->mAppIcon) {
		return in->mAppIcon;
	}
	HWND hEverything = FindWindow(_T("EVERYTHING"), NULL);
	if (IsWindow(hEverything)) {
		auto loader = IconLoader::Get();
		HICON h = loader->LoadIconFromHwnd(hEverything);
		if (h != loader->LoadDefaultIcon()) {
			in->mAppIcon = h;
			return h;
		}
	}
	return IconLoader::Get()->GetImageResIcon(-5332);
}

} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

