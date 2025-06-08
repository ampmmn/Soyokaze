#include "pch.h"
#include "UIAutomationCommandProvider.h"
#include "commands/uiautomation/WindowUIElements.h"
#include "commands/uiautomation/UIAutomationAdhocCommand.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "mainwindow/LauncherWindowEventListenerIF.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "SharedHwnd.h"
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace uiautomation {

/**
 * @brief 直前に前面にでていたウインドウハンドルを得る
 * @return ウインドウハンドルまたはNULL
 */
static HWND GetNextHwnd()
{
	SharedHwnd mainWnd;
	HWND hwndSelf = mainWnd.GetHwnd(); 
	HWND hwnd = hwndSelf;
	while (IsWindow(hwnd)) {

		TCHAR clsName[256];
		GetClassName(hwnd, clsName, 256);

		bool isProgman = false;
		if (_tcscmp(clsName, _T("Shell_TrayWnd")) == 0 || _tcscmp(clsName, _T("Progman")) == 0) {
			isProgman = true;
		}

		LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
		bool isMinimized = (style & WS_MINIMIZE) != 0;
		if (isProgman == false && hwnd != hwndSelf && IsWindowVisible(hwnd) && isMinimized == false) {
			break;
		}
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	}

	spdlog::debug("YMGW hwnd:{}",(void*)hwnd);
	return hwnd;
}

struct UIAutomationCommandProvider::PImpl :
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
		// ToDo: 設定を設ける
		//mIsEnableWorksheet = pref->IsEnableExcelWorksheet();
		//mPrefix = pref->GetWorksheetSwitchPrefix();
	}

// LauncherWindowEventListenerIF
	void OnLockScreenOccurred() override {}
	void OnUnlockScreenOccurred() override {}
	void OnTimer() override {}
	void OnLauncherActivate() override
	{
		std::thread th([&]() {

			// ランチャーのウインドウの背面にあるウインドウを取得する
			HWND hwnd = GetNextHwnd();
			if (IsWindow(hwnd) == FALSE) {
				return;
			}

			WindowUIElements windowUIElements(hwnd);
			std::vector<WindowUIElement> elems;
			windowUIElements.FetchElements(elems);

			std::lock_guard<std::mutex> lock(mMutex);
			mElements.swap(elems);
			mTargetWindow = hwnd;
		});
		th.detach();
	}
	void OnLauncherUnactivate() override
	{
	}


	void GetUIElements(HWND& hwnd, std::vector<WindowUIElement>& elems)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		hwnd = mTargetWindow;
		elems = mElements;
	}


	bool mIsEnable {true};
	bool mIsFirstCall {true};
	CString mPrefix{_T("ui")};

	HWND mTargetWindow{nullptr};
	std::vector<WindowUIElement> mElements;
	std::mutex mMutex;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(UIAutomationCommandProvider)

UIAutomationCommandProvider::UIAutomationCommandProvider() : in(std::make_unique<PImpl>())
{
}

UIAutomationCommandProvider::~UIAutomationCommandProvider()
{
}

CString UIAutomationCommandProvider::GetName()
{
	return _T("UIAutomationCommand");
}


// 一時的なコマンドを必要に応じて提供する
void UIAutomationCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	launcherapp::CommandQueryItemList& commands
)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		in->Reload();
		in->mIsFirstCall = false;
	}

	// 機能を利用しない場合は抜ける
	if (in->mIsEnable == false) {
		return ;
	}
	// プレフィックスが一致しない場合は抜ける
	const auto& prefix = in->mPrefix;
	if (prefix.IsEmpty() == FALSE && prefix.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return;
	}

	HWND hwnd{nullptr};
	std::vector<WindowUIElement> elems;
	in->GetUIElements(hwnd, elems);
	for (auto& elem : elems) {
		auto level = pattern->Match(elem.mName.c_str(), 1);
		if (level == Pattern::Mismatch) {
			continue;
		}

		if (prefix.IsEmpty() == FALSE && level == Pattern::PartialMatch) {
			// プレフィックスがある場合は少なくとも前方一致にする
			level = Pattern::FrontMatch;
		}

		commands.Add(CommandQueryItem(level, new UIAutomationAdhocCommand(hwnd, elem.mName.c_str(), elem.mRect, in->mPrefix)));
	}
}



}}} // end of namespace launcherapp::commands::uiautomation
