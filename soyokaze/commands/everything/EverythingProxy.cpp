#include "pch.h"
#include "EverythingProxy.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "commands/everything/Everything-SDK/include/Everything.h"
#include "matcher/Pattern.h"
#include "icon/IconLoader.h"
#include "utility/ScopeAttachThreadInput.h"
#include "mainwindow/MainWindowDeactivateBlocker.h"
#include "SharedHwnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace everything {

struct EverythingProxy::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
		Load();
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
		mIsUseAPI = pref->IsUseEverythingAPI();
		mIsUseWM = pref->IsUseEverythingViaWM();
		mIsRunApp = pref->IsRunEverythingApp();
		mAppPath = pref->GetEverythingExePath();
	}
	void OnAppExit() override
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	bool IsEverythingActive();
	bool RunApp();
	bool QueryWithAPI(const CString& queryStr, std::vector<EverythingResult>& results);
	bool QueryWithWM(const CString& queryStr);
	bool ShowEverythingMainWindow(bool isActivateEvWnd);

	HWND GetAppMainWindow();

	bool mIsUseAPI;
	bool mIsUseWM;
	bool mIsRunApp;
	CString mAppPath;
	HICON mAppIcon = nullptr;
	int mLastMethod = -1;
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

bool EverythingProxy::PImpl::QueryWithAPI(const CString& queryStr, std::vector<EverythingResult>& results)
{
	Everything_SetSearch(queryStr);

	// Everythingが起動していない
	if (IsEverythingActive() == false) {
		if (RunApp() == false) {
			mLastMethod  = -1;  // Err
			return false;
		}
	}
	if (Everything_Query(TRUE) == FALSE) {
		mLastMethod  = -1;  // Err
		return false;
	}

	DWORD dwNumResults = Everything_GetNumResults();

	std::vector<EverythingResult> tmp;

	constexpr int LIMIT_TIME = 100;   // 結果取得にかける時間(これを超過したら打ち切り)
	uint64_t start = GetTickCount64();

	for (DWORD i = 0; i < dwNumResults; ++i) {

		TCHAR path[MAX_PATH_NTFS];
		if (Everything_GetResultFullPathName(i, path, MAX_PATH_NTFS) == EVERYTHING_ERROR_INVALIDCALL) {
			break;
		}

		EverythingResult result;
		result.mFullPath = path;
		result.mMatchLevel = Pattern::FrontMatch;

		tmp.push_back(result);

		if (i == dwNumResults || (GetTickCount64() - start) >= LIMIT_TIME) {
			break;
		}
	}
	results.swap(tmp);

	mLastMethod  = 0;  // API
	return true;
}

bool EverythingProxy::PImpl::QueryWithWM(const CString& queryStr)
{
	// Everythingのメインウインドウを探す
	HWND hEverything = GetAppMainWindow();
	if (IsWindow(hEverything) == FALSE) {
		mLastMethod  = -1;  // Err
		return false;
	}

	struct local {
		static BOOL CALLBACK OnChildWindows(HWND hwnd, LPARAM lp)
		{
			HWND* ph = (HWND*)lp;
			TCHAR clsName[64];
			GetClassName(hwnd, clsName, 64);
			if (_tcscmp(clsName, _T("Edit")) != 0) {
				return TRUE;
			}

			*ph = hwnd;
			return FALSE;
		}
	};

	// 入力欄
	HWND hEditCtrl = nullptr;
	EnumChildWindows(hEverything, local::OnChildWindows, (LPARAM)&hEditCtrl);
	if (hEditCtrl == nullptr || IsWindow(hEditCtrl) == FALSE) {
		spdlog::debug(_T("QueryWithWM: can't find edit."));
		mLastMethod  = -1;  // Err
		return false;
	}

	SendMessage(hEditCtrl, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)queryStr);

	mLastMethod  = 1;  // WM

	return true;
}

bool EverythingProxy::PImpl::ShowEverythingMainWindow(bool isActivateEvWnd)
{
	// メインウインドウを取得できない場合はタスクバー経由で表示させる
	HWND notifyWnd = FindWindow(_T("EVERYTHING_TASKBAR_NOTIFICATION"), NULL);
	if (IsWindow(notifyWnd) == false) {
		return false;
	}

	// Everythingをアクティブにすることにより、メインウインドウはフォーカスを失う。
	// アプリ設定によってはフォーカスを失うことにより非表示になってしまうため、それを一時的に阻害する
	MainWindowDeactivateBlocker blocker;

	// アクティブにする
	WPARAM wp = (WPARAM)MAKEWPARAM(40007, 0);
	LPARAM lp = (LPARAM)0;
	SendMessage(notifyWnd, WM_COMMAND, wp, lp);

	// 直後にEverything側にウインドウが移るため、元に戻す
	if (isActivateEvWnd == false) {
		SharedHwnd mainWnd;

		HWND h = mainWnd.GetHwnd();
		SetForegroundWindow(h);
	}
	else {
		ScopeAttachThreadInput input;
		HWND hEverything = FindWindow(_T("EVERYTHING"), NULL);
		if (IsWindow(hEverything)) {
			SetForegroundWindow(hEverything);
		}
	}

	return true;
}

HWND EverythingProxy::PImpl::GetAppMainWindow()
{
	HWND hEverything = FindWindow(_T("EVERYTHING"), NULL);
	if (IsWindow(hEverything) == FALSE) {

		// メインウインドウを取得できない場合はタスクバー経由で表示させる
		if (ShowEverythingMainWindow(false) == false) {
			// タスクバーが取得できない場合は起動できていないため起動する
			if (RunApp() == false) {
				return false;
			}
		}
	}

	// アクティブ後に改めて探す
	if (IsWindow(hEverything) == FALSE) {
		hEverything = FindWindow(_T("EVERYTHING"), NULL);
		if (IsWindow(hEverything) == FALSE) {
			return nullptr;
		}
	}

	LONG_PTR style = GetWindowLongPtr(hEverything, GWL_STYLE);
	if (style & WS_MINIMIZE) {
		// メインウインドウが最小化されているときは表示
		ShowEverythingMainWindow(false);
	}

	return hEverything;
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

void EverythingProxy::Query(const CString& queryStr, std::vector<EverythingResult>& results)
{
	if (in->mIsUseAPI) {
		if (in->QueryWithAPI(queryStr, results)) {
			return;
		}
	}
	if (in->mIsUseWM) {
		// WM経由の検索結果はEverything本体の方に表示するのでresultsを渡さない
		in->QueryWithWM(queryStr);
	}
}

bool EverythingProxy::ActivateMainWindow()
{
	return in->ShowEverythingMainWindow(true);
}

bool EverythingProxy::IsUseAPI()
{
	return in->mIsUseAPI;
}

bool EverythingProxy::IsUseWM()
{
	return in->mIsUseWM;
}

int EverythingProxy::GetLastMethod()
{
	return in->mLastMethod;
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

