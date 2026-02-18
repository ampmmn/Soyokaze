#include "pch.h"
#include "WindowUIElements.h"
#include "commands/uiautomation/UIElementAliasMap.h"
#include "utility/Path.h"
#include <list>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <utility>

using json = nlohmann::json;

namespace launcherapp { namespace commands { namespace uiautomation {

enum FIND_ELEMENT_TYPE {
	TYPE_CLICKABLE,
	TYPE_TABCTRL,
};

static bool IsPaneOrTreeItemElement(CComPtr<IUIAutomationElement>& elem)
{
	// 特定アプリで、PaneコントロールがOffScreen==trueであるにもかかわらず、
	// 子要素はOffScreen==falseであるケースがあったのでそれを識別する
	CONTROLTYPEID typeId = 0;
	elem->get_CachedControlType(&typeId);
	return typeId == UIA_PaneControlTypeId || typeId == UIA_TreeItemControlTypeId || typeId == UIA_TreeControlTypeId;
}

static bool IsEndElement(CComPtr<IUIAutomationElement>& elem)
{
	CONTROLTYPEID typeId = 0;
	elem->get_CachedControlType(&typeId);

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

struct WindowUIElements::PImpl
{
	bool CreateAutomation() {
		if (!mAutomation) {
			HRESULT hr = CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&mAutomation);
			if (FAILED(hr)) {
				return false;
			}
		}
		return true;
	}

	HWND mHwnd{nullptr};
	CComPtr<IUIAutomation> mAutomation;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


constexpr uint64_t SEARCH_TIMEOUT = 5000;

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

static std::set<UINT>& GetTargetSet(int findType)
{
	if (findType == TYPE_CLICKABLE) {
		static std::set<UINT> targetTypeId {
			UIA_ButtonControlTypeId,
				UIA_CheckBoxControlTypeId,
				UIA_ComboBoxControlTypeId,
				UIA_EditControlTypeId,
				UIA_HyperlinkControlTypeId,
				UIA_ImageControlTypeId,
				UIA_MenuItemControlTypeId,
				UIA_RadioButtonControlTypeId,
				UIA_SplitButtonControlTypeId,
				UIA_TabControlTypeId,
				UIA_TabItemControlTypeId,
				UIA_TreeItemControlTypeId,
				UIA_ListItemControlTypeId,
				UIA_HeaderItemControlTypeId,
		};
		return targetTypeId;
	}
	else {
		static std::set<UINT> targetTypeId {
			UIA_TabItemControlTypeId,
		};
		return targetTypeId;
	}
}

static bool IsTargetElement(CComPtr<IUIAutomationElement>& elem, int findType)
{
	// 種別を取得
	CONTROLTYPEID typeId = 0;
	elem->get_CachedControlType(&typeId);
	if (GetTargetSet(findType).count(typeId) == 0) {
		return false;
	}

	return true;
}

static bool IsElementEnabled(CComPtr<IUIAutomationElement>& elem)
{
	BOOL isEnable = FALSE;
 	elem->get_CachedIsEnabled(&isEnable);
	if (isEnable == false) {
		return false;
	}
	return true;
}

static bool IsElementVisible(CComPtr<IUIAutomationElement>& elem)
{
		BOOL isOffScreen = FALSE;
		elem->get_CachedIsOffscreen(&isOffScreen);
		if (isOffScreen) {
			return false;
		}

		// 領域を取得
		CRect rc;
		elem->get_CachedBoundingRectangle(&rc);
		if(rc.IsRectEmpty()){
			return false;
		}
		return true;
}

struct QUEUE_ITEM
{
	CComPtr<IUIAutomationElement> mCurrentItem;
	bool mIsRoot{false};
};

bool WindowUIElements::FetchElements(UIElementList& items)
{
	return FetchElements(items, TYPE_CLICKABLE);
}

bool WindowUIElements::FetchTabItemElements(UIElementList& items)
{
	return FetchElements(items, TYPE_TABCTRL);
}

bool WindowUIElements::FetchElements(UIElementList& items, int findType)
{
	if (IsWindow(in->mHwnd) == FALSE) {
		// 対象ウインドウが無効
		return false;
	}

	if (in->CreateAutomation() == false) {
		return false;
	}

	// ウインドウハンドルからUIElementを取得する
	CComPtr<IUIAutomationElement> windowElement;
	HRESULT hr = in->mAutomation->ElementFromHandle(in->mHwnd, &windowElement);
	if (FAILED(hr)) {
		return false;
	} 

	// キャッシュリクエスト作成
	CComPtr<IUIAutomationCacheRequest> cacheReq;
	in->mAutomation->CreateCacheRequest(&cacheReq);

	// キャッシュ対象範囲
	cacheReq->put_TreeScope(TreeScope_Element);
	// キャッシュ対象プロパティ
	cacheReq->AddProperty(UIA_NamePropertyId);
	cacheReq->AddProperty(UIA_BoundingRectanglePropertyId);
	cacheReq->AddProperty(UIA_ControlTypePropertyId);
	cacheReq->AddProperty(UIA_IsOffscreenPropertyId);
	cacheReq->AddProperty(UIA_IsEnabledPropertyId);
	cacheReq->AddProperty(UIA_IsKeyboardFocusablePropertyId);
	cacheReq->AddProperty(UIA_ClickablePointPropertyId);
	cacheReq->AddPattern(UIA_InvokePatternId);

	CComPtr<IUIAutomationElement> windowElementCached;
	windowElement->BuildUpdatedCache(cacheReq, &windowElementCached);

	auto start = GetTickCount64();

	// 条件を作成する
	VARIANT varProp;
	varProp.vt = VT_BOOL;
	varProp.boolVal = VARIANT_FALSE;

	// 画面上に表示されているかどうか
	CComPtr<IUIAutomationCondition> pOffScreenCondition;
	hr = in->mAutomation->CreatePropertyCondition(UIA_IsOffscreenPropertyId, varProp, &pOffScreenCondition);

	// 有効かどうか
	varProp.boolVal = VARIANT_TRUE;
	CComPtr<IUIAutomationCondition> pEnableCondition;
	hr = in->mAutomation->CreatePropertyCondition(UIA_IsEnabledPropertyId, varProp, &pEnableCondition);

	// AND
	CComPtr<IUIAutomationCondition> pAndCondition;
	in->mAutomation->CreateAndCondition(pEnableCondition, pOffScreenCondition, &pAndCondition);

	std::list<QUEUE_ITEM> queue;
	queue.push_back(QUEUE_ITEM{windowElementCached, true});
	while (queue.empty() == false) {
		spdlog::debug("queue size : {}", queue.size());

		if (GetTickCount64()-start >= SEARCH_TIMEOUT) {
			spdlog::debug("FeatchElement timeout.");
			break;
		}

		auto item = queue.front();
		queue.pop_front();

		CComPtr<IUIAutomationElement>& curElement = item.mCurrentItem;

		// 子要素を取得
		auto& pCondition = IsPaneOrTreeItemElement(curElement) ? pEnableCondition : pAndCondition;

		CComPtr<IUIAutomationElementArray> pElementArray;
		curElement->FindAllBuildCache(TreeScope_Children, pCondition, cacheReq, &pElementArray);
		if (pElementArray == nullptr) {
			continue;
		}

		int numOfChildren = 0;
		pElementArray->get_Length(&numOfChildren);
		spdlog::debug("numOfChildren : {}", numOfChildren);

		// 探索に時間がかかりすぎる場合があるので、要素数を制限する
		if (numOfChildren > 100) {
			numOfChildren = 100;
		}

		for (int i = 0; i < numOfChildren; ++i) {

			if (GetTickCount64()-start >= SEARCH_TIMEOUT) {
				// 時間切れ
				break;
			}

			CComPtr<IUIAutomationElement> pChild;
			pElementArray->GetElement(i, &pChild);

			bool isVisible = IsElementEnabled(pChild);
			if (isVisible == false) {
				continue;
			}

			// 条件に該当する場合は検索結果として追加する
			if (IsTargetElement(pChild, findType) && IsElementVisible(pChild)) {

				// 名前を取得
				BSTR name = nullptr;
				pChild->get_CachedName(&name);
				std::wstring elemName(name ? (const wchar_t*)name : _T(""));
				SysFreeString(name);

				if (elemName.empty() == false) {

					items.push_back(RefPtr<UIElement>(new WindowUIElement(pChild)));
				}
			}

			if (IsEndElement(pChild)) {
				// 末端の要素だったら、再探索しない
				continue;
			}


			// キューに追加(子要素を探索する)
			queue.push_back(QUEUE_ITEM{ pChild, false});
		}
	}

	return true;
}

bool WindowUIElements::FetchWin32MenuItems(UIElementList& items)
{
	if (IsWindow(in->mHwnd) == FALSE) {
		// 対象ウインドウが無効
		return false;
	}

	// ウインドウのメインメニューを取得する
	HMENU hTopMenu = GetMenu(in->mHwnd);
	if (hTopMenu == nullptr) {
		// メニュー持ってない
		return false;
	}

	std::set<UINT> commandIds;

	std::vector<std::pair<HMENU, CString> > stk;
	stk.push_back(std::make_pair(hTopMenu, _T("")));

	while( stk.empty() == false) {

		auto item = stk.back();
		stk.pop_back();

		HMENU h = item.first;
		CString parentLabel = item.second;

		int count = GetMenuItemCount(h);
		for (int i = 0; i < count; ++i) {
			TCHAR text[256] = { _T('\0') };

			MENUITEMINFO info{ sizeof(MENUITEMINFO), MIIM_STRING | MIIM_STATE | MIIM_FTYPE };
			info.cch = 256;
			info.dwTypeData = text;

			GetMenuItemInfo(h, i, TRUE, &info);

			if (info.fState & MFS_DISABLED) {
				continue;
			}
			if ((info.fType & MFT_OWNERDRAW) || text[0] == _T('\0')) {
				// オーナードローの項目は、名前が取れないことがあるので、インデックスを名前にする
				_stprintf_s(text, _T("(%d)"), i + 1);
			}

			CString text_;
			if (parentLabel.IsEmpty() == FALSE) {
				text_ = parentLabel + _T(" > ") + text;
			}
			else {
				text_ = text;
			}
			text_.Replace(_T("\t"), _T(" "));
			text_.Replace(_T("&"), _T(""));

			auto subMenu = GetSubMenu(h, i);
			if (subMenu == nullptr) {
				UINT commandId = GetMenuItemID(h, i);
				items.push_back(RefPtr<UIElement>(new Win32MenuItemElement(in->mHwnd, commandId, text_)));
				commandIds.insert(commandId);
			}
			else {
				// 子要素を探す
				stk.push_back(std::make_pair(subMenu, text_));
			}
		}
	}

	// キャッシュ(uielement-menu-alias.json)にある項目を取得
	std::vector<std::pair<UINT,CString> > cachedMenuElements;

	auto aliasMap = UIElementAliasMap::GetInstance();
	TCHAR clsName[256];
	GetClassName(in->mHwnd, clsName, 256);
	aliasMap->EnumElements(clsName, cachedMenuElements);

	for (auto& elem : cachedMenuElements) {
		auto& commandId = elem.first;
		auto& name = elem.second;
		if (commandIds.count(commandId) != 0) {
			continue;
		}
		items.push_back(RefPtr<UIElement>(new Win32MenuItemElement(in->mHwnd, commandId, name)));
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// 以下デバッグ用処理


struct QUEUE_DUMP_ITEM
{
	std::shared_ptr<json> mParentJsonObj;
	CComPtr<IUIAutomationElement> mCurrentItem;
	std::string mParentLabel;
};

static std::string fromControlIdToString(int typeId)
{
	switch(typeId) {
	case UIA_ButtonControlTypeId: return "UIA_ButtonControlTypeId";
	case UIA_CalendarControlTypeId: return "UIA_CalendarControlTypeId";
	case UIA_CheckBoxControlTypeId: return "UIA_CheckBoxControlTypeId";
	case UIA_ComboBoxControlTypeId: return "UIA_ComboBoxControlTypeId";
	case UIA_EditControlTypeId: return "UIA_EditControlTypeId";
	case UIA_HyperlinkControlTypeId: return "UIA_HyperlinkControlTypeId";
	case UIA_ImageControlTypeId: return "UIA_ImageControlTypeId";
	case UIA_ListItemControlTypeId: return "UIA_ListItemControlTypeId";
	case UIA_ListControlTypeId: return "UIA_ListControlTypeId";
	case UIA_MenuControlTypeId: return "UIA_MenuControlTypeId";
	case UIA_MenuBarControlTypeId: return "UIA_MenuBarControlTypeId";
	case UIA_MenuItemControlTypeId: return "UIA_MenuItemControlTypeId";
	case UIA_ProgressBarControlTypeId: return "UIA_ProgressBarControlTypeId";
	case UIA_RadioButtonControlTypeId: return "UIA_RadioButtonControlTypeId";
	case UIA_ScrollBarControlTypeId: return "UIA_ScrollBarControlTypeId";
	case UIA_SliderControlTypeId: return "UIA_SliderControlTypeId";
	case UIA_SpinnerControlTypeId: return "UIA_SpinnerControlTypeId";
	case UIA_StatusBarControlTypeId: return "UIA_StatusBarControlTypeId";
	case UIA_TabControlTypeId: return "UIA_TabControlTypeId";
	case UIA_TabItemControlTypeId: return "UIA_TabItemControlTypeId";
	case UIA_TextControlTypeId: return "UIA_TextControlTypeId";
	case UIA_ToolBarControlTypeId: return "UIA_ToolBarControlTypeId";
	case UIA_ToolTipControlTypeId: return "UIA_ToolTipControlTypeId";
	case UIA_TreeControlTypeId: return "UIA_TreeControlTypeId";
	case UIA_TreeItemControlTypeId: return "UIA_TreeItemControlTypeId";
	case UIA_CustomControlTypeId: return "UIA_CustomControlTypeId";
	case UIA_GroupControlTypeId: return "UIA_GroupControlTypeId";
	case UIA_ThumbControlTypeId: return "UIA_ThumbControlTypeId";
	case UIA_DataGridControlTypeId: return "UIA_DataGridControlTypeId";
	case UIA_DataItemControlTypeId: return "UIA_DataItemControlTypeId";
	case UIA_DocumentControlTypeId: return "UIA_DocumentControlTypeId";
	case UIA_SplitButtonControlTypeId: return "UIA_SplitButtonControlTypeId";
	case UIA_WindowControlTypeId: return "UIA_WindowControlTypeId";
	case UIA_PaneControlTypeId: return "UIA_PaneControlTypeId";
	case UIA_HeaderControlTypeId: return "UIA_HeaderControlTypeId";
	case UIA_HeaderItemControlTypeId: return "UIA_HeaderItemControlTypeId";
	case UIA_TableControlTypeId: return "UIA_TableControlTypeId";
	case UIA_TitleBarControlTypeId: return "UIA_TitleBarControlTypeId";
	case UIA_SeparatorControlTypeId: return "UIA_SeparatorControlTypeId";
	case UIA_SemanticZoomControlTypeId: return "UIA_SemanticZoomControlTypeId";
	case UIA_AppBarControlTypeId: return "UIA_AppBarControlTypeId";
	default:
		return fmt::format("Unknown type({})", typeId);
	}
}

static void makeJsonObj(CComPtr<IUIAutomationElement> elem, json& json_obj, const std::string& parent_label)
{
	json_obj["children"] = json::array();

	CRect rc{ 0,0,0,0 };
	elem->get_CurrentBoundingRectangle(&rc);

	auto rect_array = json::array();
	rect_array.push_back(rc.left);
	rect_array.push_back(rc.top);
	rect_array.push_back(rc.right);
	rect_array.push_back(rc.bottom);

	json_obj["rect"] = rect_array;

	// 種別を取得
	CONTROLTYPEID typeId = 0;
	elem->get_CurrentControlType(&typeId);
	json_obj["control_type"] = fromControlIdToString(typeId);
	// 
	BOOL isOffScreen = FALSE;
	elem->get_CurrentIsOffscreen(&isOffScreen);
	json_obj["is_offscreen"] = isOffScreen != FALSE;

	BOOL isEnable = FALSE;
	elem->get_CurrentIsEnabled(&isEnable);
	json_obj["is_enabled"] = isEnable != FALSE;

	BOOL isFocusable = FALSE;
	elem->get_CurrentIsKeyboardFocusable(&isFocusable);
	json_obj["is_keyboard_focusable"] = isFocusable != FALSE;

	// 名前を取得
	BSTR name = FALSE;
	elem->get_CurrentName(&name);
	std::wstring elemName(name ? (const wchar_t*)name : _T(""));
	SysFreeString(name);

	std::string elemName_;
	UTF2UTF(elemName, elemName_);
	if (parent_label.empty() == false) {
		elemName_ = parent_label + "|" + elemName_;
	}

	json_obj["name"] = elemName_;
}

// デバッグ用
void WindowUIElements::Dump()
{
	if (IsWindow(in->mHwnd) == FALSE) {
		// 対象ウインドウが無効
		return ;
	}

	if (in->CreateAutomation() == false) {
		return;
	}

	// ウインドウハンドルからUIElementを取得する
	CComPtr<IUIAutomationElement> windowElement;
	HRESULT hr = in->mAutomation->ElementFromHandle(in->mHwnd, &windowElement);
	if (FAILED(hr)) {
		return ;
	}

	std::shared_ptr<json> root_json_ptr(new json);
	auto& root_json = *root_json_ptr.get();
	root_json["children"] = json::array();

	makeJsonObj(windowElement, root_json, "");

	CComPtr<IUIAutomationCondition> pCondition;
	in->mAutomation->CreateTrueCondition(&pCondition);

	std::vector<std::pair<std::shared_ptr<json>, std::shared_ptr<json> > > insert_pairs;

	auto start = GetTickCount64();

	std::list<QUEUE_DUMP_ITEM> queue;
	queue.push_back(QUEUE_DUMP_ITEM{root_json_ptr, windowElement, root_json["name"]});
	while (queue.empty() == false) {

		if (GetTickCount64()-start >= 5000) {
			// 長すぎるので打ち切る
			break;
		}

		auto item = queue.front();
		queue.pop_front();

		CComPtr<IUIAutomationElement>& curElement = item.mCurrentItem;
		auto& parent_json = *item.mParentJsonObj.get();
		auto parent_label = item.mParentLabel;

		// 子要素を取得
		CComPtr<IUIAutomationElementArray> pElementArray;
		curElement->FindAll(TreeScope_Children, pCondition, &pElementArray);
		if (pElementArray == nullptr) {
			parent_json["has_children"] = false;
			continue;
		}
		parent_json["has_children"] = true;

		int numOfChildren = 0;
		pElementArray->get_Length(&numOfChildren);
		for (int i = 0; i < numOfChildren; ++i) {

			if (GetTickCount64()-start >= 5000) {
				// 長すぎるので打ち切る
				break;
			}

			CComPtr<IUIAutomationElement> pChild;
			pElementArray->GetElement(i, &pChild);

			std::shared_ptr<json> current_json_ptr(new json);
			auto& current_json = *current_json_ptr.get();
			current_json["children"] = json::array();

			makeJsonObj(pChild, current_json, parent_label);

			insert_pairs.push_back(std::make_pair(item.mParentJsonObj, current_json_ptr));

			queue.push_back(QUEUE_DUMP_ITEM{current_json_ptr, pChild, current_json["name"].get<std::string>()});
		}
	}

	for (auto it = insert_pairs.rbegin(); it != insert_pairs.rend(); ++it) {
		auto& pa = *it;
		auto& parent = *pa.first.get();
		auto& child = *pa.second.get();
		parent["children"].push_back(child);
	}

	// ファイルに出力
	Path logPath(Path::APPDIRPERMACHINE, _T("uielements.json"));
	std::ofstream file((LPCTSTR)logPath);
	file << root_json.dump(4);
}

}}} // end of namespace launcherapp::commands::uiautomation
