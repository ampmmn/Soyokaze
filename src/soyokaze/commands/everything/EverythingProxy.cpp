#include "pch.h"
#include "EverythingProxy.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#define EVERYTHINGUSERAPI
#include "commands/everything/Everything-SDK/include/Everything.h"
#include "matcher/Pattern.h"
#include "icon/IconLoader.h"
#include "utility/ScopeAttachThreadInput.h"
#include "mainwindow/MainWindowDeactivateBlocker.h"
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace everything {

constexpr DWORD REPLY_ID = 0xDEAD;
constexpr int LIMIT_TIME = 250;   // 結果取得にかける時間(これを超過したら打ち切り)

struct EverythingProxy::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
		Load();
		mQueryEvent = CreateEvent(nullptr, TRUE, TRUE, nullptr);
	}
	virtual ~PImpl()
	{
		CloseHandle(mQueryEvent);
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

	bool IsCanceled()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsCanceled;
	}
	bool WaitPreviousQueryDone()
	{
		if (WaitForSingleObject(mQueryEvent, 0) == WAIT_TIMEOUT) {
			std::lock_guard<std::mutex> lock(mMutex);
			mIsCanceled = true;
		}
		if (WaitForSingleObject(mQueryEvent, 2500) == WAIT_TIMEOUT) {
			return false;
		}
		return true;
	}

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
	bool mIsCanceled{false};
	CString mAppPath;
	HICON mAppIcon{nullptr};
	HWND mReceiverWindow{nullptr};
	HANDLE mQueryEvent{nullptr};
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

bool EverythingProxy::Query(const CString& queryStr, std::vector<EverythingResult>& results)
{
	if (in->WaitPreviousQueryDone() == false) {
		spdlog::error("Failed to wait for the previous query to complete.");
		return false;
	}

	ResetEvent(in->mQueryEvent);
	in->mIsCanceled = false;

	PERFLOG("EverythingProxy::Query Start");
	spdlog::stopwatch sw;

	Everything_SetSearch(queryStr);
	// 日付降順で得る
	Everything_SetSort(EVERYTHING_SORT_DATE_MODIFIED_DESCENDING);
	// 取得数を限定する
	Everything_SetMax(32);

	PERFLOG("EverythingProxy::Query Everything_SetSearch:{0:.6f} s.", sw); sw.reset();


	// Everythingが起動していない
	if (in->IsEverythingActive() == false) {
		if (in->RunApp() == false) {
			SetEvent(in->mQueryEvent);
			return false;
		}
	}
	PERFLOG("EverythingProxy::Query RunCheck:{0:.6f} s.", sw); sw.reset();

	in->mIsResultReceived = false;

	// クエリ結果を受け取るためのハンドルを設定する
	Everything_SetReplyWindow(in->GetReplyWindowHandle());
	Everything_SetReplyID(REPLY_ID);

 	if (Everything_Query(FALSE) == FALSE) {
		SetEvent(in->mQueryEvent);
		return false;
	}

	uint64_t start = GetTickCount64();

	MSG msg;
	while(in->IsCanceled() == false && in->mIsResultReceived == false && GetTickCount64() - start < LIMIT_TIME) {
		if (PeekMessage(&msg, nullptr, 0, 0, 0) == FALSE) {
			continue;
		}
		GetMessage(&msg, nullptr, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (in->IsCanceled() || in->mIsResultReceived == false) {
		SetEvent(in->mQueryEvent);
		return false;
	}


	DWORD dwNumResults = Everything_GetNumResults();

	PERFLOG("EverythingProxy::Query Everything_GetNumResults:{0:.6f} s.", sw); sw.reset();

	std::vector<EverythingResult> tmp;


	std::vector<TCHAR> path(MAX_PATH_NTFS);

	for (DWORD i = 0; i < dwNumResults; ++i) {

		if (Everything_GetResultFullPathName(i, path.data(), MAX_PATH_NTFS) == EVERYTHING_ERROR_INVALIDCALL) {
			break;
		}

		EverythingResult result;
		result.mFullPath = path.data();

		tmp.push_back(result);
	}
	results.swap(tmp);
	PERFLOG("EverythingProxy::Query GetResult:{0:.6f} s.", sw); sw.reset();

	SetEvent(in->mQueryEvent);
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

