#include "pch.h"
#include "GlobalHotKey.h"
#include "hotkey/HotKeyAttribute.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace core {

// 修飾キー
// MOD_ALT     (0x0001)
// MOD_CONTROL (0x0002)
// MOD_SHIFT   (0x0004)
// MOD_WIN     (0x0008)
// MOD_NOREPEAT(0x4000)

GlobalHotKey::GlobalHotKey(HWND targetWnd) :
	mTargetWnd(targetWnd),
	mVirtualKey(0),
	mModifiers(0),
	mID(0)

{
}

GlobalHotKey::~GlobalHotKey()
{
	Unregister();
}

// 設定ファイルから設定値を取得してホットキー登録
bool GlobalHotKey::Register(UINT id, UINT mod, UINT vk)
{
	mID = id;
	mModifiers = mod;
	mVirtualKey = vk;
	return RegisterHotKey(mTargetWnd, mID, mModifiers, mVirtualKey);
}

// 登録解除する
void GlobalHotKey::Unregister()
{
	UnregisterHotKey(mTargetWnd, mID);
}

/**
 * WM_HOTKEYのlParamが示す通知コードが同じキーを表すものかを判定する
 * @return true: 同じ false: 異なる
 * @param lParam lParamの値
 */
bool GlobalHotKey::IsSameKey(LPARAM lParam)
{
	UINT modifiers = LOWORD(lParam);
	UINT vk = HIWORD(lParam);
	return modifiers == mModifiers && vk == mVirtualKey;
}

UINT GlobalHotKey::GetID() const
{
	return mID;
}

CString GlobalHotKey::ToString() const
{
	return HOTKEY_ATTR(mModifiers, mVirtualKey).ToString();
}


}
}

