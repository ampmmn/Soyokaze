#include "pch.h"
#include "Win32MenuCommandProvider.h"
#include "commands/win32menu/Win32MenuElements.h"
#include "commands/win32menu/Win32MenuAdhocCommand.h"
#include "commands/win32menu/Win32MenuParam.h"
#include "commands/win32menu/UIElementAliasMap.h"
#include "commands/core/CommandRepository.h"
#include "commands/activate_window/WindowList.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "mainwindow/LauncherWindowEventListenerIF.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "core/IFIDDefine.h"
#include "matcher/PartialMatchPattern.h"
#include "SharedHwnd.h"
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using WindowList = launcherapp::commands::activate_window::WindowList;

namespace {

bool IsToplevelWindow(HWND hwnd)
{
	if (IsWindow(hwnd) == FALSE) {
		return false;
	}

	TCHAR clsName[256];
	GetClassName(hwnd, clsName, 256);
	if (_tcscmp(clsName, _T("Shell_TrayWnd")) == 0 || _tcscmp(clsName, _T("Shell_SecondaryTrayWnd")) == 0 || 
	    _tcscmp(clsName, _T("Progman")) == 0 || _tcscmp(clsName, _T("#32769")) == 0) {
		return false;
	}

	CRect rc;
	GetClientRect(hwnd, &rc);
	if (rc.Width() < 30 || rc.Height() < 30) {
		// 小さすぎ
		return false;
	}
	if (IsWindowVisible(hwnd) == FALSE) {
		// 不可視
		return false;
	}

	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	bool isMinimized = (style & WS_MINIMIZE) != 0;
	if (isMinimized) {
		return false;
	}

	LONG_PTR style_ex = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	if (style_ex & WS_EX_NOACTIVATE) {
		// 有効化しないウインドウ
		return false;
	}
			
	return true;
}

/**
 * @brief 直前に前面にでていたウインドウハンドルを得る
 * @return ウインドウハンドルまたはNULL
 */
HWND GetNextHwnd()
{
	SharedHwnd mainWnd;
	HWND hwndSelf = mainWnd.GetHwnd(); 
	HWND hwnd = FindWindow(nullptr, nullptr);
	while (IsWindow(hwnd)) {
		if (IsToplevelWindow(hwnd) && hwnd != hwndSelf) {
			break;
		}
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	}

	if (IsWindow(hwnd)) {
		TCHAR clsName[256];
		GetClassName(hwnd, clsName, 256);
		TCHAR caption[256];
		GetWindowText(hwnd, caption, 256);
		spdlog::debug(_T("UIAutomation: GetNextHwnd hwnd:{0}, text:{1}, class:{2}"),(void*)hwnd, caption, clsName);
	}
	else {
		spdlog::debug("UIAutomation: GetNextHwnd : window not found.");
	}

	return hwnd;
}

}

namespace launcherapp { namespace commands { namespace win32menu {

struct Win32MenuCommandProvider::PImpl :
 	public AppPreferenceListenerIF,
	public LauncherWindowEventListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
		LauncherWindowEventDispatcher::Get()->AddListener(this);
	}
	virtual ~PImpl()
	{
		LauncherWindowEventDispatcher::Get()->RemoveListener(this);
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Reload();
	}
	void OnAppExit() override {}

	void Reload()
	{
		auto pref = AppPreference::Get();
		auto& settings = const_cast<Settings&>(pref->GetSettings());
		mParam.Load(settings);
	}

// LauncherWindowEventListenerIF
	void OnLockScreenOccurred() override {}
	void OnUnlockScreenOccurred() override {}
	void OnTimer() override {}
	void OnLauncherActivate() override
	{
		if (mParam.mIsEnable == false) {
			spdlog::debug("Win32Menu feature is disabled.");
			return;
		}

		// 対象ウインドウを取得
		std::vector<HWND> targetWindows;
		GetTargetWindows(targetWindows);

		Win32MenuElements::Win32MenuElementList elems;
		for (auto hwnd : targetWindows) {

			if (IsWindow(hwnd) == FALSE) {
				continue;
			}

			TCHAR caption[64];
			GetWindowText(hwnd, caption, 64);

			PERFLOG(_T("FetchWin32MenuElements Start hwnd:{0}, title:{1}"), (void*)hwnd, caption);
			spdlog::stopwatch sw;

			Win32MenuElements windowUIElements;
			windowUIElements.FetchWin32MenuItems(hwnd, elems);

			PERFLOG("FetchElements End {0:.6f} s.", sw);
		}

		// Win32メニュー項目をとった時点で結果を反映する
		std::lock_guard<std::mutex> lock(mMutex);
		mElements = elems;
	}
	void OnLauncherUnactivate() override
	{
		UIElementAliasMap::GetInstance()->Update();
	}

