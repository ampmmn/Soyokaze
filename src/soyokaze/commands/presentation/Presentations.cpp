#include "pch.h"
#include "Presentations.h"
#include "commands/common/DispWrapper.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"
#include "utility/ManualEvent.h"
#include "utility/ScopeExit.h"
#include <thread>
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using json = nlohmann::json;

namespace launcherapp {
namespace commands {
namespace presentation {

using DispWrapper = launcherapp::commands::common::DispWrapper;
using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

struct Presentations::PImpl : public AppPreferenceListenerIF
{
	PImpl() : mExitEvent(false)
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	bool FetchPresentations(std::vector<SLIDE_ITEM>& items);
	bool EnumPresentationSlides(std::wstring& filePath, std::vector<std::wstring>& slideTitles);

	bool IsAvailable() {
		// PowerPointのCLSIDを得る
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(L"PowerPoint.Application", &clsid);
		return !(FAILED(hr));
	}

	CString GetSlideTitle(DispWrapper& slide);

	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Load();
	}
	void OnAppExit() override {}

	bool IsEnable() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsEnable;
	}

	void Load()
	{
		auto pref = AppPreference::Get();
		std::lock_guard<std::mutex> lock(mMutex);
		mIsEnable = pref->IsEnablePowerPointSlide();
		mPrefix = pref->GetPresentationSwitchPrefix();
	}

	bool mIsEnable{true};
	CString mPrefix;

	// 前回の取得時のタイムスタンプ
	uint64_t mLastUpdate{0};

	std::mutex mMutex;
	ManualEvent mExitEvent;
	bool mIsAvailable{false};

