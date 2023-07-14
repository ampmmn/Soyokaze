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

// 
/**
 	IDispatchのプロパティ/メソッドにアクセスする
 	
 	このメソッドは可変長引数をうけとる。
 	メソッドに渡す引数がある場合は、cArgsの後ろに、cArgsで指定した数ぶんのVARIANTを指定する。

 	下記URLに掲載されているコードをベースにしている。
 	https://learn.microsoft.com/en-us/previous-versions/office/troubleshoot/office-developer/automate-excel-from-c

 	@return 処理結果HRESULT
 	@param[in]     autoType 種別(DISPATCH_PROPERTYGET or DISPATCH_PROPERTYPUT)
 	@param[out]    pvResult 呼び出し結果
 	@param[in,out] pDisp    IDispatchオブジェクト
 	@param[in]     ptName   プロパティ/メソッドの名前
 	@param[in]     cArgs    引数の数
*/
static HRESULT AutoWrap(
	int autoType,
	VARIANT* pvResult,
	IDispatch* pDisp,
	LPOLESTR ptName,
	int cArgs...
)
{
	if (!pDisp) {
		return E_FAIL;
	}

	va_list marker;
	va_start(marker, cArgs);

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

// この時間以内に再実行されたら、前回の結果を再利用する
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


	// ExcelのCLSIDを得る
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

	if (FAILED(hr)) {
		// 取得できなかった(インストールされていないとか)
		return false;
	}

	// 既存のExcel.Applicationインスタンスを取得する
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if(FAILED(hr)) {
		// 起動してない
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

	CComBSTR name;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, excelApp, L"Path", 0);
		name = result.bstrVal;
	}
	CString appPath = name;

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

			tmpList.push_back(new Worksheet(appPath, workbookName, sheetName));
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
	// EXCEL.EXEのフルパス
	CString mAppPath;
	// ブック名
	CString mBookName;
	// シート名
	CString mSheetName;
	// 参照カウント
	uint32_t mRefCount;
	
};

/**
 	コンストラクタ
 	@param[in] appPath      excel.exeが置かれるフォルダのフルパス
 	@param[in] workbookName ワークブック名
 	@param[in] sheetName    シート名
*/
 Worksheet::Worksheet(
	const CString& appPath,
	const CString& workbookName,
	const CString& sheetName
) : 
	in(new PImpl)
{
	in->mRefCount = 1;

	// 引数で与えられる文字列はフォルダパスなので、ファイル名を補う
	in->mAppPath = appPath;
	in->mAppPath += _T("\\EXCEL.EXE");

	in->mBookName = workbookName;
	in->mSheetName = sheetName;
}

Worksheet::~Worksheet()
{
}

const CString& Worksheet::GetAppPath()
{
	return in->mAppPath;
}

const CString& Worksheet::GetWorkbookName()
{
	return in->mBookName;
}

const CString& Worksheet::GetSheetName()
{
	return in->mSheetName;
}

/**
 	オブジェクトに紐づけられたワークシートを有効にする
 	@return 処理の成否
*/
BOOL Worksheet::Activate()
{
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

	if (FAILED(hr)) {
		// 初期化できなかった(たぶん起こらない。このオブジェクト作れてる時点でEXCELは入ってるはずなので)
		return FALSE;
	}

	// 既存のExcel.Applicationインスタンスを取得する
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if(FAILED(hr)) {
		// インスタンス生成後にEXCELが終了されたとか?
		return FALSE;
	}

	CComPtr<IDispatch> excelApp;
	unkPtr->QueryInterface(&excelApp);

	VARIANT result;

	// Excel.ApplicationからたどってWorksheetを得る

	CComPtr<IDispatch> workbooks;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, excelApp, L"Workbooks", 0);
		workbooks = result.pdispVal;
	}

	// 最後にウインドウを全面に持っていきたいので、
	// アプリケーションのウインドウハンドルも得ておく
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

			// アプリのウインドウを全面に出す
			if (IsWindow(hwndApp)) {
				ScopeAttachThreadInput scope;
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