	void GetTargetWindows(std::vector<HWND>& targetWindows)
	{
		if (mParam.mIsFindAllMenu == false) {
			HWND hwnd = GetNextHwnd();
			targetWindows.push_back(hwnd);
			return;
		}
		else {
			// 対象ウインドウを列挙
			WindowList wndList;
			wndList.EnumWindowHandles(targetWindows);
		}

	}

	void GetElements(Win32MenuElements::Win32MenuElementList& elems)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		elems.clear();
		for (auto& elem : mElements) {
			elems.push_back(elem);
		}
	}

	// プレフィックスがマッチするかどうかチェック
	bool CheckPrefix(Pattern* pattern, uint32_t& ignoreMask) {

		const auto& prefix = mParam.mPrefix;
		if (prefix.IsEmpty()) {
			// プレフィックスがない場合はチェック不要
			ignoreMask = 0;
			return true;
		}

		RefPtr<PatternInternal> pat2;
		if (pattern->QueryInterface(IFID_PATTERNINTERNAL, (void**)&pat2) == false) {
			spdlog::error("Failed to get IFID_PATTERNINTERNAL interface");
			return false;
		}

		std::vector<CString> words;
		CString queryStr;
		pat2->GetRawWords(words);
		size_t n = (std::min)(words.size(), (size_t)32);
		for (size_t i = 0; i < n; ++i) {
			// プレフィックスと一致する引数をマッチング対象外にする(任意の位置にあってよい、という点でプレフィックスではない気がするが)
			if (prefix.CompareNoCase(words[i]) == 0) {
				ignoreMask = (uint32_t)(1 << i);
				return true;
			}
		}
		return false;
	}


	CommandParam mParam;
	Win32MenuElements::Win32MenuElementList mElements;
	std::mutex mMutex;

	bool mIsFirstCall {true};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(Win32MenuCommandProvider)

Win32MenuCommandProvider::Win32MenuCommandProvider() : in(std::make_unique<PImpl>())
{
}

Win32MenuCommandProvider::~Win32MenuCommandProvider()
{
}

CString Win32MenuCommandProvider::GetName()
{
	return _T("Win32MenuCommand");
}


// 一時的なコマンドを必要に応じて提供する
void Win32MenuCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	launcherapp::CommandQueryItemList& commands
)
{
	ASSERT(pattern);

	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		in->Reload();
		in->mIsFirstCall = false;
	}

	// 機能を利用しない場合は抜ける
	if (in->mParam.mIsEnable == false) {
		return ;
	}

	// プレフィックスが一致しない場合は検索を実施しない
	uint32_t ignoreMask = 0;
	if (in->CheckPrefix(pattern, ignoreMask) == false) {
		// プレフィックス不一致
		return;
	}

	CString prefix = in->mParam.mPrefix;

	int matchCount = 0;

	HWND hwnd{nullptr};
	Win32MenuElements::Win32MenuElementList elems;
	in->GetElements(elems);
	for (auto& elem : elems) {

		CString name(elem.GetName());
		auto level = pattern->Match(name, ignoreMask);
		if (level == Pattern::Mismatch) {
			continue;
		}

		if (ignoreMask == 1 && level == Pattern::PartialMatch) {
			// プレフィックスがある場合は少なくとも前方一致にする
			level = Pattern::FrontMatch;
		}

		commands.Add(CommandQueryItem(level, new Win32MenuAdhocCommand(elem, prefix)));
		matchCount++;
	}

	if (matchCount == 0 && prefix.IsEmpty() == FALSE) {
		commands.Add(CommandQueryItem(Pattern::HiddenMatch, new Win32MenuAdhocCommand()));
	}
}



}}} // end of namespace launcherapp::commands::win32menu
