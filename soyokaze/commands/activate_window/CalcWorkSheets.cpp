#include "pch.h"
#include "CalcWorkSheets.h"
#include "commands/activate_window/AutoWrap.h"
#include "utility/ScopeAttachThreadInput.h"
#include <mutex>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace activate_window {


enum {
	STATUS_READY,   // 待機状態
	STATUS_BUSY,   //  更新中
};

struct CalcWorkSheets::PImpl
{
	void Update();

	bool IsBusy() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mStatus == STATUS_BUSY;
	}

	// 前回の取得時のタイムスタンプ
	DWORD mLastUpdate;
	std::vector<CalcWorksheet*> mCache;

	// 生成処理の排他制御
	std::mutex mMutex;

	int mStatus;
};

void CalcWorkSheets::PImpl::Update()
{
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mStatus = STATUS_BUSY;
	}

	std::thread th([&]() {
		// ExcelのCLSIDを得る
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(L"com.sun.star.ServiceManager", &clsid);

		if (FAILED(hr)) {
			// 取得できなかった(インストールされていないとか)
			std::lock_guard<std::mutex> lock(mMutex);
			mLastUpdate = GetTickCount();
			mStatus = STATUS_READY;
			return ;
		}

		// ServiceManagerを生成する
		CComPtr<IUnknown> unkPtr;
		hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&unkPtr);
		if(FAILED(hr)) {
			// 取得できなかった
			std::lock_guard<std::mutex> lock(mMutex);

			for (auto& item : mCache) {
				item->Release();
			}
			mCache.clear();

			mLastUpdate = GetTickCount();
			mStatus = STATUS_READY;
			return ;
		}

		CComPtr<IDispatch> serviceManager;
		unkPtr->QueryInterface(&serviceManager);

		VARIANT result;
		// Desktopを生成する
		CComPtr<IDispatch> desktop;
		{
			CComBSTR argVal(L"com.sun.star.frame.Desktop");
			VARIANT arg1;
			VariantInit(&arg1);
			arg1.vt = VT_BSTR;
			arg1.bstrVal = argVal;

			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, serviceManager, L"createInstance", 1, &arg1);
			desktop = result.pdispVal;
		}

		// ドキュメントのリスト取得
		CComPtr<IDispatch> components;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, desktop, L"getComponents", 0);
			components = result.pdispVal;
		}

		// Enum取得
		CComPtr<IDispatch> enumulation;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, components, L"createEnumeration", 0);
			enumulation = result.pdispVal;
		}

		std::vector<CalcWorksheet*> tmpList;

		bool isLoop = true;
		while(isLoop) {
			{
				VariantInit(&result);
				AutoWrap(DISPATCH_METHOD, &result, enumulation, L"hasMoreElements", 0);
				if (!result.boolVal) {
					break;
				}
			}

			CComPtr<IDispatch> doc;
			{
				VariantInit(&result);
				AutoWrap(DISPATCH_METHOD, &result, enumulation, L"nextElement", 0);
				doc = result.pdispVal;
			}

			// ドキュメントのパスを取得
			CComBSTR bstrVal;
			{
				VariantInit(&result);
				AutoWrap(DISPATCH_METHOD, &result, doc, L"getURL", 0);
				bstrVal = result.bstrVal;
			}
			CString url = bstrVal;

			// 拡張子でファイル種別を判別する
			CString fileExt = PathFindExtension(url);
			if (fileExt != _T(".xlsx") && fileExt != _T(".xls") && fileExt != _T(".xlsm") && fileExt != _T(".ods")) {
				continue;
			}

			// シートオブジェクトを取得
			CComPtr<IDispatch> sheets;
			{
				VariantInit(&result);
				AutoWrap(DISPATCH_METHOD, &result, doc, L"getSheets", 0);
				sheets = result.pdispVal;
			}

			// シート数を得る
			int sheetCount = 0;
			{
				VariantInit(&result);
				AutoWrap(DISPATCH_METHOD, &result, sheets, L"getCount", 0);
				sheetCount = result.intVal;
			}

			// シート名を列挙する
			for (int i = 0; i < sheetCount; ++i) {

				VARIANT arg1;
				VariantInit(&arg1);
				arg1.vt = VT_INT;
				arg1.intVal = i;

				CComPtr<IDispatch> sheet;
				VariantInit(&result);
				AutoWrap(DISPATCH_METHOD, &result, sheets, L"getByIndex", 1, &arg1);
				sheet = result.pdispVal;

				// シート名を得る
				VariantInit(&result);
				AutoWrap(DISPATCH_PROPERTYGET, &result, sheet, L"name", 0);
				bstrVal = result.bstrVal;

				CString sheetName = bstrVal;

				tmpList.push_back(new CalcWorksheet(url, sheetName));
			}
		}

		std::lock_guard<std::mutex> lock(mMutex);
		mCache.swap(tmpList);
		mLastUpdate = GetTickCount();
		mStatus = STATUS_READY;

		for (auto& item : tmpList) {
			item->Release();
		}
	});
	th.detach();
}

CalcWorkSheets::CalcWorkSheets() : in(std::make_unique<PImpl>())
{
	CoInitialize(NULL);

	in->mLastUpdate = 0;
	in->mStatus = STATUS_READY;
}

CalcWorkSheets::~CalcWorkSheets()
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

