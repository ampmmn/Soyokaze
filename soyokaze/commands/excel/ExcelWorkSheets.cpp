#include "pch.h"
#include "ExcelWorkSheets.h"
#include "utility/ScopeAttachThreadInput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace excel {

struct WorkSheets::PImpl
{
	// 前回の取得時のタイムスタンプ
	DWORD mLastUpdate;
	std::vector<Worksheet*> mCache;
};

WorkSheets::WorkSheets() : in(new PImpl)
{
	CoInitialize(NULL);

	in->mLastUpdate = 0;
}

WorkSheets::~WorkSheets()
{
	for (auto elem : in->mCache) {
		elem->Release();
	}
	in->mCache.clear();

	CoUninitialize();
}

static HRESULT AutoWrap(
	int autoType,
	VARIANT* pvResult,
	IDispatch* pDisp,
	LPOLESTR ptName,
	int cArgs...
)
{
	if (!pDisp) {
		return S_OK;
	}

	va_list marker;
	va_start(marker, cArgs);

	// Get DISPID for name passed...
	DISPID dispID;
	HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
	if (FAILED(hr)) {
		return hr;
	}

	std::vector<VARIANT> args(cArgs + 1);
	for (int i = 0; i < cArgs; i++) {
		args[i] = va_arg(marker, VARIANT);
	}
	va_end(marker);

	DISPPARAMS dp = { NULL, NULL, 0, 0 };
	dp.cArgs = cArgs;
	dp.rgvarg = &args.front();

	DISPID dispidNamed = DISPID_PROPERTYPUT;
	if (autoType & DISPATCH_PROPERTYPUT) {
		dp.cNamedArgs = 1;
		dp.rgdispidNamedArgs = &dispidNamed;
	}

	return pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, &dp, pvResult, NULL, NULL);
}

#define INTERVAL_REUSE 5000

bool WorkSheets::GetWorksheets(std::vector<Worksheet*>& worksheets)
{
	// 前回取得時から一定時間経過していない場合は前回の結果を再利用する
	DWORD elapsed = GetTickCount() - in->mLastUpdate;
	if (elapsed < INTERVAL_REUSE) {
		worksheets = in->mCache;
		for (auto elem : worksheets) {
			elem->AddRef();
		}
		return true;
	}

	// キャッシュクリア
	for (auto elem : in->mCache) {
		elem->Release();
	}
	in->mCache.clear();


	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

	if (FAILED(hr)) {
		// 初期化できなかった
		return false;
	}

	// 既存のExcel.Applicationインスタンスを取得
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if(FAILED(hr)) {
		return false;
	}

	CComPtr<IDispatch> excelApp;
	unkPtr->QueryInterface(&excelApp);

	VARIANT result;

	// Get Workbooks collection
	CComPtr<IDispatch> workbooks;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, excelApp, L"Workbooks", 0);
		workbooks = result.pdispVal;
	}

	int wbCount = 0;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, workbooks, L"Count", 0);
		wbCount = result.intVal;
	}

	std::vector<Worksheet*> tmpList;

	for (int i = 0; i < wbCount; ++i) {

		CComPtr<IDispatch> wb;
		{
			VARIANT arg1;
			VariantInit(&arg1);
			arg1.vt = VT_INT;
			arg1.intVal = i + 1;

			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, workbooks, L"Item", 1, &arg1);
			wb = result.pdispVal;
		}

		CComBSTR name;

		{
			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, wb, L"Name", 0);
			name = result.bstrVal;
		}

		CString workbookName = name;

		CComPtr<IDispatch> sheets;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, wb, L"Worksheets", 0);
			sheets = result.pdispVal;
		}

		int sheetCount = 0;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, sheets, L"Count", 0);
			sheetCount = result.intVal;
		}

		for (int j = 0; j < sheetCount; ++j) {

			CComPtr<IDispatch> sheet;
			{
				VARIANT arg1;
				VariantInit(&arg1);
				arg1.vt = VT_INT;
				arg1.intVal = j + 1;

				VariantInit(&result);
				AutoWrap(DISPATCH_PROPERTYGET, &result, sheets, L"Item", 1, &arg1);
				sheet = result.pdispVal;
			}


			{
				VariantInit(&result);
				AutoWrap(DISPATCH_PROPERTYGET, &result, sheet, L"Name", 0);
				name = result.bstrVal;
			}

			CString sheetName = name;

			tmpList.push_back(new Worksheet(workbookName, sheetName));
		}
	}


	for (auto item : worksheets) {
		item->Release();
	}
	worksheets.clear();

	worksheets.swap(tmpList);

	// キャッシュに保持しておく
	in->mCache = worksheets;
	for (auto elem : in->mCache) {
		elem->AddRef();
	}
	in->mLastUpdate = GetTickCount();

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct Worksheet::PImpl
{
	CString mBookName;
	CString mSheetName;
	uint32_t mRefCount;
	
};

