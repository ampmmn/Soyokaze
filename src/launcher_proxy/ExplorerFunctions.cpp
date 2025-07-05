#include "ExplorerFunctions.h"
#include "AutoWrap.h"
#include <windows.h>
#include <atlcomcli.h>
#include <crtdbg.h>

constexpr int MAX_PATH_NTFS = (32767 + 1);

IDispatch* GetShellApplicationDispatch()
{
	CLSID clsId;
	HRESULT hr = CLSIDFromProgID(L"Shell.Application", &clsId);
	if (FAILED(hr)) {
		return nullptr;
	}

	IDispatch* dispPtr = nullptr;
	hr = CoCreateInstance(clsId, NULL, CLSCTX_ALL, IID_IDispatch, (void**)&dispPtr);
	if (FAILED(hr)) {
		return nullptr;
	}

	return dispPtr;
}

static IDispatch* GetTargetWindowObject()
{
	CComPtr<IDispatch> pDisp;
	pDisp.Attach(GetShellApplicationDispatch());
	if (!pDisp) {
		return nullptr;
	}

	CComVariant vResult;

	HRESULT hr = AutoWrap(DISPATCH_METHOD, &vResult, pDisp, L"Windows", 0);
	if (FAILED(hr)) {
		return nullptr;
	}

	CComPtr<IDispatch> pWindowsDisp(vResult.pdispVal);
	hr = AutoWrap(DISPATCH_PROPERTYGET, &vResult, pWindowsDisp, L"Count", 0);
	if (FAILED(hr)) {
		return nullptr;
	}
	int count = vResult.intVal;

	if (count == 0) {
		return nullptr;
	}

	// すべてのWindowオブジェクトのHWNDを取得し、最も前面にあるものを選択する
	std::vector<std::pair<HWND, int> > targetElems;

	for (int i = 0; i < count; ++i) {
		
		VARIANT arg1{ VT_I4, 0, 0, 0, i };
		hr = AutoWrap(DISPATCH_METHOD, &vResult, pWindowsDisp, L"Item", 1, &arg1);
		if (FAILED(hr)) {
			return false;
		}
		CComPtr<IDispatch> pWindowItemDisp(vResult.pdispVal);

		// HWNDを得る
		hr = AutoWrap(DISPATCH_PROPERTYGET, &vResult, pWindowItemDisp, L"HWND", 0);

		if (FAILED(hr)) {
			break;
		}

		HWND hwnd = (HWND)vResult.llVal;
		targetElems.push_back(std::pair<HWND,int>(hwnd, i));
	}

	int index = 0;

	if (count > 1) {
		// 最も前面にあるものを選択
		HWND current = GetTopWindow(nullptr);
		while (IsWindow(current)) {

			bool isFound = false;
			for(auto& item : targetElems) {
				if (item.first != current) {
					continue;
				}
				index = item.second;
				isFound = true;
				break;
			}

			if (isFound) {
				break;
			}

			// 次のウインドウへ
			current = GetWindow(current, GW_HWNDNEXT);
		}
	}


	VARIANT arg1{ VT_I4, 0, 0, 0, index };
	hr = AutoWrap(DISPATCH_METHOD, &vResult, pWindowsDisp, L"Item", 1, &arg1);
	if (FAILED(hr)) {
		return false;
	}

	IDispatch* pDispResult = vResult.pdispVal;
	pDispResult->AddRef();
	return pDispResult;
}

// Exploreで表示中のパスを取得
bool Expr_GetCurrentDir(std::wstring& curDir)
{
	CComPtr<IDispatch> pWindowItemDisp;
	pWindowItemDisp.Attach(GetTargetWindowObject());
	if (!pWindowItemDisp) {
		return false;
	}
	CComVariant vResult;

	// LocationURLを得る
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &vResult, pWindowItemDisp, L"LocationURL", 0);
	if (FAILED(hr)) {
		return false;
	}

	// 取得されるパスはfile:///C:/... というURL形式なので、URLデコードしてローカルパス表記にする
	DWORD length = MAX_PATH_NTFS;
	std::vector<wchar_t> buff(length);
	PathCreateFromUrlW((LPWSTR)vResult.bstrVal, buff.data(), &length, 0);

	curDir = (LPWSTR)buff.data();
	return true;
}

