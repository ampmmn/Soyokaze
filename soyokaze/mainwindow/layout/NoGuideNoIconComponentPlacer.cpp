#include "pch.h"
#include "NoGuideNoIconComponentPlacer.h"
#include "mainwindow/layout/MainWindowPlacement.h"
#include "utility/HWNDRect.h"


namespace launcherapp {
namespace mainwindow {
namespace layout {

struct NoGuideNoIconComponentPlacer::PImpl
{
	MainWindowPlacement mPlacement;
};

NoGuideNoIconComponentPlacer::NoGuideNoIconComponentPlacer(HWND hwnd) : in(new PImpl)
{
	in->mPlacement.SetMainWindowHwnd(hwnd);
}

NoGuideNoIconComponentPlacer::~NoGuideNoIconComponentPlacer()
{
}


// アイコン欄のサイズ計算と配置
bool NoGuideNoIconComponentPlacer::PlaceIcon(HWND elemHwnd)
{
	// アイコンは表示しない
	UNREFERENCED_PARAMETER(elemHwnd);
	return true;
}

// 説明欄のサイズ計算と配置
bool NoGuideNoIconComponentPlacer::PlaceDescription(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement.GetMarginLeft();
	int MARGIN_Y = in->mPlacement.GetMarginTop();

	// アイコンを表示しないので左端に表示
	int x = MARGIN_X;
	int y =	MARGIN_Y;

	// 親ウインドウの幅と、説明欄の配置した位置をもとに幅を決定
	int cx = in->mPlacement.GetMainWindowWidth() - x - MARGIN_X;
	// 高さは変えない
	int cy = in->mPlacement.GetDescriptionWindowHeight();

	// 説明欄の位置・サイズを移動
	spdlog::debug("desc xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}

// ガイド欄のサイズ計算と配置
bool NoGuideNoIconComponentPlacer::PlaceGuide(HWND elemHwnd)
{
	// 表示しない
	UNREFERENCED_PARAMETER(elemHwnd);
	return true;
}

// 入力欄のサイズ計算と配置
bool NoGuideNoIconComponentPlacer::PlaceEdit(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement.GetMarginLeft();

	int margin = 2;

	// アイコンを表示しないので左端に表示
	int x = MARGIN_X;

	// 説明欄の下に配置
	HWNDRect rcDesc(in->mPlacement.GetDescriptionLabel()->GetSafeHwnd());
	rcDesc.MapToParent();
	int y =	rcDesc->bottom + margin;

	// 親ウインドウの幅と、説明欄の配置した位置をもとに幅を決定
	int cx = in->mPlacement.GetMainWindowWidth() - MARGIN_X * 2;

	// 高さは変えない
	int cy = in->mPlacement.GetEditWindowHeight();

	// 説明欄の位置・サイズを移動
	spdlog::debug("edit xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}

// 候補欄のサイズ計算と配置
bool NoGuideNoIconComponentPlacer::PlaceCandidateList(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement.GetMarginLeft();
	int MARGIN_Y = in->mPlacement.GetMarginTop();
	int margin = in->mPlacement.GetMarginEditToList();

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
void NoGuideNoIconComponentPlacer::Apply(HWND hwnd)
{
	UNREFERENCED_PARAMETER(hwnd);

	in->mPlacement.GetIconLabel()->ShowWindow(SW_HIDE);

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