Worksheet::Worksheet(const CString& workbookName, const CString& sheetName) : 
	in(new PImpl)
{
	in->mRefCount = 1;
	in->mBookName = workbookName;
	in->mSheetName = sheetName;
}

Worksheet::~Worksheet()
{
}

const CString& Worksheet::GetWorkbookName()
{
	return in->mBookName;
}

const CString& Worksheet::GetSheetName()
{
	return in->mSheetName;
}

BOOL Worksheet::Activate()
{
	// ここでWorkSheetを取得
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

	if (FAILED(hr)) {
		// 初期化できなかった
		return FALSE;
	}

	// 既存のExcel.Applicationインスタンスを取得
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if(FAILED(hr)) {
		return FALSE;
	}

	CComPtr<IDispatch> excelApp;
	unkPtr->QueryInterface(&excelApp);

	VARIANT result;

	// Get Workbooks collection
	CComPtr<IDispatch> workbooks;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, excelApp, L"Workbooks", 0);
		workbooks = result.pdispVal;
	}

	HWND hwndApp;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, excelApp, L"Hwnd", 0);
		hwndApp = (HWND)result.llVal;
	}

	int wbCount = 0;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, workbooks, L"Count", 0);
		wbCount = result.intVal;
	}

	std::vector<Worksheet*> tmpList;

	for (int i = 0; i < wbCount; ++i) {

		CComPtr<IDispatch> wb;
		{
			VARIANT arg1;
			VariantInit(&arg1);
			arg1.vt = VT_INT;
			arg1.intVal = i + 1;

			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, workbooks, L"Item", 1, &arg1);
			wb = result.pdispVal;
		}

		CComBSTR name;

		{
			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, wb, L"Name", 0);
			name = result.bstrVal;
		}

		CString workbookName = name;

		if (workbookName != GetWorkbookName()) {
			continue;
		}

		CComPtr<IDispatch> sheets;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, wb, L"Worksheets", 0);
			sheets = result.pdispVal;
		}

		int sheetCount = 0;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, sheets, L"Count", 0);
			sheetCount = result.intVal;
		}

		for (int j = 0; j < sheetCount; ++j) {

			CComPtr<IDispatch> sheet;
			{
				VARIANT arg1;
				VariantInit(&arg1);
				arg1.vt = VT_INT;
				arg1.intVal = j + 1;

				VariantInit(&result);
				AutoWrap(DISPATCH_PROPERTYGET, &result, sheets, L"Item", 1, &arg1);
				sheet = result.pdispVal;
			}


			{
				VariantInit(&result);
				AutoWrap(DISPATCH_PROPERTYGET, &result, sheet, L"Name", 0);
				name = result.bstrVal;
			}

			CString sheetName = name;

			if (sheetName != GetSheetName()) {
				continue;
			}

			// シートをアクティブにする
			VARIANT result;
			VariantInit(&result);
			HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, sheet, L"Activate", 0);

			if (FAILED(hr)) {
				return FALSE;
			}

			// アプリを全面に出す
			if (IsWindow(hwndApp)) {
				ScopeAttachThreadInput scope;
				BringWindowToTop(hwndApp);
				SetForegroundWindow(hwndApp);
				ShowWindow(hwndApp, SW_SHOWNORMAL);
			}
			return TRUE;
		}
	}

	return FALSE;
}


uint32_t Worksheet::AddRef()
{
	return ++in->mRefCount;
}

uint32_t Worksheet::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace excel
} // end of namespace commands
} // end of namespace soyokaze

