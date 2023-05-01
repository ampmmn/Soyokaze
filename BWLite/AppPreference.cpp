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
	mFilerPath = pProfile->Get(_T("BWLite"), _T("FilerPath"), _T(""));
	mFilerParam = pProfile->Get(_T("BWLite"), _T("FilerParam"), _T(""));
}


CString AppPreference::GetFilerPath() const
{
	return mFilerPath;
}

CString AppPreference::GetFilerParam() const
{
	return mFilerParam;
}
