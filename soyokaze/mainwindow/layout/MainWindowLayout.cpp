#include "pch.h"
#include "MainWindowLayout.h"
#include "resource.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "mainwindow/layout/DefaultComponentPlacer.h"
#include "mainwindow/layout/NoGuideComponentPlacer.h"
#include "mainwindow/layout/NoIconComponentPlacer.h"
#include "mainwindow/layout/NoGuideNoIconComponentPlacer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ComponentPlacer = launcherapp::mainwindow::layout::ComponentPlacer;

struct MainWindowLayout::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override
	{
	}
	void OnAppNormalBoot() override {}

	void OnAppPreferenceUpdated() override
	{
		LoadSettings();

		if (mParentWnd) {
			mThisPtr->RecalcControls(mParentWnd);
		}
	}

	void OnAppExit() override
	{
	}

	bool IsShowGuide()
	{
		return mIsShowGuide;
	}
	bool IsDrawIcon()
	{
		return mIsDrawIcon;
	}

	void LoadSettings()
	{
			AppPreference* pref = AppPreference::Get();
			mIsShowGuide = pref->IsShowGuide();
			mIsDrawIcon = pref->IsDrawIcon();
	}

	ComponentPlacer* CreateComponentPlacer(HWND hwnd)
	{
		if (mIsFirstCall) {
			LoadSettings();
			mIsFirstCall = false;
		}

		if (mIsShowGuide == false && mIsDrawIcon) {
			// ガイドなし、アイコンあり
			return new launcherapp::mainwindow::layout::NoGuideComponentPlacer(hwnd);
		}
		if (mIsShowGuide && mIsDrawIcon == false) {
			// ガイドあり、アイコンなし
			return new launcherapp::mainwindow::layout::NoIconComponentPlacer(hwnd);
		}
		if (mIsShowGuide == false && mIsDrawIcon == false) {
			// ガイドなし、アイコンなし
			return new launcherapp::mainwindow::layout::NoGuideNoIconComponentPlacer(hwnd);
		}
		else {
			return new launcherapp::mainwindow::layout::DefaultComponentPlacer(hwnd);
		}
	}

	HWND mParentWnd = nullptr;
	MainWindowLayout* mThisPtr = nullptr;
	bool mIsFirstCall = true;
	bool mIsShowGuide = false;
	bool mIsDrawIcon = true;

	std::unique_ptr<ComponentPlacer> mPlacer;

};

MainWindowLayout::MainWindowLayout() : in(new PImpl)
{
	in->mThisPtr = this;
}

MainWindowLayout::~MainWindowLayout()
{
}

void MainWindowLayout::RecalcWindowSize(HWND hwnd, UINT side, LPRECT rect)
{
	// 直近のウインドウを親ウインドウとして覚えておく(基本的に変化しないけど)
	in->mParentWnd = hwnd;

	UNREFERENCED_PARAMETER(hwnd);
	UNREFERENCED_PARAMETER(side);
	UNREFERENCED_PARAMETER(rect);
	// ToDo: 必要に応じて実装
}

void MainWindowLayout::RecalcControls(HWND hwnd)
{
	// 直近のウインドウを親ウインドウとして覚えておく(基本的に変化しないけど)
	in->mParentWnd = hwnd;

	CWnd* parent = CWnd::FromHandle(hwnd);

	auto iconLabel = parent->GetDlgItem(IDC_STATIC_ICON);
	if (iconLabel == nullptr) {
		// ウインドウ初期化が終わっていない場合はコントロールを取得できないのでここで抜ける
		return;
	}

	std::unique_ptr<ComponentPlacer> placer(in->CreateComponentPlacer(hwnd));

	// アイコン欄
	placer->PlaceIcon(iconLabel->GetSafeHwnd());
	// 説明欄
	auto comment = parent->GetDlgItem(IDC_STATIC_DESCRIPTION);
	placer->PlaceDescription(comment->GetSafeHwnd());
	// ガイド欄
	auto guide = parent->GetDlgItem(IDC_STATIC_GUIDE);
	placer->PlaceGuide(guide->GetSafeHwnd());
	// 入力欄
	auto edit = parent->GetDlgItem(IDC_EDIT_COMMAND);
	placer->PlaceEdit(edit->GetSafeHwnd());
	// 候補欄
	auto listCtrl = parent->GetDlgItem(IDC_LIST_CANDIDATE);
	placer->PlaceCandidateList(listCtrl->GetSafeHwnd());

	placer->Apply(hwnd);
}


