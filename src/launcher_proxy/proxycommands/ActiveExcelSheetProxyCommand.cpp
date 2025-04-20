#include "ActiveExcelSheetProxyCommand.h"
#include <windows.h>
#include "processproxy/share/AfxWFunctions.h"
#include "ScopeAttachThreadInput.h"
#include "StringUtil.h"
#include "AutoWrap.h"
#include <oleauto.h>
#include <atlbase.h>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace launcherproxy { 

REGISTER_PROXYCOMMAND(ActiveExcelSheetProxyCommand)

ActiveExcelSheetProxyCommand::ActiveExcelSheetProxyCommand()
{
}

ActiveExcelSheetProxyCommand::~ActiveExcelSheetProxyCommand()
{
}

std::string ActiveExcelSheetProxyCommand::GetName()
{
	return "activeexcelsheet";
}

bool ActiveExcelSheetProxyCommand::Execute(json& json_req, json& json_res)
{
	std::wstring dst;
	auto target_workbook = utf2utf(json_req["workbook"], dst);
	auto target_worksheet = utf2utf(json_req["worksheet"], dst);
	bool isShowMaximize = json_req["maximize"].get<bool>();


	// ExcelのCLSIDを得る
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

	if (FAILED(hr)) {
		json_res["reason"] = "Failed to initialize Excel Application.";
		return false;
	}

	// 既存のExcel.Applicationインスタンスを取得する
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if(FAILED(hr)) {
		// 起動してない
		json_res["reason"] = "Excel is not running.";
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

		std::wstring workbookName = (const wchar_t*)name;

		if (workbookName != target_workbook) {
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

			std::wstring sheetName = (const wchar_t*)name;

			if (sheetName != target_worksheet) {
				continue;
			}

			// シートをアクティブにする
			VariantInit(&result);
			hr = AutoWrap(DISPATCH_PROPERTYGET, &result, sheet, L"Activate", 0);
			if (FAILED(hr)) {
				return FALSE;
			}

			// アプリのウインドウを全面に出す
			if (IsWindow(hwndApp) == FALSE) {
				break;
			}

			ScopeAttachThreadInput scope;
			LONG_PTR style = GetWindowLongPtr(hwndApp, GWL_STYLE);

			if (isShowMaximize && (style & WS_MAXIMIZE) == 0) {
				// 最大化表示する
				PostMessage(hwndApp, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
			}
			if (style & WS_MINIMIZE) {
				// 最小化されていたら元に戻す
				PostMessage(hwndApp, WM_SYSCOMMAND, SC_RESTORE, 0);
			}
			SetForegroundWindow(hwndApp);

			json_res["result"] = true;
			return true;
		}
	}

	json_res["result"] = false;
	json_res["reason"] = "sheet does not found.";
	return true;
}

} //"sheet does not found."end of namespace 