	std::vector<SLIDE_ITEM> mItems;
	CString mFilePath;
};

// この時間以内に再実行されたら、前回の結果を再利用する
constexpr int INTERVAL_REUSE = 5000;

bool Presentations::PImpl::FetchPresentations(std::vector<SLIDE_ITEM>& items)
{
	// 前回取得時から一定時間経過していない場合は前回の結果を再利用する
	uint64_t elapsed = GetTickCount64() - mLastUpdate;
	if (elapsed < INTERVAL_REUSE) {

		std::lock_guard<std::mutex> lock(mMutex);
		items.insert(items.end(), mItems.begin(), mItems.end());
		return true;
	}

	auto threadFunc = [&]() {

		// 処理を抜けるときにイベントを立てる
		::utility::ScopeExit guard([&]() { mExitEvent.Set(); });

		std::wstring filePath;
		std::vector<std::wstring> slideTitles;
		EnumPresentationSlides(filePath, slideTitles);

		std::vector<SLIDE_ITEM> tmpItems;
		tmpItems.reserve(slideTitles.size());
		for (int16_t i = 0; i < (int16_t)slideTitles.size(); ++i) {
			// 取得したタイトル,indexを登録
			tmpItems.emplace_back(SLIDE_ITEM{ i+1, Pattern::Mismatch, slideTitles[i].c_str() });
		}

		std::lock_guard<std::mutex> lock(mMutex);
		mItems.swap(tmpItems);
		mFilePath = filePath.c_str();
		mLastUpdate = GetTickCount64();
	};

	if (mLastUpdate == 0) {
		// 初回は同期で取得
		threadFunc();
		return true;
	}

	// 2回目以降は非同期で取得
	mExitEvent.Reset();
	std::thread th(threadFunc);
	th.detach();

	std::lock_guard<std::mutex> lock(mMutex);
	items.insert(items.end(), mItems.begin(), mItems.end());
	return true;
}

// オープンされているPowerpointスライドの一覧を取得する
bool Presentations::PImpl::EnumPresentationSlides(
		std::wstring& filePath,
	 	std::vector<std::wstring>& slideTitles
)
{
	try {
		json json_req;
		json_req["command"] = "enumpresentationslides";

		auto proxy = NormalPriviledgeProcessProxy::GetInstance();

		// リクエストを送信する
		json json_res;
		if (proxy->SendRequest(json_req, json_res) == false) {
			return false;
		}
		
		if (json_res["result"] == false) {
			return false;
		}

		UTF2UTF(json_res["file_path"].get<std::string>(), filePath);

		std::wstring tmp;

		auto items = json_res["slides"];
		for (auto& item : items) {
			slideTitles.push_back(UTF2UTF(item["title"].get<std::string>(), tmp));
		}

		return true;
	}
	catch(...) {
		spdlog::error("[EnumPresentationSlides] Unexpected exception occurred.");
		return false;
	}
}

CString Presentations::PImpl::GetSlideTitle(DispWrapper& slide)
{
	// Shapes
	DispWrapper shapes;
	slide.GetPropertyObject(L"Shapes", shapes);

	// Shapes.Count
	int shapeCount = shapes.GetPropertyInt(L"Count");

	// foreach(shapes)
	for (int16_t j = 1; j <= (int16_t)shapeCount; ++j) {

		DispWrapper shape;
		shapes.CallObjectMethod(L"Item", j, shape);

		// Type
		int type = shape.GetPropertyInt(L"Type");
		if (type != 14 && type != 17) {  // 14:PlaceHolder 17:TextBox
			continue;
		}

		// Shape.Name
		CString shapeName = shape.GetPropertyString(L"Name");

		// shape名がTitleでなければスキップ
		static tregex pat(_T("^ *Title.*$"));      // 英語版向け
		static tregex pat2(_T("^ *タイトル.*$"));  // 日本語版向け
		if (std::regex_match(tstring(shapeName), pat) == false && 
				std::regex_match(tstring(shapeName), pat2) == false) {
			continue;
		}

		// スライドタイトルを取得(shape.TextFrame2.TextRange.Item(1))
		DispWrapper txtFrame;
		shape.GetPropertyObject(L"TextFrame2", txtFrame);
		DispWrapper txtRange;
		txtFrame.GetPropertyObject(L"TextRange", txtRange);

		return txtRange.GetPropertyString(L"Text");
	}
	return _T("");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


Presentations::Presentations() : in(new PImpl)
{
	in->mIsAvailable = in->IsAvailable();
}

Presentations::~Presentations()
{
	Abort();
}

void Presentations::Abort()
{
	// 更新スレッドの終了を待つ(最大3秒)
	in->mExitEvent.WaitFor(3000);
}

void Presentations::Load()
{
	in->Load();
}

void Presentations::Query(Pattern* pattern, std::vector<SLIDE_ITEM>& items, int limit)
{
	// 機能を利用しない場合は抜ける
	if (in->mIsAvailable == false || in->IsEnable() == false) {
		return;
	}
	// プレフィックスが一致しない場合は抜ける
	const auto& prefix = in->mPrefix;
	if (prefix.IsEmpty() == FALSE && prefix.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return;
	}

	bool hasPrefix =  prefix.IsEmpty() == FALSE;
	int offset = hasPrefix ? 1 : 0;

	// キーワードが数値として解釈できる場合、ページ番号でのマッチングを行うため、数値として拾っておく
	int pageNo = -1;
	ParseTextAsPageNo(pattern->GetWholeString(), pageNo);

	std::vector<SLIDE_ITEM> allItems;
	in->FetchPresentations(allItems);

	if (allItems.empty()) {
		return;
	}

	auto fileName = PathFindFileName(in->mFilePath);

	for (const auto& item : allItems) {

		int level = Pattern::Mismatch;

		bool isMatchPageNo = pageNo != -1 && pageNo == item.mPage;
		if (isMatchPageNo) {
			// 入力キーワードが数値で、スライド番号として一致する場合は全体一致とみなす
			level = Pattern::WholeMatch;
		}
		else {
			auto str = item.mTitle + _T(" ") + fileName;
			// スライド番号として一致しなかった場合、キーワードでのマッチングを行う
			int level_t = pattern->Match(item.mTitle, offset);
			// ファイル名でもマッチングする
			int level_w = pattern->Match(str, offset);

			if (level_t == Pattern::Mismatch && level_w == Pattern::Mismatch) {
				continue;
			}

			level = (std::max)(level_t, level_w);
		}

		// プレフィックスがある場合は最低でも前方一致とする
		if (hasPrefix && level == Pattern::PartialMatch) {
			level = Pattern::FrontMatch;
		}

		auto itemCopy(item);
		itemCopy.mMatchLevel = level;
		items.push_back(itemCopy);

		if ((int)items.size() >= limit) {
			break;
		}
	}
}

CString Presentations::GetFilePath()
{
	return in->mFilePath;
}

bool Presentations::ParseTextAsPageNo(LPCTSTR text, int& pageNo)
{
	static tregex pat(_T("^ *[0-9]+ *$"));
	return std::regex_match(text, pat) == false || _stscanf_s(text, _T("%d"), &pageNo) == -1;
}


}
}
}
