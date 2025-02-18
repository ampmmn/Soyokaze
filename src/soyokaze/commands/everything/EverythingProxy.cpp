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

	bool mIsUseAPI;
	bool mIsRunApp;
	CString mAppPath;
	HICON mAppIcon = nullptr;
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
			return false;
		}
	}
	if (Everything_Query(TRUE) == FALSE) {
		return false;
	}

	DWORD dwNumResults = Everything_GetNumResults();

	std::vector<EverythingResult> tmp;

	constexpr int LIMIT_TIME = 100;   // 結果取得にかける時間(これを超過したら打ち切り)
	uint64_t start = GetTickCount64();

	std::vector<TCHAR> path(MAX_PATH_NTFS);

	for (DWORD i = 0; i < dwNumResults; ++i) {

		if (Everything_GetResultFullPathName(i, path.data(), MAX_PATH_NTFS) == EVERYTHING_ERROR_INVALIDCALL) {
			break;
		}

		EverythingResult result;
		result.mFullPath = path.data();
		result.mMatchLevel = Pattern::FrontMatch;

		tmp.push_back(result);

		if (i == dwNumResults || (GetTickCount64() - start) >= LIMIT_TIME) {
			break;
		}
	}
	results.swap(tmp);
	return true;
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
	if (in->mIsUseAPI == false) {
		return;
	}

	if (in->QueryWithAPI(queryStr, results)) {
		return;
	}
}

bool EverythingProxy::IsUseAPI()
{
	return in->mIsUseAPI;
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

