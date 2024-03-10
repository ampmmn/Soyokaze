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

constexpr int EMPTY_LIMIT = 20;   // この数だけ空白が連続で続いたら検索を打ち切る

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

static bool CreateExcelApplication(DispWrapper& excelApp)
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
	DispWrapper excelApp;
	if (CreateExcelApplication(excelApp) == false) {
		return -1;
	}

	// 関数を抜けるときにExcelを終了する
	struct local_quit {
		local_quit(DispWrapper& dispPtr) : mDisp(dispPtr) {}
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
		DispWrapper mDisp;
	} _quit_(excelApp);


	DispWrapper workBooks;
	if (excelApp.GetPropertyObject(L"WorkBooks", workBooks) == false) {
		return -2;
	}

	VARIANT result;

	DispWrapper workBook;
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
		local_close(DispWrapper& dispPtr) : mDisp(dispPtr) {}
		~local_close() {
			mDisp.CallVoidMethod(L"Close", false);
		}
		DispWrapper mDisp;
	} _close_(workBook);

	DispWrapper workSheets;
	if (workBook.GetPropertyObject(L"WorkSheets", workSheets) == false)  {
		return -4;
	}

	DispWrapper workSheet;
	if (workSheets.GetPropertyObject(L"Item", (LPOLESTR)(LPCOLESTR)sheetName, workSheet) == false) {
		return -5;
	}

	DispWrapper range;
	if (workSheet.GetPropertyObject(L"Range", (LPOLESTR)(LPCOLESTR)address, range) == false) {
		return -6;
	}

	DispWrapper rows;
	if (range.GetPropertyObject(L"Rows", rows) == false) {
		return -7;
	}

	DispWrapper cols;
	if (range.GetPropertyObject(L"Columns", cols) == false) {
		return -8;
	}

	int row_count = rows.GetPropertyInt(L"Count");
	int col_count = cols.GetPropertyInt(L"Count");

	CString line;

	int emptyCount = 0;

	DispWrapper cell;
	for (int row = 1; row <= row_count; ++row) {
		if (emptyCount >= EMPTY_LIMIT) {
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

			CString text = cell.GetPropertyString(L"Text");

			if (line.IsEmpty() == FALSE) {
				line += _T(" ");
			}
			line += text;
		}
		texts.push_back(line);
		if (line.IsEmpty() == FALSE) {
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

