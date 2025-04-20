#include "EnumCalcSheetsProxyCommand.h"
#include <windows.h>
#include "processproxy/share/AfxWFunctions.h"
#include "StringUtil.h"
#include "AutoWrap.h"
#include <oleauto.h>
#include <atlbase.h>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace launcherproxy { 

REGISTER_PROXYCOMMAND(EnumCalcSheetsProxyCommand)

EnumCalcSheetsProxyCommand::EnumCalcSheetsProxyCommand()
{
}

EnumCalcSheetsProxyCommand::~EnumCalcSheetsProxyCommand()
{
}

std::string EnumCalcSheetsProxyCommand::GetName()
{
	return "enumcalcsheets";
}

bool EnumCalcSheetsProxyCommand::Execute(json& json_req, json& json_res)
{
	std::vector<std::map<std::string, std::string> > emptyItems;

	// CalcのCLSIDを得る
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"com.sun.star.ServiceManager", &clsid);
	if (FAILED(hr)) {
		json_res["items"] = emptyItems;
		json_res["result"] = true;
		json_res["reason"] = "Failed to initialize Calc Application.";
		return true;
	}

	// ServiceManagerを生成する
	CComPtr<IUnknown> unkPtr;
	hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&unkPtr);
	if(FAILED(hr)) {
		// 起動してない
		json_res["items"] = emptyItems;
		json_res["result"] = true;
		json_res["reason"] = "Calc is not running.";
		return true;
	}

	DispWrapper serviceManager;
	unkPtr->QueryInterface(&serviceManager);

	// Desktopを生成する
	DispWrapper desktop;
	serviceManager.CallObjectMethod(L"createInstance", L"com.sun.star.frame.Desktop", desktop);

	// ドキュメントのリスト取得
	DispWrapper components;
	desktop.CallObjectMethod(L"getComponents", components);

	// Enum取得
	DispWrapper enumelation;
	components.CallObjectMethod(L"createEnumeration", enumelation);

	std::vector<std::map<std::string, std::string> > items;

	std::string tmp;
	std::string tmp2;

	for(int loop = 0; loop < 65536; ++loop) {   // 無限ループ防止

		// 要素がなければ終了
		if (enumelation.CallBooleanMethod(L"hasMoreElements", false) == false) {
			break;
		}

		DispWrapper doc;
		enumelation.CallObjectMethod(L"nextElement", doc);

		// ドキュメントのパスを取得
		std::wstring workbook_name = doc.CallStringMethod(L"getURL", _T(""));

		// 拡張子でファイル種別を判別する
		std::wstring fileExt{PathFindExtension(workbook_name.c_str())};
		if (fileExt != _T(".xlsx") && fileExt != _T(".xls") && fileExt != _T(".xlsm") && fileExt != _T(".ods")) {
			continue;
		}

		// シートオブジェクトを取得
		DispWrapper sheets;
		doc.CallObjectMethod(L"getSheets", sheets);

		// シート数を得る
		int sheetCount = sheets.CallIntMethod(L"getCount", 0);

		CComBSTR bstrVal;

		// シート名を列挙する
		for (int i = 0; i < sheetCount; ++i) {

			// シートオブジェクトを得る
			DispWrapper sheet;
			sheets.CallObjectMethod(L"getByIndex", i, sheet);

			// シート名を得る
			std::wstring worksheet_name = sheet.GetPropertyString(L"name");

			items.emplace_back(std::map<std::string, std::string>{
			    { "workbook", utf2utf(workbook_name, tmp) },
			    { "worksheet", utf2utf(worksheet_name, tmp2) },
			});
		}
	}

	json_res["result"] = true;
	json_res["items"] = items;

	return true;
}

} // end of namespace 



