#include "pch.h"
#include "framework.h"
#include "AppPreference.h"
#include "AppProfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/**
 * コンストラクタ
 */
AppPreference::AppPreference()
{
	Load();
}

/**
 * デストラクタ
 */
AppPreference::~AppPreference()
{
}


AppPreference* AppPreference::Get()
{
	static AppPreference thePrefObj;
	return &thePrefObj;
}

/**
 * 設定値を読み込む
 */
void AppPreference::Load()
{
	CAppProfile* pProfile = CAppProfile::Get();
	mIsUseExternalFiler = (pProfile->Get(_T("BWLite"), _T("UseFiler"), 0) != 0);
	mFilerPath = pProfile->Get(_T("BWLite"), _T("FilerPath"), _T(""));
	mFilerParam = pProfile->Get(_T("BWLite"), _T("FilerParam"), _T(""));
	mModifiers = (UINT)pProfile->Get(_T("HotKey"), _T("Modifiers"), MOD_ALT);
	mHotKeyVK = (UINT)pProfile->Get(_T("HotKey"), _T("VirtualKeyCode"), VK_RETURN);

	mIsTransparencyEnable = (pProfile->Get(_T("WindowTransparency"), _T("Enable"), 0) != 0);

	mIsTransparencyInactiveOnly =(pProfile->Get(_T("WindowTransparency"), _T("InactiveOnly"), 1) != 0);
	mAlpha = pProfile->Get(_T("WindowTransparency"), _T("Alpha"), 255);

	mIsTopmost =(pProfile->Get(_T("BWLite"), _T("Topmost"), 0) != 0);

	mMatchLevel = pProfile->Get(_T("BWLite"), _T("MatchLevel"), 2);
}

void AppPreference::Save()
{
	CAppProfile* pProfile = CAppProfile::Get();
	pProfile->Write(_T("BWLite"), _T("UseFiler"), (int)mIsUseExternalFiler);
	pProfile->Write(_T("BWLite"), _T("FilerPath"), (LPCTSTR)mFilerPath);
	pProfile->Write(_T("BWLite"), _T("FilerParam"), (LPCTSTR)mFilerParam);
	pProfile->Write(_T("HotKey"), _T("Modifiers"), (int)mModifiers);
	pProfile->Write(_T("HotKey"), _T("VirtualKeyCode"), (int)mHotKeyVK);
	pProfile->Write(_T("WindowTransparency"), _T("Enable"), (int)mIsTransparencyEnable);
	pProfile->Write(_T("WindowTransparency"), _T("InactiveOnly"), (int)mIsTransparencyInactiveOnly);
	pProfile->Write(_T("WindowTransparency"), _T("Alpha"), (int)mAlpha);
	pProfile->Write(_T("BWLite"), _T("Topmost"), (int)mIsTopmost);
	pProfile->Write(_T("BWLite"), _T("MatchLevel"), (int)mMatchLevel);

	// リスナーへ通知
	for (auto listener : mListeners) {
		listener->OnAppPreferenceUpdated();
	}
}


CString AppPreference::GetFilerPath() const
{
	return mFilerPath;
}

CString AppPreference::GetFilerParam() const
{
	return mFilerParam;
}

void AppPreference::RegisterListener(AppPreferenceListenerIF* listener)
{
	mListeners.insert(listener);
}

void AppPreference::UnregisterListener(AppPreferenceListenerIF* listener)
{
	mListeners.erase(listener);
}