bool CalcWorkSheets::GetWorksheets(std::vector<CalcWorksheet*>& worksheets)
{
	// 前回取得時から一定時間経過していない場合は前回の結果を再利用する
	DWORD elapsed = GetTickCount() - in->mLastUpdate;
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


struct CalcWorksheet::PImpl
{
	// ブック名
	CString mBookName;
	// シート名
	CString mSheetName;
	// 参照カウント
	uint32_t mRefCount;
	
};

/**
 	コンストラクタ
 	@param[in] workbookName ワークブック名
 	@param[in] sheetName    シート名
*/
 CalcWorksheet::CalcWorksheet(
	const CString& workbookName,
	const CString& sheetName
) : 
	in(std::make_unique<PImpl>())
{
	in->mRefCount = 1;

	in->mBookName = workbookName;
	in->mSheetName = sheetName;
}

CalcWorksheet::~CalcWorksheet()
{
}

const CString& CalcWorksheet::GetWorkbookName()
{
	// ToDo: URLをファイルパスに変換する
	return in->mBookName;
}

const CString& CalcWorksheet::GetSheetName()
{
	return in->mSheetName;
}

/**
 	オブジェクトに紐づけられたワークシートを有効にする
 	@return 処理の成否
*/
BOOL CalcWorksheet::Activate(bool isShowMaximize)
{
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"com.sun.star.ServiceManager", &clsid);

	if (FAILED(hr)) {
		// 初期化できなかった(たぶん起こらない。このオブジェクト作れてる時点でEXCELは入ってるはずなので)
		return FALSE;
	}

	// ServiceManagerを生成する
	CComPtr<IUnknown> unkPtr;
	hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&unkPtr);
	if(FAILED(hr)) {
		return FALSE;
	}

	CComPtr<IDispatch> serviceManager;
	unkPtr->QueryInterface(&serviceManager);

	VARIANT result;

	// Desktopを生成する
	CComPtr<IDispatch> desktop;
	{
		CComBSTR argVal(_T("com.sun.star.frame.Desktop"));
		VARIANT arg1;
		VariantInit(&arg1);
		arg1.vt = VT_BSTR;
		arg1.bstrVal = argVal;

		VariantInit(&result);
		AutoWrap(DISPATCH_METHOD, &result, serviceManager, L"createInstance", 1, &arg1);
		desktop = result.pdispVal;
	}

	// ドキュメントのリスト取得
	CComPtr<IDispatch> components;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_METHOD, &result, desktop, L"getComponents", 0);
		components = result.pdispVal;
	}

	// Enum取得
	CComPtr<IDispatch> enumulation;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_METHOD, &result, components, L"createEnumeration", 0);
		enumulation = result.pdispVal;
	}

	std::vector<CalcWorksheet*> tmpList;

	CComPtr<IDispatch> sheet;
	CComPtr<IDispatch> controller;

	// 該当するシートを探す
	bool isFoundSheet = false;

	bool isLoop = true;
	while(isLoop) {
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, enumulation, L"hasMoreElements", 0);
			if (!result.boolVal) {
				break;
			}
		}

		CComPtr<IDispatch> doc;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, enumulation, L"nextElement", 0);
			doc = result.pdispVal;
		}

		// ドキュメントのパスを取得
		CComBSTR bstrVal;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, doc, L"getURL", 0);
			bstrVal = result.bstrVal;
		}
		CString url = bstrVal;
		if (url != in->mBookName) {
			continue;
		}

		// シートオブジェクトを取得
		CComPtr<IDispatch> sheets;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, doc, L"getSheets", 0);
			sheets = result.pdispVal;
		}

		// シート数を得る
		int sheetCount = 0;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, sheets, L"getCount", 0);
			sheetCount = result.intVal;
		}

		// シート名を列挙する
		for (int i = 0; i < sheetCount; ++i) {

			VARIANT arg1;
			VariantInit(&arg1);
			arg1.vt = VT_INT;
			arg1.intVal = i;

			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, sheets, L"getByIndex", 1, &arg1);
			sheet = result.pdispVal;

			// シート名を得る
			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, sheet, L"name", 0);
			bstrVal = result.bstrVal;

			CString sheetName = bstrVal;
			if (sheetName != in->mSheetName) {
				continue;
			}

			// コントローラを得る
			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, doc, L"getCurrentController", 0);
			controller = result.pdispVal;

			isFoundSheet = true;
			isLoop = false;
			break;
		}
	}

	if (isFoundSheet == false) {
		return FALSE;
	}

	// アクティブなワークシートを変える

	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_DISPATCH;
	arg1.pdispVal = (IDispatch*)controller;

	VariantInit(&result);
	AutoWrap(DISPATCH_METHOD, &result, controller, L"setActiveSheet", 1, &arg1);

	// アプリのウインドウを全面に出す
	//if (IsWindow(hwndApp)) {
	//	ScopeAttachThreadInput scope;
	//	LONG_PTR style = GetWindowLongPtr(hwndApp, GWL_STYLE);

	//	if (isShowMaximize && (style & WS_MAXIMIZE) == 0) {
	//		// 最大化表示する
	//		PostMessage(hwndApp, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	//	}
	//	if (style & WS_MINIMIZE) {
	//		// 最小化されていたら元に戻す
	//		PostMessage(hwndApp, WM_SYSCOMMAND, SC_RESTORE, 0);
	//	}
	//	SetForegroundWindow(hwndApp);
	//}
	return TRUE;
}

uint32_t CalcWorksheet::AddRef()
{
	return ++in->mRefCount;
}

uint32_t CalcWorksheet::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace soyokaze

