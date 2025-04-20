#include "pch.h"
#include "ExcelWorkSheets.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"
#include "utility/ScopeAttachThreadInput.h"
#include <mutex>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace activate_worksheet {

using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

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
	if (IsBusy()) {
		// 既に実行中
		return;
	}

	{
		std::lock_guard<std::mutex> lock(mMutex);
		mStatus = STATUS_BUSY;
	}

	auto threadFunc= [&]() {

		std::vector<std::pair<std::wstring, std::wstring> > sheets;
		auto proxy = NormalPriviledgeProcessProxy::GetInstance();
		proxy->EnumExcelSheets(sheets);

		std::vector<Worksheet*> tmpList;
		for (auto& item : sheets) {
			tmpList.push_back(new Worksheet(item.first, item.second));
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mCache.swap(tmpList);
		mLastUpdate = GetTickCount64();
		mStatus = STATUS_READY;

		for (auto& item : tmpList) {
			item->Release();
		}
	};

	if (mLastUpdate == 0) {
		// 初回は同期で取得
		threadFunc();
		return;
	}
	else {
		// 2回目以降は非同期で取得
		std::thread th(threadFunc);
		th.detach();
	}
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
	if (INTERVAL_REUSE <= elapsed) {
		// Excelのシート一覧を更新する
		in->Update();
	}

	std::lock_guard<std::mutex> lock(in->mMutex);

	worksheets = in->mCache;
	for (auto& elem : worksheets) {
		elem->AddRef();
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct Worksheet::PImpl
{
	// ブック名
	std::wstring mBookName;
	// シート名
	std::wstring mSheetName;
	// 参照カウント
	uint32_t mRefCount{1};
	
};

/**
 	コンストラクタ
 	@param[in] workbookName ワークブック名
 	@param[in] sheetName    シート名
*/
 Worksheet::Worksheet(
	const std::wstring& workbookName,
	const std::wstring& sheetName
) : 
	in(std::make_unique<PImpl>())
{
	in->mBookName = workbookName;
	in->mSheetName = sheetName;
}

Worksheet::~Worksheet()
{
}

const std::wstring& Worksheet::GetWorkbookName()
{
	return in->mBookName;
}

const std::wstring& Worksheet::GetSheetName()
{
	return in->mSheetName;
}

/**
 	オブジェクトに紐づけられたワークシートを有効にする
 	@return 処理の成否
*/
BOOL Worksheet::Activate(bool isShowMaximize)
{
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	return proxy->ActiveExcelSheet(in->mBookName, in->mSheetName, isShowMaximize);
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

