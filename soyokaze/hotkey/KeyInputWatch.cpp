#include "pch.h"
#include "KeyInputWatch.h"
#include "hotkey/AppHotKey.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "SharedHwnd.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr UINT TIMERID_HOTKEY = 1;

struct KeyInputWatch::KEYEVENT
{
	UINT VKey;
	UINT Message;
};

struct KeyInputWatch::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override
	{
	}

	void OnAppNormalBoot() override {}

	void OnAppPreferenceUpdated() override
	{
		LoadSettings();
	}

	void OnAppExit() override
	{
	}

	bool UpdateKeyState(std::vector<KEYEVENT>& events, SHORT vk, SHORT& prevState)
	{
		SHORT curState = GetKeyState(vk);
		bool isPrevDown = (prevState & 0x8000);
		bool isCurDown = (curState & 0x8000);

		prevState = curState;

		if (isPrevDown == isCurDown) {
			// 変化なし
			return false;
		}

		KEYEVENT evt;
		evt.Message = isCurDown ? WM_KEYDOWN : WM_KEYUP;
		evt.VKey = vk;
		events.push_back(evt);
		return true;
	}

	void GetKeyEvents(std::vector<KEYEVENT>& events)
	{
		// 各修飾キーの状態をチェックする
		UpdateKeyState(events, VK_LSHIFT, mPrevStateLShift);
		UpdateKeyState(events, VK_RSHIFT, mPrevStateRShift);
		UpdateKeyState(events, VK_LCONTROL, mPrevStateLCtrl);
		UpdateKeyState(events, VK_RCONTROL, mPrevStateRCtrl);
		UpdateKeyState(events, VK_LWIN, mPrevStateLWin);
		UpdateKeyState(events, VK_RWIN, mPrevStateRWin);
		UpdateKeyState(events, VK_LMENU, mPrevStateLMenu);
		UpdateKeyState(events, VK_RMENU, mPrevStateRMenu);
	}

	void LoadSettings()
	{
		auto pref = AppPreference::Get();

		bool prevEnable = mIsEnableHotKey;
		mIsEnableHotKey = pref->IsEnableModifierHotKey();

		mFirstVK = pref->GetFirstModifierVirtualKeyCode();
		mSecondVK = pref->GetSecondModifierVirtualKeyCode();

		if (mHwnd == nullptr) {
			return;
		}

		if (prevEnable && mIsEnableHotKey == false) {
			KillTimer(mHwnd, TIMERID_HOTKEY);
		}
		else if (prevEnable && mIsEnableHotKey) {
			// なにもしない
		}
		else if (prevEnable == false && mIsEnableHotKey == false) {
			// なにもしない
		}
		else if (prevEnable == false && mIsEnableHotKey) {
			SetTimer(mHwnd, TIMERID_HOTKEY, 50, 0);
		}
	}

	HWND mHwnd = nullptr;
	UINT mPrevVK = 0;
	DWORD mPrevTime = 0;

	bool mIsEnableHotKey = false;
	SHORT mPrevStateLShift = 0;
	SHORT mPrevStateRShift = 0;
	SHORT mPrevStateLCtrl = 0;
	SHORT mPrevStateRCtrl = 0;
	SHORT mPrevStateLWin = 0;
	SHORT mPrevStateRWin = 0;
	SHORT mPrevStateLMenu = 0;
	SHORT mPrevStateRMenu = 0;

	UINT mFirstVK = 0;
	UINT mSecondVK = 0;
};



KeyInputWatch::KeyInputWatch() : in(new PImpl)
{
}

KeyInputWatch::~KeyInputWatch()
{
	if (IsWindow(in->mHwnd)) {
		DestroyWindow(in->mHwnd);
	}
}

LRESULT CALLBACK KeyInputWatch::OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg != WM_TIMER || wp != TIMERID_HOTKEY) {
		return DefWindowProc(hwnd, msg, wp, lp);
	}

	auto thisPtr = (KeyInputWatch*)(size_t)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	auto& in = thisPtr->in;

	std::vector<KEYEVENT> events;
	in->GetKeyEvents(events);

	for (auto& evt : events) {

		if (evt.Message != WM_KEYUP) {
			// あつかうのはKEYUPのみ
			continue;
		}

		UINT vk = evt.VKey;

		// アップされたキーが着目するキー(修飾キー)かどうかを判断する
		UINT curVK = 0;
		switch(vk) {
		case VK_SHIFT:
		case VK_LSHIFT:
		case VK_RSHIFT:
			curVK = VK_SHIFT;
			break;
		case VK_CONTROL:
		case VK_LCONTROL:
		case VK_RCONTROL:
			curVK = VK_CONTROL;
			break;
		case VK_LWIN:
		case VK_RWIN:
			curVK = VK_LWIN;
			break;
		case VK_MENU:
		case VK_LMENU:
		case VK_RMENU:
			curVK = VK_MENU;
			break;
		}

		if (curVK == 0) {
			// 着目しないキーの場合はスルー
			continue;
		}

		UINT prevVK = in->mPrevVK;

		DWORD time = GetTickCount();

		// 同一キー2回押し判定
		bool isDblPressed = (prevVK == curVK && (time - in->mPrevTime) < 350);
		// 同時押し判定(100msec以内であること)
		bool isSimulPressed = (prevVK != curVK && (time - in->mPrevTime) < 100);

		if (isDblPressed || isSimulPressed) {
			LPARAM vk = ((curVK << 16) & 0xFFFF0000) |(in->mPrevVK & 0xFFFF);

			// ここで設定したキーかどうかの判断をする
			bool isMatchNormalOrder = (in->mFirstVK == curVK && in->mSecondVK == prevVK);
			bool isMatchReverseOrder = (in->mFirstVK == prevVK && in->mSecondVK == curVK);
			if (isMatchNormalOrder == false && isMatchReverseOrder == false) {
				continue;
			}

			in->mPrevVK = 0;
			in->mPrevTime = 0;

			// アクティブなウインドウがリモートデスクトップの場合はホットキー通知しない
			HWND h = GetForegroundWindow();
			TCHAR clsName[64];
			GetClassName(h,clsName, 64);
			if (_tcscmp(clsName, _T("TscShellContainerClass")) == 0) {
				spdlog::debug(_T("mstsc is active. className:{}"), (LPCTSTR)clsName);
				break;
			}		

			SharedHwnd hwnd;
			PostMessage(hwnd.GetHwnd(), WM_APP+2, 0, 0);

			continue;
		}

		in->mPrevVK = curVK;
		in->mPrevTime = time;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}



bool KeyInputWatch::Create()
{
	CRect rc(0, 0, 0, 0);
	HINSTANCE hInst = AfxGetInstanceHandle();

	// 内部のmessage処理用の不可視のウインドウを作っておく
	HWND hwnd = CreateWindowEx(0, _T("STATIC"), _T("LncrModifierHotKey"), 0, 
	                           rc.left, rc.top, rc.Width(), rc.Height(),
	                           NULL, NULL, hInst, NULL);
	ASSERT(hwnd);

	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

	in->mHwnd = hwnd;

	in->LoadSettings();

	return true;
}



