#pragma once

#include <UIAutomation.h>

namespace launcherapp { namespace commands { namespace uiautomation {

class UIElement
{
public:
	virtual ~UIElement();

	virtual CString GetName() = 0;
	virtual CRect GetRect() = 0;
	virtual bool Click() = 0;
	virtual bool Focus() = 0;
	virtual bool CanFocus() = 0;

	virtual uint32_t AddRef() = 0;
	virtual uint32_t Release() = 0;
};

class WindowUIElement : public UIElement 
{
public:
	WindowUIElement();
	WindowUIElement(const CComPtr<IUIAutomationElement>& elem);
	~WindowUIElement();

	CString GetName() override;
	CRect GetRect() override;
	bool Click() override;
	bool Focus() override;
	bool CanFocus() override;
	uint32_t AddRef() override;
	uint32_t Release() override;


	CComPtr<IUIAutomationElement> mElem;
	uint32_t mRefCount{1};
};

}}} // end of namespace launcherapp::commands::uiautomation

