#include "pch.h"
#include "ExcelWrapper.h"
#include "commands/activate_window/AutoWrap.h"
#include "utility/ScopeAttachThreadInput.h"
#include <mutex>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace simple_dict {

using namespace soyokaze::commands::activate_window;

struct ExcelApplication::PImpl
{
};

ExcelApplication::ExcelApplication() : in(new PImpl)
{
	CoInitialize(NULL);
}

ExcelApplication::~ExcelApplication()
{
	CoUninitialize();
}

static bool GetExcelApplication(CComPtr<IDispatch>& excelApp)
{
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

		hr = unkPtr->QueryInterface(&excelApp);
		if (FAILED(hr)) {
			return false;
		}
		return true;
}

static bool CreateExcelApplication(CComPtr<IDispatch>& excelApp)
{
		// ExcelのCLSIDを得る
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

		if (FAILED(hr)) {
			// 取得できなかった(インストールされていないとか)
			return false;
		}

		// 既存のExcel.Applicationインスタンスを取得する
		hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void**)&excelApp);
		if(FAILED(hr)) {
			// 起動してない
			return false;
		}

		return true;
}

static bool GetActiveSheet(
		CComPtr<IDispatch>& excelApp,
	 	CComPtr<IDispatch>& activeSheet
)
{
	VARIANT result;
	VariantInit(&result);
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, excelApp, L"ActiveSheet", 0);
	if (FAILED(hr)) {
		return false;
	}

	activeSheet = result.pdispVal;
	return true;
}

bool ExcelApplication::IsInstalled()
{
	// ExcelのCLSIDを得る
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

	if (FAILED(hr)) {
		// 取得できなかった(インストールされていないとか)
		return false;
	}
	return true;
}

bool ExcelApplication::IsAvailable()
{
	CComPtr<IDispatch> excelApp;
	if (GetExcelApplication(excelApp) == false) {
		return false;
	}

	CComPtr<IDispatch> activeSheet;
	return GetActiveSheet(excelApp, activeSheet);
}

bool ExcelApplication::GetFilePath(CString& filePath)
{
	CComPtr<IDispatch> excelApp;
	if (GetExcelApplication(excelApp) == false) {
		return false;
	}

	CComPtr<IDispatch> activeSheet;
	if (GetActiveSheet(excelApp, activeSheet) == false) {
		return false;
	}


	VARIANT result;
	VariantInit(&result);
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, activeSheet, L"Parent", 0);
	if (FAILED(hr)) {
		return false;
	}

	CComPtr<IDispatch> wb;
	wb = result.pdispVal;

	CComBSTR pathStr;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, wb, L"Path", 0);
		pathStr = result.bstrVal;
	}
	filePath = pathStr;

	CComBSTR nameStr;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, wb, L"Name", 0);
		nameStr = result.bstrVal;
	}
	CString name = nameStr;

	filePath += _T("\\");
	filePath += name;

	return true;
}

CString ExcelApplication::GetActiveSheetName()
{
	CComPtr<IDispatch> excelApp;
	if (GetExcelApplication(excelApp) == false) {
		return false;
	}

	CComPtr<IDispatch> activeSheet;
	if (GetActiveSheet(excelApp, activeSheet) == false) {
		return false;
	}

	CComBSTR sheetName;
	{
		VARIANT result;
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, activeSheet, L"Name", 0);
		sheetName = result.bstrVal;
	}
	CString name = sheetName;
	return name;
}

CString ExcelApplication::GetSelectionAddress(int& cols, int& rows)
{
	CComPtr<IDispatch> excelApp;
	if (GetExcelApplication(excelApp) == false) {
		return _T("");
	}

	VARIANT result;

	CComPtr<IDispatch> selection;
	{
		VariantInit(&result);
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, excelApp, L"Selection", 0);
		if (FAILED(hr)) {
			return _T("");
		}
		selection = result.pdispVal;
	}

	// 選択範囲を表すテキストを取得
	CComBSTR adressBStr;
	{
		VARIANT result;
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, selection, L"Address", 0);
		adressBStr = result.bstrVal;
	}

	CString addressStr = adressBStr;

	// 選択範囲の行数・列数を取得する
	CComPtr<IDispatch> rowsObj;
	{
		VariantInit(&result);
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, selection, L"Rows", 0);
		if (FAILED(hr)) {
			return _T("");
		}
		rowsObj = result.pdispVal;
	}
	CComPtr<IDispatch> columnsObj;
	{
		VariantInit(&result);
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, selection, L"Columns", 0);
		if (FAILED(hr)) {
			return _T("");
		}
		columnsObj = result.pdispVal;
	}

	{
		VariantInit(&result);
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, rowsObj, L"Count", 0);
		if (FAILED(hr)) {
			return _T("");
		}
		rows = result.intVal;
	}
	{
		VariantInit(&result);
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, columnsObj, L"Count", 0);
		if (FAILED(hr)) {
			return _T("");
		}
		cols = result.intVal;
	}

	addressStr.Replace(_T("$"), _T(""));

	return addressStr;
}

