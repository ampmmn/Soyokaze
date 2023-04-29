#include "pch.h"
#include "framework.h"
#include "HotKey.h"
#include "AppProfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 修飾キー
// MOD_ALT     (0x0001)
// MOD_CONTROL (0x0002)
// MOD_SHIFT   (0x0004)
// MOD_WIN     (0x0008)
// MOD_NOREPEAT(0x4000)

static const int ID_BWLITE_HOTKEY = 0xB31E;

HotKey::HotKey(HWND targetWnd) : mTargetWnd(targetWnd)
{
}

HotKey::~HotKey()
{
	Unregister();
}

// 設定ファイルから設定値を取得してホットキー登録
bool HotKey::Register()
{
	UINT modifier;
	UINT vk;
	if (LoadKeyConfig(modifier, vk) == false) {
		// キー設定がない、あるいは無効化されている
		return false;
	}

	return RegisterHotKey(mTargetWnd, ID_BWLITE_HOTKEY, modifier, vk);
}

// 登録解除する
void HotKey::Unregister()
{
	UnregisterHotKey(mTargetWnd, ID_BWLITE_HOTKEY);
}

// 再登録(登録解除→登録)
bool HotKey::Reload()
{
	// 下記URLの説明によると、IDが同じなら上書きされるそうなので
	// Unregisterしなくてもよさそうだけど一応しておく
	//   http://www.kab-studio.biz/Programing/Codian/MFCTips/13.html

	Unregister();
	return Register();
}

bool HotKey::SaveKeyConfig(UINT modifiers, UINT vk)
{
	CAppProfile* pProfile = CAppProfile::Get();
	pProfile->Write(_T("HotKey"), _T("Modifiers"), (int)modifiers);
	pProfile->Write(_T("HotKey"), _T("VirtualKeyCode"), (int)vk);

	return true;
}

bool HotKey::LoadKeyConfig(UINT& modifiers, UINT& vk)
{
	CAppProfile* pProfile = CAppProfile::Get();

	bool isEnable = pProfile->Get(_T("HotKey"), _T("Enable"), 1) != 0;
	if (isEnable == false) {
		return false;
	}

	modifiers = (UINT)pProfile->Get(_T("HotKey"), _T("Modifiers"), MOD_ALT);
	vk = (UINT)pProfile->Get(_T("HotKey"), _T("VirtualKeyCode"), VK_RETURN);

	return true;
}


