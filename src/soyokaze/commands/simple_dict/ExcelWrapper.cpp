#include "pch.h"
#include "ExcelWrapper.h"
#include "commands/common/AutoWrap.h"
#include "commands/common/DispWrapper.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"
#include "utility/TimeoutChecker.h"
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using json = nlohmann::json;


namespace launcherapp {
namespace commands {
namespace simple_dict {

constexpr int EMPTY_LIMIT = 20;   // この数だけ空白が連続で続いたら検索を打ち切る

using DispWrapper = launcherapp::commands::common::DispWrapper;
using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

struct ExcelApplication::PImpl
{
	DispWrapper mApp;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

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

		HWND hwndApp = (HWND)excelApp.GetPropertyInt64(L"Hwnd");
		if (IsWindow(hwndApp)) {
			DWORD pid;
			GetWindowThreadProcessId(hwndApp, &pid);
			spdlog::debug(_T("Excel PID is {}"), pid);
		}
		else {
			spdlog::debug(_T("Excel HWND is invalid {}"), (int64_t)hwndApp);
		}

		// 警告ダイアログをださない
		excelApp.CallVoidMethod(L"DisplayAlerts", false);

		return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ExcelApplication::ExcelApplication() : in(new PImpl)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		SPDLOG_ERROR(_T("Failed to CoInitialize!"));
	}
}


ExcelApplication::~ExcelApplication()
{
	Quit();

	in->mApp.Release();
	CoUninitialize();
}

bool ExcelApplication::IsInstalled()
{
	// ExcelのCLSIDを得る
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

	bool isInstalled = SUCCEEDED(hr);
	spdlog::info(_T("Excel isInstalled:{}"), isInstalled);

	return isInstalled;
}

/**
 	指定した範囲のセルのテキストを取得する
 	@return  0:正常終了 ほか:エラー
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
	SPDLOG_DEBUG(_T("args path:{0} sheet:{1} address:{2}"),
	             (LPCTSTR)wbPath, (LPCTSTR)sheetName, (LPCTSTR)address);

	DispWrapper& excelApp = in->mApp;
	if (!excelApp) {
		if (CreateExcelApplication(excelApp) == false) {
			SPDLOG_ERROR(_T("Failed to create app instance"));
			return -1;
		}
	}


	DispWrapper workBooks;
	if (excelApp.GetPropertyObject(L"WorkBooks", workBooks) == false) {
		SPDLOG_ERROR(_T("Failed to get WorkBooks"));
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
			SPDLOG_ERROR(_T("Failed to open workbook. path:{}"), (LPCTSTR)wbPath);
			return -3;
		}
	}
	workBook = result.pdispVal;

	struct local_close {
		local_close(DispWrapper& dispPtr) : mDisp(dispPtr) {}
		~local_close() {
			mDisp.CallVoidMethod(L"Close", false);
			SPDLOG_DEBUG(_T("Workbook closed."));
		}
		DispWrapper mDisp;
	} _close_(workBook);

	DispWrapper workSheets;
	if (workBook.GetPropertyObject(L"WorkSheets", workSheets) == false)  {
		SPDLOG_ERROR(_T("Failed to get WorkSheets"));
		return -4;
	}

	DispWrapper workSheet;
	if (workSheets.GetPropertyObject(L"Item", (LPOLESTR)(LPCOLESTR)sheetName, workSheet) == false) {
		SPDLOG_ERROR(_T("Failed to get Item"));
		return -5;
	}

	DispWrapper range;
	if (workSheet.GetPropertyObject(L"Range", (LPOLESTR)(LPCOLESTR)address, range) == false) {
		SPDLOG_ERROR(_T("Failed to get Range"));
		return -6;
	}

	DispWrapper rows;
	if (range.GetPropertyObject(L"Rows", rows) == false) {
		SPDLOG_ERROR(_T("Failed to get Rows"));
		return -7;
	}

	DispWrapper cols;
	if (range.GetPropertyObject(L"Columns", cols) == false) {
		SPDLOG_ERROR(_T("Failed to get Columns"));
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
				SPDLOG_ERROR(_T("Failed to get Item row,col:({0},{1})"), row, col);
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
	SPDLOG_DEBUG(_T("Completed. path:{0} sheet:{1} address:{2}"),
	             (LPCTSTR)wbPath, (LPCTSTR)sheetName, (LPCTSTR)address);
	return 0;
}

void ExcelApplication::Quit()
{
	bool isInitialized = (((IDispatch*)in->mApp) != nullptr);
	if (isInitialized == false) {
		// 終了不要
		return;
	}
			
	// Quitしても終わらないときは強制終了させるので、プロセスIDを取得しておく
	DWORD pid = 0;
	HWND hwndApp = (HWND)in->mApp.GetPropertyInt64(L"Hwnd");
	if (IsWindow(hwndApp)) {
		GetWindowThreadProcessId(hwndApp, &pid);
	}

	in->mApp.CallVoidMethod(L"Quit");

	if (pid == 0) {
		return;
	}

	// 2秒まってもプロセスが終了していなかったら強制的に落とす
	HANDLE h = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, pid);
	std::thread th([pid, h]() {
			if (WaitForSingleObject(h, 2000) != WAIT_TIMEOUT) {
				return;
			}
			spdlog::debug(_T("Excel app terminated PID:{}"), pid);
			TerminateProcess(h, 0);
	});
	th.detach();
}

bool ExcelApplication::GetSelection(CString* wbPath, CString* sheetName, CString* address)
{
	json json_req;
	json_req["command"] = "getexcelcurrentselection";

	auto proxy = NormalPriviledgeProcessProxy::GetInstance();

	// リクエストを送信する
	json json_res;
	if (proxy->SendRequest(json_req, json_res) == false) {
		return false;
	}
	
	if (json_res["result"] == false) {
		return false;
	}

	std::wstring workbook;
	UTF2UTF(json_res["workbook"].get<std::string>(), workbook);
	std::wstring worksheet;
	UTF2UTF(json_res["worksheet"].get<std::string>(), worksheet);
	std::wstring address_str;
	UTF2UTF(json_res["address"].get<std::string>(), address_str);

	if (wbPath) {
		*wbPath = workbook.c_str();
	}
	if (sheetName) {
		*sheetName = worksheet.c_str();
	}
	if (address) {
		*address = address_str.c_str();
	}

	return true;
}

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