/**
 	指定した範囲のセルのテキストを取得する
 	@return 
 	@param[in]  wbPath    workbookファイルパス
 	@param[in]  sheetName シート名
 	@param[in]  address   範囲
 	@param[out] texts     取得したテキスト
*/
int ExcelApplication::GetCellText(
	const CString wbPath,
	const CString& sheetName,
	const CString& address,
	std::vector<CString>& texts
)
{
	CComPtr<IDispatch> excelApp;
	if (CreateExcelApplication(excelApp) == false) {
		return -1;
	}

	// 関数を抜けるときにExcelを終了する
	struct local_quit {
		local_quit(CComPtr<IDispatch>& dispPtr) : mDisp(dispPtr) {}
		~local_quit() {
			VARIANT result;
			VariantInit(&result);
			VARIANT arg1;
			VariantInit(&arg1);
			arg1.vt = VT_BOOL;
			arg1.boolVal = VARIANT_FALSE;
			HRESULT hr = AutoWrap(DISPATCH_PROPERTYPUT, &result, mDisp, L"DisplayAlerts", 1, &arg1);

			hr = AutoWrap(DISPATCH_METHOD, &result, mDisp, L"Quit", 0);
		}
		CComPtr<IDispatch> mDisp;
	} _quit_(excelApp);

	VARIANT result;

	CComPtr<IDispatch> workBooks;
	{
		VariantInit(&result);
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, excelApp, L"WorkBooks", 0);
		if (FAILED(hr)) {
			return -2;
		}
	}
	workBooks = result.pdispVal;

	CComPtr<IDispatch> workBook;
	{
		VariantInit(&result);

		CComBSTR argVal(wbPath);
		VARIANT arg1;
		VariantInit(&arg1);
		arg1.vt = VT_BSTR;
		arg1.bstrVal = argVal;

		VARIANT arg2;
		VariantInit(&arg2);
		arg2.vt = VT_BOOL;
		arg2.boolVal = VARIANT_FALSE;

		VARIANT arg3;
		VariantInit(&arg3);
		arg3.vt = VT_BOOL;
		arg3.boolVal = VARIANT_TRUE;

		HRESULT hr = AutoWrap(DISPATCH_METHOD, &result, workBooks, L"Open", 3, &arg3, &arg2, &arg1);
		if (FAILED(hr)) {
			return -3;
		}
	}
	workBook = result.pdispVal;

	struct local_close {
		local_close(CComPtr<IDispatch>& dispPtr) : mDisp(dispPtr) {}
		~local_close() {
			VARIANT result;
			VariantInit(&result);
			VARIANT arg1;
			VariantInit(&arg1);
			arg1.vt = VT_BOOL;
			arg1.boolVal = VARIANT_FALSE;
			HRESULT hr = AutoWrap(DISPATCH_METHOD, &result, mDisp, L"Close", 1, &arg1);
		}
		CComPtr<IDispatch> mDisp;
	} _close_(workBook);

	CComPtr<IDispatch> workSheets;
	{
		VariantInit(&result);
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, workBook, L"WorkSheets", 0);
		if (FAILED(hr)) {
			return -4;
		}
	}
	workSheets = result.pdispVal;

	CComPtr<IDispatch> workSheet;
	{
		VariantInit(&result);

		CComBSTR argVal(sheetName);
		VARIANT arg1;
		VariantInit(&arg1);
		arg1.vt = VT_BSTR;
		arg1.bstrVal = argVal;

		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, workSheets, L"Item", 1, &arg1);
		if (FAILED(hr)) {
			return -5;
		}
	}
	workSheet = result.pdispVal;

	CComPtr<IDispatch> range;
	{
		VariantInit(&result);

		CComBSTR argVal(address);
		VARIANT arg1;
		VariantInit(&arg1);
		arg1.vt = VT_BSTR;
		arg1.bstrVal = argVal;

		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, workSheet, L"Range", 1, &arg1);
		if (FAILED(hr)) {
			return -6;
		}
	}
	range = result.pdispVal;

	CComPtr<IDispatch> rows;
	{
		VariantInit(&result);
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, range, L"Rows", 0);
		if (FAILED(hr)) {
			return -7;
		}
	}
	rows = result.pdispVal;

	CComPtr<IDispatch> cols;
	{
		VariantInit(&result);
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, range, L"Columns", 0);
		if (FAILED(hr)) {
			return -8;
		}
	}
	cols = result.pdispVal;

	int row_count = 0;
	{
		VariantInit(&result);
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, rows, L"Count", 0);
		if (FAILED(hr)) {
			return -9;
		}
	}
	row_count = result.intVal;

	int col_count = 0;
	{
		VariantInit(&result);
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, cols, L"Count", 0);
		if (FAILED(hr)) {
			return -10;
		}
	}
	col_count = result.intVal;

	CString line;

	int emptyCount = 0;

	CComPtr<IDispatch> cell;
	for (int row = 1; row <= row_count; ++row) {
		if (emptyCount >= 5) {
			break;
		}
		line.Empty();
		for (int col = 1; col <= col_count; ++col) {
			VariantInit(&result);

			VARIANT arg1;
			VariantInit(&arg1);
			arg1.vt = VT_INT;
			arg1.intVal = row;

			VARIANT arg2;
			VariantInit(&arg2);
			arg2.vt = VT_INT;
			arg2.intVal = col;

			HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, range, L"Item", 2, &arg2, &arg1);
			if (FAILED(hr)) {
				return -11;
			}
			cell = result.pdispVal;

			VariantInit(&result);
			hr = AutoWrap(DISPATCH_PROPERTYGET, &result, cell, L"Text", 0);
			if (FAILED(hr)) {
				return -12;
			}

			CComBSTR text_ = result.bstrVal;
			CString text = text_;

			if (line.IsEmpty() == FALSE) {
				line += _T(" ");
			}
			line += text;
		}
		if (line.IsEmpty() == FALSE) {
			texts.push_back(line);
			emptyCount = 0;
		}
		else {
			emptyCount++;
		}
	}

	return 0;
}

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace soyokaze

