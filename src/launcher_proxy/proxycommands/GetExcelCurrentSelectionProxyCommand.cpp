#include "GetExcelCurrentSelectionProxyCommand.h"
#include <windows.h>
#include "StringUtil.h"
#include "AutoWrap.h"
#include <oleauto.h>
#include <atlbase.h>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace launcherproxy { 

REGISTER_PROXYCOMMAND(GetExcelCurrentSelectionProxyCommand)

GetExcelCurrentSelectionProxyCommand::GetExcelCurrentSelectionProxyCommand()
{
}

GetExcelCurrentSelectionProxyCommand::~GetExcelCurrentSelectionProxyCommand()
{
}

std::string GetExcelCurrentSelectionProxyCommand::GetName()
{
	return "getexcelcurrentselection";
}

bool GetExcelCurrentSelectionProxyCommand::Execute(json& json_req, json& json_res)
{
	// ExcelのCLSIDを得る
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

	if (FAILED(hr)) {
		// 取得できなかった(インストールされていないとか)
		json_res["result"] = false;
		json_res["reason"] = "Failed to initilze excel.";
		return true;
	}

	// 既存のExcel.Applicationインスタンスを取得する
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if(FAILED(hr)) {
		// 起動してない
		json_res["result"] = false;
		json_res["reason"] = "excel is not running.";
		return true;
	}

	DispWrapper excelApp;
	unkPtr->QueryInterface(&excelApp);

	std::string tmp;

	// ブック名を得る
	DispWrapper activeSheet;
	if (excelApp.GetPropertyObject(L"ActiveSheet", activeSheet)) {
		DispWrapper wb;
		if (activeSheet.GetPropertyObject(L"Parent", wb)) {
			std::wstring filePath = wb.GetPropertyString(L"Path"); 
			filePath += _T("\\");
			filePath += wb.GetPropertyString(L"Name");

			json_res["workbook"] = utf2utf(filePath, tmp);
		}

		// シート名を得る
		std::wstring sheetName = activeSheet.GetPropertyString(L"Name");
		json_res["worksheet"] = utf2utf(sheetName, tmp);
	}

	// 選択範囲を表すテキストを得る
	DispWrapper selection;
	if (excelApp.GetPropertyObject(L"Selection", selection)) {
		// 選択範囲を表すテキストを取得
		std::wstring addressStr = selection.GetPropertyString(L"Address");
		// $を除去
		addressStr.erase(std::remove(addressStr.begin(), addressStr.end(), L'$'), addressStr.end());

		json_res["address"] = utf2utf(addressStr, tmp);
	}	

	json_res["result"] = true;
	return true;
}

} // end of namespace 

