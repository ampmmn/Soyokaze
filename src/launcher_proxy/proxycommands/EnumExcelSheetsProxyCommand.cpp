#include "EnumExcelSheetsProxyCommand.h"
#include <windows.h>
#include "StringUtil.h"
#include "AutoWrap.h"
#include <oleauto.h>
#include <atlbase.h>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace launcherproxy { 

REGISTER_PROXYCOMMAND(EnumExcelSheetsProxyCommand)

EnumExcelSheetsProxyCommand::EnumExcelSheetsProxyCommand()
{
}

EnumExcelSheetsProxyCommand::~EnumExcelSheetsProxyCommand()
{
}

std::string EnumExcelSheetsProxyCommand::GetName()
{
	return "enumexcelsheets";
}

bool EnumExcelSheetsProxyCommand::Execute(json& json_req, json& json_res)
{
	std::vector<std::map<std::string, std::string> > emptyItems;

	// ExcelのCLSIDを得る
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

	if (FAILED(hr)) {
		json_res["items"] = emptyItems;
		json_res["result"] = true;
		json_res["reason"] = "Failed to initialize Excel Application.";
		return true;
	}

	// 既存のExcel.Applicationインスタンスを取得する
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if(FAILED(hr)) {
		// 起動してない
		json_res["items"] = emptyItems;
		json_res["result"] = true;
		json_res["reason"] = "Excel is not running.";
		return true;
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

	CComBSTR workbook_name;
	CComBSTR worksheet_name;
	std::vector<std::map<std::string, std::string> > items;

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

			workbook_name = result.bstrVal;
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

		std::string tmp;
		std::string tmp2;

		for (int j = 0; j < sheetCount; ++j) {

			Sleep(0);

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
				worksheet_name = result.bstrVal;
			}

			items.emplace_back(std::map<std::string, std::string>{
			    { "workbook", utf2utf((const wchar_t*)workbook_name, tmp) },
					{ "worksheet", utf2utf((const wchar_t*)worksheet_name, tmp2) },
			});
		}
	}

	json_res["result"] = true;
	json_res["items"] = items;

	return true;
}

} // end of namespace 



