#include "pch.h"
#include "ExcelWorkSheets.h"
#include "commands/common/AutoWrap.h"
#include "utility/ScopeAttachThreadInput.h"
#include <mutex>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace activate_worksheet {

using namespace launcherapp::commands::common;

enum {
	STATUS_READY,   // 待機状態
	STATUS_BUSY,   //  更新中
};

struct WorkSheets::PImpl
{
	void Update();

	bool IsBusy() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mStatus == STATUS_BUSY;
	}

	// 前回の取得時のタイムスタンプ
	uint64_t mLastUpdate{0};
	std::vector<Worksheet*> mCache;

	// 生成処理の排他制御
	std::mutex mMutex;

	int mStatus= STATUS_BUSY;
};

void WorkSheets::PImpl::Update()
{
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mStatus = STATUS_BUSY;
	}

	std::thread th([&]() {
		// ExcelのCLSIDを得る
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

		if (FAILED(hr)) {
			// 取得できなかった(インストールされていないとか)
			std::lock_guard<std::mutex> lock(mMutex);
			mLastUpdate = GetTickCount64();
			mStatus = STATUS_READY;
			return ;
		}

		// 既存のExcel.Applicationインスタンスを取得する
		CComPtr<IUnknown> unkPtr;
		hr = GetActiveObject(clsid, NULL, &unkPtr);
		if(FAILED(hr)) {
			// 起動してない
			std::lock_guard<std::mutex> lock(mMutex);

			for (auto& item : mCache) {
				item->Release();
			}
			mCache.clear();

			mLastUpdate = GetTickCount64();
			mStatus = STATUS_READY;
			return ;
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
					name = result.bstrVal;
				}

				CString sheetName = name;

				tmpList.push_back(new Worksheet(appPath, workbookName, sheetName));
			}
		}

		std::lock_guard<std::mutex> lock(mMutex);
		mCache.swap(tmpList);
		mLastUpdate = GetTickCount64();
		mStatus = STATUS_READY;

		for (auto& item : tmpList) {
			item->Release();
		}
	});
	th.detach();
}

WorkSheets::WorkSheets() : in(std::make_unique<PImpl>())
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		SPDLOG_ERROR(_T("Failed to CoInitialize!"));
	}

	in->mLastUpdate = 0;
	in->mStatus = STATUS_READY;
}

WorkSheets::~WorkSheets()
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	for (auto& elem : in->mCache) {
		elem->Release();
	}
	in->mCache.clear();

	CoUninitialize();
}

// この時間以内に再実行されたら、前回の結果を再利用する
constexpr int INTERVAL_REUSE = 5000;

bool WorkSheets::GetWorksheets(std::vector<Worksheet*>& worksheets)
{
	// 前回取得時から一定時間経過していない場合は前回の結果を再利用する
	uint64_t elapsed = GetTickCount64() - in->mLastUpdate;
	if (elapsed < INTERVAL_REUSE) {

		std::lock_guard<std::mutex> lock(in->mMutex);

		worksheets = in->mCache;
		for (auto& elem : worksheets) {
			elem->AddRef();
		}
		return true;
	}

	if (in->IsBusy()) {
		return false;
	}

	// Excelのシート一覧を更新する
	in->Update();
	return false;
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
	uint32_t mRefCount{1};
	
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
	in(std::make_unique<PImpl>())
{
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
BOOL Worksheet::Activate(bool isShowMaximize)
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
			VariantInit(&result);
			hr = AutoWrap(DISPATCH_PROPERTYGET, &result, sheet, L"Activate", 0);
			if (FAILED(hr)) {
				return FALSE;
			}

			// アプリのウインドウを全面に出す
			if (IsWindow(hwndApp)) {
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

} // end of namespace activate_worksheet
} // end of namespace commands
} // end of namespace launcherapp

