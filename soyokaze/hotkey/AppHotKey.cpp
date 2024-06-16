#include "pch.h"
#include "AppHotKey.h"
#include "setting/AppPreference.h"
#include "hotkey/GlobalHotKey.h"

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

static const int ID_SOYOKAZE_HOTKEY = 0xB31E;

struct AppHotKey::PImpl
{
	std::unique_ptr<GlobalHotKey> mHotKey;
	bool mIsEnableHotKey;
};

AppHotKey::AppHotKey(HWND targetWnd) : in(new PImpl)
{
	in->mHotKey = std::make_unique<GlobalHotKey>(targetWnd);
	in->mIsEnableHotKey = true;

	AppPreference::Get()->RegisterListener(this);
}

AppHotKey::~AppHotKey()
{
	AppPreference::Get()->UnregisterListener(this);
}

// 設定ファイルから設定値を取得してホットキー登録
bool AppHotKey::Register()
{
	auto pref = AppPreference::Get();
	in->mIsEnableHotKey = pref->IsEnableAppHotKey();
	
	if (in->mIsEnableHotKey) {
		UINT mod = pref->GetModifiers();
		UINT vk = pref->GetVirtualKeyCode();
		return in->mHotKey->Register(ID_SOYOKAZE_HOTKEY, mod, vk);
	}
	else {
		// グローバルホットキーを利用しない
		return true;
	}
}

// 登録解除する
void AppHotKey::Unregister()
{
	if (in->mIsEnableHotKey) {
		in->mHotKey->Unregister();
	}
}

bool AppHotKey::IsSameKey(LPARAM lParam)
{
	if (in->mIsEnableHotKey == false) {
		return false;
	}
	return in->mHotKey->IsSameKey(lParam);
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

CString AppHotKey::ToString() const
{
	return in->mHotKey->ToString();
}


void AppHotKey::OnAppFirstBoot()
{
}

void AppHotKey::OnAppPreferenceUpdated()
{
	// アプリ設定変更の影響を受ける項目の再登録
	Reload();
}

void AppHotKey::OnAppExit()
{
	AppPreference::Get()->UnregisterListener(this);
}


}
}

