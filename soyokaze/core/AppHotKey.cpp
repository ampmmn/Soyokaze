#include "pch.h"
#include "AppHotKey.h"
#include "AppPreference.h"
#include "core/GlobalHotKey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace core {

// 修飾キー
// MOD_ALT     (0x0001)
// MOD_CONTROL (0x0002)
// MOD_SHIFT   (0x0004)
// MOD_WIN     (0x0008)
// MOD_NOREPEAT(0x4000)

static const int ID_SOYOKAZE_HOTKEY = 0xB31E;

AppHotKey::AppHotKey(HWND targetWnd) : 
	mHotKey(new GlobalHotKey(targetWnd))
{
	AppPreference::Get()->RegisterListener(this);
}

AppHotKey::~AppHotKey()
{
	AppPreference::Get()->UnregisterListener(this);
}

// 設定ファイルから設定値を取得してホットキー登録
bool AppHotKey::Register()
{
	UINT mod;
	UINT vk;
	if (LoadKeyConfig(mod, vk) == false) {
		// キー設定がない、あるいは無効化されている
		return false;
	}
	return mHotKey->Register(ID_SOYOKAZE_HOTKEY, mod, vk);
}

// 登録解除する
void AppHotKey::Unregister()
{
	mHotKey->Unregister();
}

bool AppHotKey::IsSameKey(LPARAM lParam)
{
	return mHotKey->IsSameKey(lParam);
}

// 再登録(登録解除→登録)
bool AppHotKey::Reload()
{
	// 下記URLの説明によると、IDが同じなら上書きされるそうなので
	// Unregisterしなくてもよさそうだけど一応しておく
	//   http://www.kab-studio.biz/Programing/Codian/MFCTips/13.html

	Unregister();
	return Register();
}

bool AppHotKey::LoadKeyConfig(UINT& modifiers, UINT& vk)
{
	auto pref = AppPreference::Get();

	modifiers = pref->GetModifiers();
	vk = pref->GetVirtualKeyCode();

	return true;
}

void AppHotKey::OnAppFirstBoot()
{
}

void AppHotKey::OnAppPreferenceUpdated()
{
	// アプリ設定変更の影響を受ける項目の再登録
	Reload();
}

}
}