// Exploreで選択中のパスを取得
bool Expr_GetSelectionPath(std::wstring& selPath, int index)
{
	CComPtr<IDispatch> pWindowItemDisp;
	pWindowItemDisp.Attach(GetTargetWindowObject());
	if (!pWindowItemDisp) {
		return false;
	}

	CComVariant vResult;

	// Documentを得る
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &vResult, pWindowItemDisp, L"Document", 0);
	if (FAILED(hr)) {
		return false;
	}
	CComPtr<IDispatch> pDocumentDisp(vResult.pdispVal);

	// SelectedItemsを得る
	hr = AutoWrap(DISPATCH_METHOD, &vResult, pDocumentDisp, L"SelectedItems", 0);
	if (FAILED(hr)) {
		return false;
	}
	CComPtr<IDispatch> pSelectedItemsDisp(vResult.pdispVal);

	// 選択項目の数を得る
	hr = AutoWrap(DISPATCH_PROPERTYGET, &vResult, pSelectedItemsDisp, L"Count", 0);
	if (FAILED(hr)) {
		return false;
	}

	int count = vResult.intVal;
	if (index < 0 || count <= index) {
		// 範囲外
		return false;
	}


	VARIANT arg1{VT_I4, 0, 0, 0, index };
	hr = AutoWrap(DISPATCH_METHOD, &vResult, pSelectedItemsDisp, L"Item", 1, &arg1);
	if (FAILED(hr)) {
		return false;
	}

	CComPtr<IDispatch> pSelectionItemDisp(vResult.pdispVal);

	hr = AutoWrap(DISPATCH_PROPERTYGET, &vResult, pSelectionItemDisp, L"Path", 0);
	if (FAILED(hr)) {
		return false;
	}

	selPath = (LPWSTR)vResult.bstrVal;
	return true;
}

bool Expr_GetAllSelectionPath(std::vector<std::wstring>& selPaths)
{
	CComPtr<IDispatch> pWindowItemDisp;
	pWindowItemDisp.Attach(GetTargetWindowObject());
	if (!pWindowItemDisp) {
		return false;
	}

	CComVariant vResult;

	// Documentを得る
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &vResult, pWindowItemDisp, L"Document", 0);
	if (FAILED(hr)) {
		return false;
	}
	CComPtr<IDispatch> pDocumentDisp(vResult.pdispVal);

	// SelectedItemsを得る
	hr = AutoWrap(DISPATCH_METHOD, &vResult, pDocumentDisp, L"SelectedItems", 0);
	if (FAILED(hr)) {
		return false;
	}
	CComPtr<IDispatch> pSelectedItemsDisp(vResult.pdispVal);

	// 選択項目の数を得る
	hr = AutoWrap(DISPATCH_PROPERTYGET, &vResult, pSelectedItemsDisp, L"Count", 0);
	if (FAILED(hr)) {
		return false;
	}

	std::vector<std::wstring> tmpPaths;

	int count = vResult.intVal;
	for (int i = 0; i < count; ++i) {

		VARIANT arg1{VT_I4, 0, 0, 0, i};
		hr = AutoWrap(DISPATCH_METHOD, &vResult, pSelectedItemsDisp, L"Item", 1, &arg1);
		if (FAILED(hr)) {
			break;
		}

		CComPtr<IDispatch> pSelectionItemDisp(vResult.pdispVal);

		hr = AutoWrap(DISPATCH_PROPERTYGET, &vResult, pSelectionItemDisp, L"Path", 0);
		if (FAILED(hr)) {
			break;
		}
		std::wstring((LPWSTR)vResult.bstrVal);
		tmpPaths.push_back(std::wstring((LPWSTR)vResult.bstrVal));
	}

	selPaths.swap(tmpPaths);
	return true;
}


