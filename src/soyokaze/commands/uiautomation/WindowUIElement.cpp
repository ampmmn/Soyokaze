#include "pch.h"
#include "WindowUIElement.h"
#include "utility/ScopeAttachThreadInput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace uiautomation {

UIElement::~UIElement()
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



WindowUIElement::WindowUIElement()
{
}

WindowUIElement::WindowUIElement(
	const CComPtr<IUIAutomationElement>& elem
) : mElem(elem)
{
}

WindowUIElement::~WindowUIElement()
{
}

CString WindowUIElement::GetName()
{
	// 名前を取得
	BSTR name;
	mElem->get_CurrentName(&name);
	CString elemName(name ? (const wchar_t*)name : _T(""));
	SysFreeString(name);

	return elemName;
}

CRect WindowUIElement::GetRect()
{
	CRect rc{ 0,0,0,0 };
	mElem->get_CurrentBoundingRectangle(&rc);
	return rc;
}

bool WindowUIElement::Click()
{
	IUIAutomationInvokePattern* invoke{nullptr};
	HRESULT hr = mElem->GetCurrentPatternAs(UIA_InvokePatternId, IID_IUIAutomationInvokePattern, (void**)&invoke);
	if (FAILED(hr) || invoke == nullptr) {
		// 失敗したらきやすめにフォーカスする
		return Focus();
	}

	invoke->Invoke();
	invoke->Release();
	return true;
}

uint32_t WindowUIElement::AddRef()
{
	return InterlockedIncrement(&mRefCount);
}
uint32_t WindowUIElement::Release()
{
	auto n = InterlockedDecrement(&mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}

bool WindowUIElement::Focus()
{
	mElem->SetFocus();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


Win32MenuItemElement::Win32MenuItemElement()
{
}

Win32MenuItemElement::Win32MenuItemElement(HWND hwnd, UINT id, const CString& name) : 
	mHwnd(hwnd), mMenuId(id), mName(name)
{
}

Win32MenuItemElement::~Win32MenuItemElement()
{
}


CString Win32MenuItemElement::GetName()
{
	return mName;
}

CRect Win32MenuItemElement::GetRect()
{
	return CRect(0,0,0,0);
}

bool Win32MenuItemElement::Click()
{
	if (IsWindow(mHwnd) == FALSE) {
		return false;
	}

	// 親ウインドウを前面に出す
	ScopeAttachThreadInput scope;
	SetForegroundWindow(mHwnd);

	// メニューを選択する操作
	SendMessage(mHwnd, WM_COMMAND, MAKEWPARAM(mMenuId, 0), 0);

	return true;
}

uint32_t Win32MenuItemElement::AddRef()
{
	return InterlockedIncrement(&mRefCount);
}

uint32_t Win32MenuItemElement::Release()
{
	auto n = InterlockedDecrement(&mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}



}}} // end of namespace launcherapp::commands::uiautomation

