#include "ActiveCalcSheetProxyCommand.h"
#include <windows.h>
#include "processproxy/share/AfxWFunctions.h"
#include "ScopeAttachThreadInput.h"
#include "StringUtil.h"
#include "AutoWrap.h"
#include <oleauto.h>
#include <atlbase.h>
#include <spdlog/spdlog.h>
#include <regex>

using json = nlohmann::json;

namespace launcherproxy { 


static std::wstring DecodeURI(const std::wstring& src);

static HWND FindWindowHandle(const wchar_t* filePath)
{
	struct local_param {
		static BOOL CALLBACK callback(HWND h, LPARAM lp) {
			auto param = (local_param*)lp;

			std::wstring buf;
			buf.resize(255);
			GetWindowText(h, buf.data(), 255+1);

			if  (buf.find(param->mFileName.c_str()) != std::wstring::npos) {
				param->mHwnd = h;
				return FALSE;
			}
			return TRUE;
		}

		HWND mHwnd = nullptr;
		std::wstring mFileName;
	} param;
	param.mFileName = PathFindFileName(filePath);

	EnumWindows(local_param::callback, (LPARAM)&param);

	return param.mHwnd;
}

REGISTER_PROXYCOMMAND(ActiveCalcSheetProxyCommand)

ActiveCalcSheetProxyCommand::ActiveCalcSheetProxyCommand()
{
}

ActiveCalcSheetProxyCommand::~ActiveCalcSheetProxyCommand()
{
}

std::string ActiveCalcSheetProxyCommand::GetName()
{
	return "activecalcsheet";
}

bool ActiveCalcSheetProxyCommand::Execute(json& json_req, json& json_res)
{
	std::wstring dst;
	auto target_workbook = utf2utf(json_req["workbook"], dst);
	auto target_worksheet = utf2utf(json_req["worksheet"], dst);
	bool isShowMaximize = json_req["maximize"].get<bool>();


	// CalcのCLSIDを得る
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"com.sun.star.ServiceManager", &clsid);
	if (FAILED(hr)) {
		json_res["result"] = true;
		json_res["reason"] = "Failed to initialize Calc Application.";
		return false;
	}

	// ServiceManagerを生成する
	CComPtr<IUnknown> unkPtr;
	hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&unkPtr);
	if(FAILED(hr)) {
		json_res["result"] = true;
		json_res["reason"] = "Calc is not running.";
		return FALSE;
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
	DispWrapper enumeration;
	components.CallObjectMethod(L"createEnumeration",enumeration); 

	DispWrapper sheet;
	DispWrapper controller;

	// 該当するシートを探す
	bool isFoundSheet = false;
	for (int loop = 0; isFoundSheet == false && loop < 65536; ++loop) {

		if (enumeration.CallBooleanMethod(L"hasMoreElements", false) == false) {
			break;
		}

		// 次の要素を取得
		DispWrapper doc;
		enumeration.CallObjectMethod(L"nextElement", doc);

		// ドキュメントのパスを取得
		std::wstring url{doc.CallStringMethod(L"getURL", _T(""))};

		// 探しているブックかどうか
		if (url != target_workbook) {
			continue;
		}

		// シートオブジェクトを取得
		DispWrapper sheets;
		doc.CallObjectMethod(L"getSheets", sheets);

		// シート数を得る
		int sheetCount = sheets.CallIntMethod(L"getCount", 0);

		// シート名を列挙する
		for (int i = 0; i < sheetCount; ++i) {

			sheets.CallObjectMethod(L"getByIndex", i, sheet);

			// シート名を得る
			std::wstring sheetName{sheet.GetPropertyString(L"name")};
			if (sheetName != target_worksheet) {
				continue;
			}

			// コントローラを得る
			doc.CallObjectMethod(L"getCurrentController", controller);

			isFoundSheet = true;
			break;
		}
	}

	if (isFoundSheet == false) {
		json_res["result"] = true;
		json_res["reason"] = "sheet does not found.";
		return FALSE;
	}

	// アクティブなワークシートを変える
	controller.CallVoidMethod(L"setActiveSheet", sheet);

	HWND hwndApp = FindWindowHandle(DecodeURI(target_workbook).c_str());

	// アプリのウインドウを全面に出す
	if (IsWindow(hwndApp) == FALSE) {
		json_res["result"] = false;
		json_res["reason"] = fmt::format("App window does not found. {0}", json_req["workbook"].get<std::string>());
		return true;
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


std::wstring DecodeURI(const std::wstring& src)
{
	static std::wregex reg(L"^.*%[0-9a-fA-F][0-9a-fA-F].*$");
	if (std::regex_match(src, reg) == false) {
		// エンコード表現を含まない場合は何もしない
		return src;
	}

	std::string srcA;
	utf2utf(src, srcA);

	std::string buf;

	size_t len = srcA.size();
	for (size_t i = 0; i < len; ++i) {
		char c = srcA[i];

		if (c != '%') {
			buf.push_back(c);
			continue;
		}

		if (i +2 >= len) {
			buf.push_back(c);
			continue;
		}

		char num[3] = { srcA[i+1], srcA[i+2], 0x00 };
		char& c2 = num[0];
		char& c3 = num[1];

		if (_istxdigit(c2) == 0 || _istxdigit(c3) == 0) {
			continue;
		}

		uint32_t n;
		sscanf_s(num, "%02x", &n);
		buf.push_back((char)n);

		i+=2;
	}

	std::wstring out;
	return utf2utf(buf, out);
}

} // end of namespace 



