#include "pch.h"
#include "AppSettingPageBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace settingwindow {


AppSettingPageBase::AppSettingPageBase(LPCTSTR pagePath, LPCTSTR name) :
	mPagePath(pagePath), mPageName(name), mParam(nullptr), mRefCount(1)
{
}

AppSettingPageBase::~AppSettingPageBase()
{
}

// ウインドウを作成する
bool AppSettingPageBase::Create(HWND parentWindow)
{
	UNREFERENCED_PARAMETER(parentWindow);

	spdlog::warn("Not Implemented Yet");
	return false;
}

// ウインドウハンドルを取得する
HWND AppSettingPageBase::GetHwnd()
{
	spdlog::warn("Not Implemented Yet");
	return nullptr;
}

// ページのパス(\\区切りで階層を表現する)
CString AppSettingPageBase::GetPagePath() 
{
	if (mPagePath.IsEmpty()) {
		return mPageName;
	}
	else {
		return mPagePath + _T("\\") + mPageName;
	}
}

// ページ名
CString AppSettingPageBase::GetName() 
{
	return mPageName;
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageBase::GetOrder() 
{
	return 100;
}

// 
bool AppSettingPageBase::OnEnterSettings()
{
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageBase::OnSetActive()
{
	return true;
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageBase::OnKillActive()
{
	return true;
}

//
void AppSettingPageBase::OnOKCall() 
{
}

//
void AppSettingPageBase::SetParam(void* param) 
{
	mParam = param;
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageBase::GetHelpPageId(String& helpPageId)
{
	UNREFERENCED_PARAMETER(helpPageId);

	spdlog::warn("Not Implemented Yet");
	return false;
}

// インスタンスを複製する
AppSettingPageIF* AppSettingPageBase::Clone()
{
	ASSERT(0);  // 派生クラスでCloneを実装してください
	return nullptr;
}

void* AppSettingPageBase::GetParam()
{
	return mParam;
}

bool AppSettingPageBase::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	UNREFERENCED_PARAMETER(ifid);
	UNREFERENCED_PARAMETER(cmd);

	spdlog::warn("Not Implemented Yet.");
	return false;
}

uint32_t AppSettingPageBase::AddRef()
{
	return (uint32_t)InterlockedIncrement(&mRefCount);
}

uint32_t AppSettingPageBase::Release()
{
	auto n = InterlockedDecrement(&mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

}  // end of namespace settingwindow
}  // end of namespace launcherapp

