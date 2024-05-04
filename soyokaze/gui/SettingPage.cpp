#include "pch.h"
#include "SettingPage.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SettingPage::SettingPage(LPCTSTR nameStr, UINT resourceId, CWnd* parent) :
 	mName(nameStr), mResourceId(resourceId), mParentWnd(parent), mParamPtr(nullptr)
{
}

SettingPage::SettingPage(UINT nameId, UINT resourceId, CWnd* parent) :
 	mResourceId(resourceId), mParentWnd(parent), mParamPtr(nullptr)
{
	mName.LoadStringW(nameId);
}

SettingPage::~SettingPage()
{
}

BOOL SettingPage::SettingPage::Create()
{
	return CPropertyPage::Create(mResourceId, mParentWnd);
}

void SettingPage::SetName(const CString& name)
{
	mName = name;
}

const CString& SettingPage::GetName()
{
	return mName;
}

void SettingPage::SetParam(void* param)
{
	mParamPtr = param;
}

void* SettingPage::GetParam()
{
	return mParamPtr;
}

void SettingPage::EnalbleOKButton()
{
	::PostMessage(GetParent()->GetSafeHwnd(), WM_APP+1, 1, (LPARAM)this);
}

void SettingPage::DisalbleOKButton()
{
	::PostMessage(GetParent()->GetSafeHwnd(), WM_APP+2, 0, (LPARAM)this);
}

void SettingPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(SettingPage, CPropertyPage)
END_MESSAGE_MAP()


bool SettingPage::GetHelpPageId(CString& id)
{
	return false;
}
