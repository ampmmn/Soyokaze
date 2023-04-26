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
AppPreference::AppPreference() :
	mIsMatchCase(false)
{
}

/**
 * デストラクタ
 */
AppPreference::~AppPreference()
{
}

/**
 * 設定値を読み込む
 */
void AppPreference::Load()
{
	CAppProfile* pProfile = CAppProfile::Get();
	mIsMatchCase = pProfile->Get(_T("BWLite"), _T("IsMatchCase"), 0) != 0;
	mFilerPath = pProfile->Get(_T("BWLite"), _T("FilerPath"), _T(""));
	mFilerParam = pProfile->Get(_T("BWLite"), _T("FilerParam"), _T(""));
}


/**
 * 大文字小文字を区別するかどうかの設定値の状態を取得する
 */
bool AppPreference::IsMatchCase() const
{
	return mIsMatchCase;
}

CString AppPreference::GetFilerPath() const
{
	return mFilerPath;
}

CString AppPreference::GetFilerParam() const
{
	return mFilerParam;
}
