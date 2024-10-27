#include "pch.h"
#include "NoGuideComponentPlacer.h"
#include "mainwindow/layout/MainWindowPlacement.h"
#include "utility/HWNDRect.h"


constexpr int MARGIN_X = 2;
constexpr int MARGIN_Y = 2;


namespace launcherapp {
namespace mainwindow {
namespace layout {

struct NoGuideComponentPlacer::PImpl
{
	MainWindowPlacement mPlacement;
};

NoGuideComponentPlacer::NoGuideComponentPlacer(HWND hwnd) : in(new PImpl)
{
	in->mPlacement.SetMainWindowHwnd(hwnd);
}

NoGuideComponentPlacer::~NoGuideComponentPlacer()
{
}


// アイコン欄のサイズ計算と配置
bool NoGuideComponentPlacer::PlaceIcon(HWND elemHwnd)
{
	ASSERT(elemHwnd);
	SetWindowPos(elemHwnd, nullptr, MARGIN_X, MARGIN_Y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	return true;
}

// 説明欄のサイズ計算と配置
bool NoGuideComponentPlacer::PlaceDescription(HWND elemHwnd)
{
	int margin = 2;

	// アイコン欄の右側に説明欄を配置する
	int x = MARGIN_X + in->mPlacement.GetIconWindowWidth() + margin;
	int y =	MARGIN_Y;

	// 親ウインドウの幅と、説明欄の配置した位置をもとに幅を決定
	int cx = in->mPlacement.GetMainWindowWidth() - x - 1;
	// 高さは変えない
	int cy = in->mPlacement.GetDescriptionWindowHeight();

	// 説明欄の位置・サイズを移動
	spdlog::debug("desc xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}

// ガイド欄のサイズ計算と配置
bool NoGuideComponentPlacer::PlaceGuide(HWND elemHwnd)
{
	// ガイド欄は表示しない
	return true;
}

// 入力欄のサイズ計算と配置
bool NoGuideComponentPlacer::PlaceEdit(HWND elemHwnd)
{
	int margin = 2;

	// アイコン欄の右側に説明欄を配置する
	int x = MARGIN_X + in->mPlacement.GetIconWindowWidth() + margin;

	// 説明欄の下に配置
	int y =	MARGIN_Y + in->mPlacement.GetDescriptionWindowHeight() + margin;

	// 親ウインドウの幅と、説明欄の配置した位置をもとに幅を決定
	int cx = in->mPlacement.GetMainWindowWidth() - x - MARGIN_X;
	// 高さは変えない
	int cy = in->mPlacement.GetEditWindowHeight();

	// 説明欄の位置・サイズを移動
	spdlog::debug("edit xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}

// 候補欄のサイズ計算と配置
bool NoGuideComponentPlacer::PlaceCandidateList(HWND elemHwnd)
{
	int margin = 1;

	int x = MARGIN_X;

	// 入力欄の下に配置する
	HWNDRect rcEdit(in->mPlacement.GetEdit()->GetSafeHwnd());
	rcEdit.MapToParent();

	int y =	rcEdit->bottom + margin;

	// 親ウインドウの幅をもとに幅を決定
	int cx = in->mPlacement.GetMainWindowWidth() - MARGIN_X * 2;

	int cy = in->mPlacement.GetMainWindowHeight() - rcEdit->bottom - margin - MARGIN_Y;

	// 説明欄の位置・サイズを移動
	spdlog::debug("candidate xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}


// 適用
void NoGuideComponentPlacer::Apply(HWND hwnd)
{
	UNREFERENCED_PARAMETER(hwnd);

	in->mPlacement.GetIconLabel()->ShowWindow(SW_SHOW);

	auto descLabel = in->mPlacement.GetDescriptionLabel();
	descLabel->InvalidateRect(nullptr);
	descLabel->UpdateWindow();

	auto guideLabel = in->mPlacement.GetGuideLabel();
	guideLabel->ShowWindow(SW_HIDE);

	auto edit = in->mPlacement.GetEdit();
	edit->InvalidateRect(nullptr);
	edit->UpdateWindow();

	auto list = in->mPlacement.GetCandidateList();
	list->InvalidateRect(nullptr);
	list->UpdateWindow();
}

}
}
}


