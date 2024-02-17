#include "pch.h"
#include "Presentations.h"
#include "commands/activate_window/AutoWrap.h"
#include "AppPreferenceListenerIF.h"
#include "AppPreference.h"
#include <thread>
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace presentation {

using DispWrapper = soyokaze::commands::activate_window::DispWrapper;

struct Presentations::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);

		OnAppPreferenceUpdated();
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void WatchPresentations();

	bool IsAvailable() {
		// PowerPointのCLSIDを得る
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(L"PowerPoint.Application", &clsid);
		return !(FAILED(hr));
	}

	bool IsAbort() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsAbort;
	}
	void Abort() {
		std::lock_guard<std::mutex> lock(mMutex);
		mIsAbort = true;
	}


	void UpdateCurrentSlide(DispWrapper& activeWindow, DispWrapper& activePresentation);
	void UpdateAllSlides(DispWrapper& activePresentation);
	
	CString GetSlideTitle(DispWrapper& slide);


	void ClearSlides()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mItems.clear();
	}

	void SetSlideCount(int count)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mItems.resize(count);
	}

	void SetSlide(int page, const CString& title)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		int index = page-1;
		if (0 <= index && index < (int)mItems.size()) {
			mItems[index].mPage = page;
			mItems[index].mTitle = title;
		}
	}

	bool IsPathChanged(const CString& path, const CString& name)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return path != mFilePath || name != mFileName;
	}

	void OnAppFirstBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();

		std::lock_guard<std::mutex> lock(mMutex);
		mIsEnable = pref->IsEnablePowerPointSlide();

	}
	void OnAppExit() override {}

	bool IsEnable() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsEnable;
	}

	bool mIsEnable;

	std::mutex mMutex;
	bool mIsAbort;
	bool mIsAvailable;

	// pptxファイルのパス(ディレクトリまで)
	CString mFilePath;
	// pptxファイルのファイル名
	CString mFileName;

	std::vector<SLIDE_ITEM> mItems;
};

void Presentations::PImpl::WatchPresentations()
{
	if (IsEnable() == false) {
		// 機能は無効
		return;
	}

	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"PowerPoint.Application", &clsid);
	ASSERT(hr == S_OK);

	// アプリケーションを取得
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if(FAILED(hr)) {
		// 起動してない
		ClearSlides();
		return;
	}

	DispWrapper pptApp;
	unkPtr->QueryInterface(&pptApp);

	// アクティブなPresentationを取得
	DispWrapper activePresentation;
	if (pptApp.GetPropertyObject(L"ActivePresentation", activePresentation) == false) {
		// アプリは起動してるけど何も表示してない(初期画面とか)
		return;
	}

	DispWrapper activeWindow;
	if (pptApp.GetPropertyObject(L"ActiveWindow", activeWindow) == false) {
		// アプリは起動してるけど何も表示してない(初期画面とか)
		return;
	}

	// アクティブなPresentationのパスとファイル名を取得
	CString fileDir = activePresentation.GetPropertyString(L"Path");
	CString fileName = activePresentation.GetPropertyString(L"Name");

	if (IsPathChanged(fileDir, fileName)) {
		// 別のスライドに代わっていたら前回の内容を破棄する
		UpdateAllSlides(activePresentation);
	}
	else {
		// 同じだったら現在のスライドだけ更新する
		UpdateCurrentSlide(activeWindow, activePresentation);
	}
}

void Presentations::PImpl::UpdateCurrentSlide(DispWrapper& activeWindow, DispWrapper& activePresentation)
{
	// ActiveWindow.Selection.SlideRange.SlideIndex

	DispWrapper selection;
	activeWindow.GetPropertyObject(L"Selection", selection);

	DispWrapper slideRange;
	selection.GetPropertyObject(L"SlideRange", slideRange);

	int16_t slideIndex = (int16_t)slideRange.GetPropertyInt(L"SlideIndex");
	
	// Slidesを取得
	DispWrapper slides;
	activePresentation.GetPropertyObject(L"Slides", slides);

	// スライドを取得
	DispWrapper slide;
	slides.CallObjectMethod(L"Item", slideIndex, slide);

	CString titleString = GetSlideTitle(slide);
	// 取得したタイトル,indexを登録
	SetSlide(slideIndex, titleString);
}

// 全てのスライドの情報を更新
void Presentations::PImpl::UpdateAllSlides(DispWrapper& activePresentation)
{
	ClearSlides();

	CString fileDir = activePresentation.GetPropertyString(L"Path");
	CString fileName = activePresentation.GetPropertyString(L"Name");

	{
		std::lock_guard<std::mutex> lock(mMutex);
		mFilePath = fileDir;
		mFileName = fileName;
	}

	// Slidesを取得
	DispWrapper slides;
	activePresentation.GetPropertyObject(L"Slides", slides);

	// Count
	int slideCount = slides.GetPropertyInt(L"Count");
	SetSlideCount(slideCount);

	for (int16_t i = 1; i <= (int16_t)slideCount; ++i) {

		if (IsAbort()) {
			return;
		}

		// スライドを取得
		DispWrapper slide;
		slides.CallObjectMethod(L"Item", i, slide);

		CString titleString = GetSlideTitle(slide);
		// 取得したタイトル,indexを登録
		SetSlide(i, titleString);
		Sleep(0);
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
		if (type != 14) {
			continue;
		}

		// Shape.Name
		CString shapeName = shape.GetPropertyString(L"Name");

		// shape名がTitleでなければスキップ
		static tregex pat(_T("^ *Title.*$"));
		if (std::regex_match(tstring(shapeName), pat) == false) {
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
	in->mIsAbort = false;
	in->mIsAvailable = in->IsAvailable();
	if (in->mIsAvailable == false) {
		// PowerPointが利用できない
		return ;
	}

	std::thread th([&]() {
		while(in->IsAbort() == false) {
			try {
				in->WatchPresentations();
				Sleep(1000);
			}
			catch(...) {
			}
		}
	});
	th.detach();
}

Presentations::~Presentations()
{
	Abort();
}

void Presentations::Abort()
{
	in->Abort();
}

void Presentations::Query(Pattern* pattern, std::vector<SLIDE_ITEM>& items, int limit)
{
	if (in->mIsAvailable == false || in->IsEnable() == false) {
		return;
	}

	std::lock_guard<std::mutex> lock(in->mMutex);

	for (auto& item : in->mItems) {
		int level = pattern->Match(item.mTitle);
		if (level == Pattern::Mismatch) {
			continue;
		}

		auto itemCopy(item);
		itemCopy.mMatchLevel = level;
		items.push_back(itemCopy);

		if ((int)items.size() >= limit) {
			break;
		}
	}
}



}
}
}
