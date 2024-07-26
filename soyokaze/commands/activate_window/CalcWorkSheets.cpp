#include "pch.h"
#include "CalcWorkSheets.h"
#include "commands/activate_window/AutoWrap.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/CharConverter.h"
#include <mutex>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
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
	uint64_t mLastUpdate = 0;
	std::vector<CalcWorksheet*> mCache;

	// 生成処理の排他制御
	std::mutex mMutex;

	int mStatus = STATUS_BUSY;
};

void CalcWorkSheets::PImpl::Update()
{
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mStatus = STATUS_BUSY;
	}

	std::thread th([&]() {
		HRESULT hr = CoInitialize(nullptr);
		if (FAILED(hr)) {
			SPDLOG_ERROR(_T("Failed to CoInitialize!"));

			std::lock_guard<std::mutex> lock(mMutex);
			mLastUpdate = GetTickCount64();
			mStatus = STATUS_READY;
			return ;
		}

		struct scope_uninit {
			~scope_uninit() {
				CoUninitialize();
			}
		};

		// ExcelのCLSIDを得る
		CLSID clsid;
		hr = CLSIDFromProgID(L"com.sun.star.ServiceManager", &clsid);
		if (FAILED(hr)) {
			// 取得できなかった(インストールされていないとか)
			std::lock_guard<std::mutex> lock(mMutex);
			mLastUpdate = GetTickCount64();
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

			mLastUpdate = GetTickCount64();
			mStatus = STATUS_READY;
			return ;
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

		std::vector<CalcWorksheet*> tmpList;

		bool isLoop = true;
		while(isLoop) {

			// 要素がなければ終了
			if (enumelation.CallBooleanMethod(L"hasMoreElements", false) == false) {
				break;
			}

			DispWrapper doc;
			enumelation.CallObjectMethod(L"nextElement", doc);

			// ドキュメントのパスを取得
			CString url = doc.CallStringMethod(L"getURL", _T(""));

			// 拡張子でファイル種別を判別する
			CString fileExt = PathFindExtension(url);
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

				Sleep(0);

				// シートオブジェクトを得る
				DispWrapper sheet;
				sheets.CallObjectMethod(L"getByIndex", i, sheet);

				// シート名を得る
				CString sheetName = sheet.GetPropertyString(L"name");

				tmpList.push_back(new CalcWorksheet(url, sheetName));
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

CalcWorkSheets::CalcWorkSheets() : in(std::make_unique<PImpl>())
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		SPDLOG_ERROR(_T("Failed to CoInitialize! err={:x}"), GetLastError());
	}

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


struct CalcWorksheet::PImpl
{
	HWND FindWindowHandle();
	// ブック名
	CString mBookName;
	CString mBookLocalName;
	// シート名
	CString mSheetName;
	// 参照カウント
	uint32_t mRefCount = 1;
	
};

HWND CalcWorksheet::PImpl::FindWindowHandle()
{
	struct local_param {
		static BOOL CALLBACK callback(HWND h, LPARAM lp) {
			auto param = (local_param*)lp;

			CString buf;
			GetWindowText(h, buf.GetBuffer(256), 256);
			buf.ReleaseBuffer();

			if  (buf.Find(param->mFileName) != -1) {
				param->mHwnd = h;
				return FALSE;
			}
			return TRUE;
		}

		HWND mHwnd = nullptr;
		CString mFileName;
	} param;
	param.mFileName = PathFindFileName(mBookLocalName);

	EnumWindows(local_param::callback, (LPARAM)&param);

	return param.mHwnd;
}


static CString DecodeURI(const CString& src)
{
	static tregex reg(_T("^.*%[0-9a-fA-F][0-9a-fA-F].*$"));
	if (std::regex_match(tstring(src), reg) == false) {
		// エンコード表現を含まない場合は何もしない
		return src;
	}

	CStringA srcA(src);

	std::string buf;

	int len = srcA.GetLength();
	for (int i = 0; i < len; ++i) {
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

		if ((c2 < '0' || '9' < c2) && (c2 < 'a' || 'f' < c2) && (c2 < 'A' || 'F' < c2)) {
			continue;
		}
		if ((c3 < '0' || '9' < c3) && (c3 < 'a' || 'f' < c3) && (c3 < 'A' || 'F' < c3)) {
			continue;
		}

		uint32_t n;
		sscanf_s(num, "%02x", &n);
		buf.push_back((char)n);

		i+=2;
	}

	CString out;
	launcherapp::utility::CharConverter conv;
	conv.Convert(buf.c_str(), out);

	return out;
}

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
	in->mBookName = workbookName;

	CString decoded = DecodeURI(workbookName);
	TCHAR localPath[MAX_PATH_NTFS];
	DWORD len = MAX_PATH_NTFS;
	PathCreateFromUrl(decoded, localPath, &len, NULL);
	in->mBookLocalName = localPath;

	in->mSheetName = sheetName;
}

CalcWorksheet::~CalcWorksheet()
{
}



const CString& CalcWorksheet::GetWorkbookName()
{
	return in->mBookLocalName;
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

	std::vector<CalcWorksheet*> tmpList;

	DispWrapper sheet;
	DispWrapper controller;

	// 該当するシートを探す
	bool isFoundSheet = false;

	bool isLoop = true;
	while(isLoop) {
		if (enumeration.CallBooleanMethod(L"hasMoreElements", false) == false) {
			break;
		}

		// 次の要素を取得
		DispWrapper doc;
		enumeration.CallObjectMethod(L"nextElement", doc);

		// ドキュメントのパスを取得
		CString url = doc.CallStringMethod(L"getURL", _T(""));

		// 探しているブックかどうか
		if (url != in->mBookName) {
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
			CString sheetName = sheet.GetPropertyString(L"name");
			if (sheetName != in->mSheetName) {
				continue;
			}

			// コントローラを得る
			doc.CallObjectMethod(L"getCurrentController", controller);

			isFoundSheet = true;
			isLoop = false;
			break;
		}
	}

	if (isFoundSheet == false) {
		return FALSE;
	}

	// アクティブなワークシートを変える
	controller.CallVoidMethod(L"setActiveSheet", sheet);

	HWND hwndApp = in->FindWindowHandle();

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
} // end of namespace launcherapp

