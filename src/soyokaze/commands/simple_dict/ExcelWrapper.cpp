#include "pch.h"
#include "ExcelWrapper.h"
#include "commands/common/AutoWrap.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/TimeoutChecker.h"
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace simple_dict {

constexpr int EMPTY_LIMIT = 20;   // この数だけ空白が連続で続いたら検索を打ち切る

using DispWrapper = launcherapp::commands::common::DispWrapper;

struct ExcelApplication::PImpl
{
	DispWrapper mApp;
	bool mIsGetObject{false};

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static bool GetExcelApplication(DispWrapper& excelApp)
{
		// ExcelのCLSIDを得る
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

		if (FAILED(hr)) {
			// 取得できなかった(インストールされていないとか)
			spdlog::warn(_T("Excel is not installed."));
			return false;
		}

		// 既存のExcel.Applicationインスタンスを取得する
		CComPtr<IUnknown> unkPtr;
		hr = GetActiveObject(clsid, NULL, &unkPtr);
		if(FAILED(hr)) {
			// 起動してない
			spdlog::warn(_T("Excel is not running."));
			return false;
		}

		hr = unkPtr->QueryInterface(&excelApp);
		if (FAILED(hr)) {
			spdlog::error(_T("Excel is not running."));
			return false;
		}

		// デバッグ用にPIDを出力しておく
		HWND hwndApp = (HWND)excelApp.GetPropertyInt64(L"Hwnd");
		if (IsWindow(hwndApp)) {
			DWORD pid;
			GetWindowThreadProcessId(hwndApp, &pid);
			spdlog::debug(_T("Excel PID is {}"), pid);
		}
		else {
			spdlog::debug(_T("Excel HWND is invalid {}"), (int64_t)hwndApp);
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

ExcelApplication::ExcelApplication(bool isGetObject) : in(new PImpl)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		SPDLOG_ERROR(_T("Failed to CoInitialize!"));
	}

	if (isGetObject) {
		GetExcelApplication(in->mApp);
		in->mIsGetObject = true;
	}
}


ExcelApplication::~ExcelApplication()
{
	bool isExist = (((IDispatch*)in->mApp) != nullptr);
	if (isExist == false) {
		CoUninitialize();
		return;
	}

	// GetObjectで確保したインスタンスでない(→自分でCreateInstanceした)場合はQuitで終了する
	if (in->mIsGetObject == false) {
		Quit();
	}

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

bool ExcelApplication::IsAvailable()
{
	SPDLOG_DEBUG(_T("start"));

	DispWrapper excelApp;
	if (GetExcelApplication(excelApp) == false) {
		SPDLOG_DEBUG(_T("unavailable"));
		return false;
	}

	DispWrapper activeSheet; 
	return excelApp.GetPropertyObject(L"ActiveSheet", activeSheet);
}

bool ExcelApplication::GetFilePath(CString& filePath)
{
	DispWrapper& excelApp = in->mApp;

	DispWrapper activeSheet;
	if (excelApp.GetPropertyObject(L"ActiveSheet", activeSheet) == false) {
		SPDLOG_ERROR(_T("Failed to get Active sheet"));
		return false;
	}

	DispWrapper wb;
	if (activeSheet.GetPropertyObject(L"Parent", wb) == false) {
		SPDLOG_ERROR(_T("Failed to get Parent"));
		return false;
	}

	filePath = wb.GetPropertyString(L"Path");
	filePath += _T("\\");
	filePath += wb.GetPropertyString(L"Name");

	return true;
}

CString ExcelApplication::GetActiveSheetName()
{
	DispWrapper& excelApp = in->mApp;

	DispWrapper activeSheet;
	if (excelApp.GetPropertyObject(L"ActiveSheet", activeSheet) == false) {
		SPDLOG_ERROR(_T("Failed to get active sheet"));
		return _T("");
	}

	return activeSheet.GetPropertyString(L"Name");
}

CString ExcelApplication::GetSelectionAddress(int& cols, int& rows)
{
	DispWrapper& excelApp = in->mApp;

	DispWrapper selection;
	if (excelApp.GetPropertyObject(L"Selection", selection) == false) {
		return _T("");
	}

	// 選択範囲を表すテキストを取得
	CString addressStr = selection.GetPropertyString(L"Address");

	// 選択範囲の行数・列数を取得する
	DispWrapper rowsObj;
	if (selection.GetPropertyObject(L"Rows", rowsObj) == false) {
		return _T("");
	}

	DispWrapper columnsObj;
	if (selection.GetPropertyObject(L"Columns", columnsObj) == false) {
		return _T("");
	}

	rows = rowsObj.GetPropertyInt(L"Count");
	cols = columnsObj.GetPropertyInt(L"Count");


	addressStr.Replace(_T("$"), _T(""));

	return addressStr;
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
	HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE, FALSE, pid);
	std::thread th([pid, h]() {
			utility::TimeoutChecker ch(2000);
			DWORD exitCode = 0;
			while(ch.IsTimeout() == false) {
			if (GetExitCodeProcess(h, &exitCode) && exitCode != STILL_ACTIVE) {
			spdlog::debug(_T("Excel app exit PID:{}"), pid);
			return;
			}
			Sleep(50);
			}
			spdlog::debug(_T("Excel app terminated PID:{}"), pid);
			TerminateProcess(h, 0);
			});
	th.detach();
}

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

