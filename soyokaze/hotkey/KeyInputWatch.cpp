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

static int OTHER_KEYS[] = {
	// Backspace,tab, スペースキー..
	0x08, 0x09, 0x20, 0x21, 0x22, 0x23, 0x24, 0x2d, 0x2e,
	// 数字キー0-9
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39,
	// アルファベットキーA-Z
 	0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
	0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e,
	0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
	0x57, 0x58, 0x59, 0x5a,
	// NUMキー
 	0x60, 0x61, 0x62, 0x63,
	0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
	0x6c, 0x6d, 0x6e, 0x6f,
	// ファンクションキーF1-F24
 	0x70, 0x71, 0x72, 0x73,
	0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b,
	0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83,
	0x84, 0x85, 0x86, 0x87,
	// OEMキー
	0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xdb,
	0xdc, 0xdd, 0xde, 0xdf, 0xe2,
};

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

		for (int i = 0; i < sizeof(mPrevStates) / sizeof(mPrevStates[0]); ++i) {
			mPrevStates[i] = 0;
		}
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

	bool UpdateKeyState(std::vector<KEYEVENT>& events, SHORT vk)
	{
		ASSERT(0 <= vk && vk < 255);

		SHORT& prevState = mPrevStates[vk];
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
		UpdateKeyState(events, VK_LSHIFT);
		UpdateKeyState(events, VK_RSHIFT);
		UpdateKeyState(events, VK_LCONTROL);
		UpdateKeyState(events, VK_RCONTROL);
		UpdateKeyState(events, VK_LWIN);
		UpdateKeyState(events, VK_RWIN);
		UpdateKeyState(events, VK_LMENU);
		UpdateKeyState(events, VK_RMENU);
		
		// 他のキーが押されていたら状態リセット
		int nelems = sizeof(OTHER_KEYS) / sizeof(OTHER_KEYS[0]);
		for (int i = 0; i < nelems; ++i) {
			int vk = OTHER_KEYS[i];
			if (UpdateKeyState(events, vk) == false){
				continue;
			}
			events.clear();
			mIsOtherKeyPressed = true;
			mPrevVK = 0;
			mPrevTime = 0;
			break;  // 一つでも何かキー入力があれば、この後の結果として十分なので止める
		}
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
	bool mIsOtherKeyPressed = false;

	bool mIsEnableHotKey = false;
	SHORT mPrevStates[256];

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

		if (evt.Message == WM_KEYDOWN && (curVK == in->mFirstVK || curVK == in->mSecondVK)) {
			// リセット
			in->mIsOtherKeyPressed = false;
			continue;
		}
		if (curVK == 0) {
			// 着目しないキーの場合はスルー
			in->mIsOtherKeyPressed = true;
			continue;
		}

		if (evt.Message != WM_KEYUP) {
			// あつかうのはKEYUPのみ
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
			if (in->mIsOtherKeyPressed) {
				// 入力判定の間に別のキー入力があった場合は除外する
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



