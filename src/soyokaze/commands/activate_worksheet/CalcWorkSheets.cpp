#include "pch.h"
#include "CalcWorkSheets.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"
#include "utility/Path.h"
#include <mutex>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using json = nlohmann::json;

namespace launcherapp {
namespace commands {
namespace activate_worksheet {

using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

enum {
	STATUS_READY,   // 待機状態
	STATUS_BUSY,   //  更新中
};

struct CalcWorkSheets::PImpl
{
	void Update();
	bool EnumCalcSheets(std::vector<std::pair<std::wstring, std::wstring> >& sheets);

	bool IsBusy() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mStatus == STATUS_BUSY;
	}

	// 前回の取得時のタイムスタンプ
	uint64_t mLastUpdate{0};
	std::vector<CalcWorksheet*> mCache;

	// 生成処理の排他制御
	std::mutex mMutex;

	int mStatus{STATUS_BUSY};
};

void CalcWorkSheets::PImpl::Update()
{
	if (IsBusy()) {
		// 既に実行中
		return;
	}

	{
		std::lock_guard<std::mutex> lock(mMutex);
		mStatus = STATUS_BUSY;
	}

	auto threadFunc = ([&]() {
		std::vector<std::pair<std::wstring, std::wstring> > sheets;
		EnumCalcSheets(sheets);

		std::vector<CalcWorksheet*> tmpList;
		for (auto& item : sheets) {
			tmpList.push_back(new CalcWorksheet(item.first, item.second));
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mCache.swap(tmpList);
		mLastUpdate = GetTickCount64();
		mStatus = STATUS_READY;

		for (auto& item : tmpList) {
			item->Release();
		}
	});

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

bool CalcWorkSheets::PImpl::EnumCalcSheets(std::vector<std::pair<std::wstring, std::wstring> >& sheets)
{
	try {
		std::string dst;

		json json_req;
		json_req["command"] = "enumcalcsheets";
		
		json json_res;

		auto proxy = NormalPriviledgeProcessProxy::GetInstance();
		if (proxy->SendRequest(json_req, json_res) == false) {
			SPDLOG_ERROR("Failed to receive response.");
			return false;
		}

		if (json_res["result"] == false) {
			SPDLOG_WARN("agent returned result:false");
			return false;
		}

		std::wstring tmp_wb;
		std::wstring tmp_ws;

		auto items = json_res["items"];
		for (auto& item : items) {
			auto workbook = item["workbook"].get<std::string>();
			auto worksheet = item["worksheet"].get<std::string>();
			sheets.push_back(std::make_pair(UTF2UTF(workbook, tmp_wb), UTF2UTF(worksheet, tmp_ws)));
		}

		return true;
	}
	catch(...) {
		spdlog::error("[EnumCalcSheets] Unexpected exception occurred.");
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


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
	if (INTERVAL_REUSE <= elapsed) {
		// シート一覧を更新する
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


struct CalcWorksheet::PImpl
{
	// ブック名
	std::wstring mBookName;
	std::wstring mBookLocalName;
	// シート名
	std::wstring mSheetName;
	//
	String mErrMsg;
	// 参照カウント
	uint32_t mRefCount{1};
	
};


static std::wstring DecodeURI(const std::wstring& src)
{
	static tregex reg(_T("^.*%[0-9a-fA-F][0-9a-fA-F].*$"));
	if (std::regex_match(src, reg) == false) {
		// エンコード表現を含まない場合は何もしない
		return src;
	}

	std::string srcA;
	UTF2UTF(src, srcA);

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
	return UTF2UTF(buf, out);
}

/**
 	コンストラクタ
 	@param[in] workbookName ワークブック名
 	@param[in] sheetName    シート名
*/
 CalcWorksheet::CalcWorksheet(
	const std::wstring& workbookName,
	const std::wstring& sheetName
) : 
	in(std::make_unique<PImpl>())
{
	in->mBookName = workbookName;

	std::wstring decoded{DecodeURI(workbookName)};
	Path localPath;
	localPath.CreateFromUrl(decoded.c_str(), 0);
	in->mBookLocalName = localPath;

	in->mSheetName = sheetName;
}

CalcWorksheet::~CalcWorksheet()
{
}



const std::wstring& CalcWorksheet::GetWorkbookName()
{
	return in->mBookLocalName;
}

const std::wstring& CalcWorksheet::GetSheetName()
{
	return in->mSheetName;
}

const String& CalcWorksheet::GetErrorMessage()
{
	return in->mErrMsg;
}


/**
 	オブジェクトに紐づけられたワークシートを有効にする
 	@return 処理の成否
*/
BOOL CalcWorksheet::Activate(bool isShowMaximize)
{
	in->mErrMsg.clear();

	std::string dst;

	json json_req;
	json_req["command"] = "activecalcsheet";
	json_req["workbook"] = UTF2UTF(in->mBookName, dst);
	json_req["worksheet"] = UTF2UTF(in->mSheetName, dst);
	json_req["maximize"] = isShowMaximize;

	auto proxy = NormalPriviledgeProcessProxy::GetInstance();

	// リクエストを送信する
	json json_res;
	if (proxy->SendRequest(json_req, json_res) == false) {
		return false;
	}
	
	bool isOK = json_res["result"];
	if (isOK == false) {
		in->mErrMsg = json_res["reason"];
	}
	return isOK;

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

} // end of namespace activate_worksheet
} // end of namespace commands
} // end of namespace launcherapp

