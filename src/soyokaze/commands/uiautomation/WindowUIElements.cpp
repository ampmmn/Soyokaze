#include "pch.h"
#include "WindowUIElements.h"
#include <UIAutomation.h>
#include <list>

namespace launcherapp { namespace commands { namespace uiautomation {

static bool IsEndElement(CComPtr<IUIAutomationElement>& elem)
{
	CONTROLTYPEID typeId;
	elem->get_CurrentControlType(&typeId);

	return typeId == UIA_ButtonControlTypeId ||
	       typeId == UIA_CheckBoxControlTypeId ||
	       typeId == UIA_ComboBoxControlTypeId ||
	       typeId == UIA_EditControlTypeId ||
	       typeId == UIA_HyperlinkControlTypeId ||
	       typeId == UIA_ImageControlTypeId ||
	       typeId == UIA_MenuItemControlTypeId ||
	       typeId == UIA_RadioButtonControlTypeId ||
	       typeId == UIA_ScrollBarControlTypeId ||
	       typeId == UIA_SliderControlTypeId ||
	       typeId == UIA_SplitButtonControlTypeId ||
	       typeId == UIA_TextControlTypeId ||
	       typeId == UIA_ThumbControlTypeId;
}

static bool IsNotTargetElementType(CONTROLTYPEID typeId)
{
	return typeId == UIA_TabControlTypeId ||
	       typeId == UIA_ToolBarControlTypeId ||
	       typeId == UIA_ListControlTypeId ||
	       typeId == UIA_MenuControlTypeId ||
	       typeId == UIA_MenuBarControlTypeId ||
	       typeId == UIA_ProgressBarControlTypeId ||
	       typeId == UIA_ScrollBarControlTypeId ||
	       typeId == UIA_SliderControlTypeId ||
	       typeId == UIA_StatusBarControlTypeId ||
	       typeId == UIA_TabControlTypeId ||
	       typeId == UIA_ToolBarControlTypeId ||
	       typeId == UIA_ToolTipControlTypeId ||
	       typeId == UIA_TreeControlTypeId ||
	       typeId == UIA_GroupControlTypeId ||
	       typeId == UIA_DocumentControlTypeId ||
	       typeId == UIA_WindowControlTypeId ||
	       typeId == UIA_PaneControlTypeId ||
	       typeId == UIA_HeaderControlTypeId ||
	       typeId == UIA_TableControlTypeId ||
	       typeId == UIA_TitleBarControlTypeId ||
	       typeId == UIA_SeparatorControlTypeId ||
	       typeId == UIA_AppBarControlTypeId;
}

struct WindowUIElements::PImpl
{
	HWND mHwnd{nullptr};
	CComPtr<IUIAutomation> mAutomation;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////




WindowUIElements::WindowUIElements(HWND hwnd) : in(new PImpl)
{
	in->mHwnd = hwnd;
}

WindowUIElements::~WindowUIElements()
{
}

static bool IsRectEmpty(const RECT& rc)
{
	return rc.right-rc.left== 0 || rc.bottom-rc.top == 0;
}

static bool IsTargetElement(CComPtr<IUIAutomationElement>& elem)
{
		// 種別を取得
		CONTROLTYPEID typeId;
		elem->get_CurrentControlType(&typeId);
		if (IsNotTargetElementType(typeId)) {
			return false;
		}

		BOOL isOffScreen;
		elem->get_CurrentIsOffscreen(&isOffScreen);
		if (isOffScreen) {
			return false;
		}

		BOOL isEnable;
		elem->get_CurrentIsEnabled(&isEnable);
		if (isEnable == false) {
			return false;
		}

		BOOL isFocusable;
		elem->get_CurrentIsKeyboardFocusable(&isFocusable);
		if (isFocusable == false) {
			return false;
		}

		// 領域を取得
		CRect rc;
		elem->get_CurrentBoundingRectangle(&rc);
		if(rc.IsRectEmpty()){
			return false;
		}
		return true;
}

struct QUEUE_ITEM
{
	CComPtr<IUIAutomationElement> mCurrentItem;
	CRect mRect{0,0,0,0};
	bool mIsRoot{false};
};

bool WindowUIElements::FetchElements(std::vector<WindowUIElement>& items)
{
	if (IsWindow(in->mHwnd) == FALSE) {
		// 対象ウインドウが無効
		return false;
	}

	HRESULT hr = 
		CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&in->mAutomation);
	if (FAILED(hr)) {
		return false;
	}

	// ウインドウハンドルからUIElementを取得する
	CComPtr<IUIAutomationElement> windowElement;
	hr = in->mAutomation->ElementFromHandle(in->mHwnd, &windowElement);
	if (FAILED(hr)) {
		return false;
	}

	CRect rcRoot;
	GetWindowRect(in->mHwnd, &rcRoot);

	CComPtr<IUIAutomationCondition> pCondition;
	in->mAutomation->CreateTrueCondition(&pCondition);

	std::list<QUEUE_ITEM> queue;
	queue.push_back(QUEUE_ITEM{windowElement, {0,0,0,0}, true});
	while (queue.empty() == false) {
		spdlog::debug("queue size : {}", queue.size());

		auto item = queue.front();
		queue.pop_front();

		CComPtr<IUIAutomationElement>& curElement = item.mCurrentItem;
		CRect rcParent = item.mRect;

		CComPtr<IUIAutomationElementArray> pElementArray;
		curElement->FindAll(TreeScope_Children, pCondition, &pElementArray);
		if (pElementArray == nullptr) {
			continue;
		}

		int numOfChildren = 0;
		pElementArray->get_Length(&numOfChildren);
		spdlog::debug("numOfChildren : {}", numOfChildren);


		for (int i = 0; i < numOfChildren; ++i) {

			CComPtr<IUIAutomationElement> pChild;
			pElementArray->GetElement(i, &pChild);

			CRect rcCurrent{ 0,0,0,0 };
			pChild->get_CurrentBoundingRectangle(&rcCurrent);

			CRect rcClip;
			rcClip = rcRoot & rcCurrent;

			// 条件に該当する場合は検索結果として追加する
			if (IsTargetElement(pChild)) {

				// 名前を取得
				BSTR name;
				pChild->get_CurrentName(&name);
				std::wstring elemName(name ? (const wchar_t*)name : _T(""));
				SysFreeString(name);

				if (elemName.empty() == false) {

					items.push_back(WindowUIElement{ elemName, rcClip });
				}
			}

			if (IsEndElement(pChild)) {
				// 末端の要素だったら、再探索しない
				continue;
			}


			// キューに追加(子要素を探索する)
			queue.push_back(QUEUE_ITEM{ pChild, rcClip, false});
		}
	}

	return true;
}

}}} // end of namespace launcherapp::commands::uiautomation
