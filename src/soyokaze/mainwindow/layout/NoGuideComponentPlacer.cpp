#include "pch.h"
#include "NoGuideComponentPlacer.h"
#include "mainwindow/layout/MainWindowPlacement.h"
#include "utility/HWNDRect.h"


namespace launcherapp {
namespace mainwindow {
namespace layout {

struct NoGuideComponentPlacer::PImpl
{
	std::unique_ptr<MainWindowPlacement> mPlacement;
};

NoGuideComponentPlacer::NoGuideComponentPlacer(MainWindowPlacement* placement) : in(new PImpl)
{
	in->mPlacement.reset(placement);
}

NoGuideComponentPlacer::~NoGuideComponentPlacer()
{
}


// アイコン欄のサイズ計算と配置
bool NoGuideComponentPlacer::PlaceIcon(HWND elemHwnd)
{
	ASSERT(elemHwnd);
	int MARGIN_X = in->mPlacement->GetMarginLeft();
	int MARGIN_Y = in->mPlacement->GetMarginTop();

	// フォントサイズから説明欄(と入力欄)の高さを求める
	int fontH = in->mPlacement->GetFontPixelSize();
	int cy = fontH + 4;

	int h = in->mPlacement->GetIconWindowHeight();

	// 説明欄の高さ + 余白 + 入力欄の高さ
	int components_h = cy + 2 + cy;

	int offset = (components_h - h) / 2;

	SetWindowPos(elemHwnd, nullptr, MARGIN_X, MARGIN_Y + offset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	return true;
}

// 説明欄のサイズ計算と配置
bool NoGuideComponentPlacer::PlaceDescription(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement->GetMarginLeft();
	int MARGIN_Y = in->mPlacement->GetMarginTop();
	int margin = 2;

	// アイコン欄の右側に説明欄を配置する
	int x = MARGIN_X + in->mPlacement->GetIconWindowWidth() + margin;
	int y =	MARGIN_Y;

	// 親ウインドウの幅と、説明欄の配置した位置をもとに幅を決定
	int cx = in->mPlacement->GetMainWindowWidth() - x - 1;

	// フォントサイズに応じて高さを決定する
	int fontH = in->mPlacement->GetFontPixelSize();
	int cy = fontH + 4;

	// 説明欄の位置・サイズを移動
	spdlog::debug("desc xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}

// ガイド欄のサイズ計算と配置
bool NoGuideComponentPlacer::PlaceGuide(HWND elemHwnd)
{
	UNREFERENCED_PARAMETER(elemHwnd);
	// ガイド欄は表示しない
	return true;
}

// 入力欄のサイズ計算と配置
bool NoGuideComponentPlacer::PlaceEdit(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement->GetMarginLeft();
	int MARGIN_Y = in->mPlacement->GetMarginTop();
	int margin = 2;

	// アイコン欄の右側に説明欄を配置する
	int x = MARGIN_X + in->mPlacement->GetIconWindowWidth() + margin;

	// 説明欄の下に配置
	int y =	MARGIN_Y + in->mPlacement->GetDescriptionWindowHeight() + margin;

	// 親ウインドウの幅と、説明欄の配置した位置をもとに幅を決定
	int cx = in->mPlacement->GetMainWindowWidth() - x - MARGIN_X;

	// フォントサイズに応じて高さを決定する
	int fontH = in->mPlacement->GetFontPixelSize();
	int cy = fontH + 4;

	// 説明欄の位置・サイズを移動
	spdlog::debug("edit xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}

// 候補欄のサイズ計算と配置
bool NoGuideComponentPlacer::PlaceCandidateList(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement->GetMarginLeft();
	int MARGIN_Y = in->mPlacement->GetMarginTop();
	int margin = in->mPlacement->GetMarginEditToList();

	int x = MARGIN_X;

	// 入力欄の下に配置する
	HWNDRect rcEdit(in->mPlacement->GetEdit()->GetSafeHwnd());
	rcEdit.MapToParent();

	int y =	rcEdit->bottom + margin;

	// 親ウインドウの幅をもとに幅を決定
	int cx = in->mPlacement->GetMainWindowWidth() - MARGIN_X * 2;

	int cy = in->mPlacement->GetMainWindowHeight() - rcEdit->bottom - margin - MARGIN_Y;

	// 説明欄の位置・サイズを移動
	spdlog::debug("candidate xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}


// 適用
void NoGuideComponentPlacer::Apply(HWND hwnd)
{
	UNREFERENCED_PARAMETER(hwnd);

	in->mPlacement->GetIconLabel()->ShowWindow(SW_SHOW);

	auto descLabel = in->mPlacement->GetDescriptionLabel();
	descLabel->InvalidateRect(nullptr);
	descLabel->UpdateWindow();

	auto guideLabel = in->mPlacement->GetGuideLabel();
	guideLabel->ShowWindow(SW_HIDE);

	auto edit = in->mPlacement->GetEdit();
	edit->InvalidateRect(nullptr);
	edit->UpdateWindow();

	auto list = in->mPlacement->GetCandidateList();
	list->InvalidateRect(nullptr);
	list->UpdateWindow();
}

// それより以下のリサイズを許容しない最小限の高さを取得する
int NoGuideComponentPlacer::GetMinimumHeight()
{
	int margin = 2;
	auto& p = in->mPlacement;
	return p->GetMarginTop() + 
	       p->GetDescriptionWindowHeight() +
	       margin + 
	       p->GetEditWindowHeight() +
	       margin + 
	       p->GetMarginTop();
}

// 最低限の候補欄の高さを取得する
int NoGuideComponentPlacer::GetMinimumCandidateHeight()
{
	CRect rcItem;
	auto list = (CListCtrl*)in->mPlacement->GetCandidateList();
	if (list->GetItemRect(0, &rcItem, LVIR_BOUNDS) == FALSE) {
		return 0;
	}
	return rcItem.Height();
}


}
}
}


