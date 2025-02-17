#include "pch.h"
#include "DefaultComponentPlacer.h"
#include "mainwindow/layout/MainWindowPlacement.h"
#include "utility/HWNDRect.h"



namespace launcherapp {
namespace mainwindow {
namespace layout {

struct DefaultComponentPlacer::PImpl
{
	std::unique_ptr<MainWindowPlacement> mPlacement;
};

DefaultComponentPlacer::DefaultComponentPlacer(MainWindowPlacement* placement) : in(new PImpl)
{
	in->mPlacement.reset(placement);
}

DefaultComponentPlacer::~DefaultComponentPlacer()
{
}


// アイコン欄のサイズ計算と配置
bool DefaultComponentPlacer::PlaceIcon(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement->GetMarginLeft();
	int MARGIN_Y = in->mPlacement->GetMarginTop();

	// フォントサイズから入力欄(とガイド欄)の高さを求める
	int fontH = in->mPlacement->GetFontPixelSize();
	int editH = fontH + 4;

	// 説明欄の高さ + 余白 + ガイド欄の高さ
	int components_h = editH + 2 + editH;

	CSize sizeIconArea(32, 32);
	if (fontH > 32) {
		sizeIconArea = CSize(fontH, fontH);
	}

	int offset = (components_h - sizeIconArea.cy) / 2;

	ASSERT(elemHwnd);
	SetWindowPos(elemHwnd, nullptr, MARGIN_X, MARGIN_Y + offset, sizeIconArea.cx, sizeIconArea.cy, SWP_NOZORDER);

	return true;
}

// 説明欄のサイズ計算と配置
bool DefaultComponentPlacer::PlaceDescription(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement->GetMarginLeft();
	int MARGIN_Y = in->mPlacement->GetMarginTop();
	int margin = 2;

	// アイコン欄の右側に説明欄を配置する
	int x = MARGIN_X + in->mPlacement->GetIconWindowWidth() + margin;
	int y =	MARGIN_Y;

	// 親ウインドウの幅と、説明欄の配置した位置をもとに幅を決定
	int cx = in->mPlacement->GetMainWindowWidth() - x - MARGIN_X;

	// フォントサイズに応じて高さを決定する
	int fontH = in->mPlacement->GetFontPixelSize();
	int cy = fontH + 4;

	// 説明欄の位置・サイズを移動
	spdlog::debug("desc xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}

// ガイド欄のサイズ計算と配置
bool DefaultComponentPlacer::PlaceGuide(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement->GetMarginLeft();
	int MARGIN_Y = in->mPlacement->GetMarginTop();
	int margin = 2;

	// アイコン欄の右側にガイド欄を配置する
	int x = MARGIN_X + in->mPlacement->GetIconWindowWidth() + margin;
	// 説明欄の下
	int y =	MARGIN_Y + in->mPlacement->GetDescriptionWindowHeight() + margin;

	// 親ウインドウの幅と、ガイド欄を配置した位置をもとに幅を決定
	int cx = in->mPlacement->GetMainWindowWidth() - x - 1;

	// フォントサイズに応じて高さを決定する
	int fontH = in->mPlacement->GetFontPixelSize();
	int cy = fontH + 4;

	// 説明欄の位置・サイズを移動
	spdlog::debug("comment xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}

// 入力欄のサイズ計算と配置
bool DefaultComponentPlacer::PlaceEdit(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement->GetMarginLeft();
	int MARGIN_Y = in->mPlacement->GetMarginTop();
	int margin = 2;

	int x = MARGIN_X;

	// アイコン欄とガイド欄のうち、大きい(→下に来る)方の少し下に配置する
	int iconBottom = MARGIN_Y + in->mPlacement->GetIconWindowHeight();
	int guideBottom =	MARGIN_Y + in->mPlacement->GetDescriptionWindowHeight() + margin +
		                in->mPlacement->GetGuideWindowHeight();

	int y =	(std::max)(iconBottom, guideBottom) + margin;

	int cx = in->mPlacement->GetMainWindowWidth() - MARGIN_X * 2;

	// フォントサイズに応じて高さを決定する
	int fontH = in->mPlacement->GetFontPixelSize();
	int cy = fontH + 4;

	// 説明欄の位置・サイズを移動
	spdlog::debug("edit xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}

// 候補欄のサイズ計算と配置
bool DefaultComponentPlacer::PlaceCandidateList(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement->GetMarginLeft();
	int MARGIN_Y = in->mPlacement->GetMarginTop();
	int margin = in->mPlacement->GetMarginEditToList();

	int x = MARGIN_X;

	// 入力欄の下に配置する
	HWNDRect rcEdit(in->mPlacement->GetEdit()->GetSafeHwnd());
	rcEdit.MapToParent();

	int y =	rcEdit->bottom + margin;

	// 親ウインドウの幅と、ガイド欄を配置した位置をもとに幅を決定
	int cx = in->mPlacement->GetMainWindowWidth() - MARGIN_X * 2;

	int cy = in->mPlacement->GetMainWindowHeight() - rcEdit->bottom - margin - MARGIN_Y;

	// 説明欄の位置・サイズを移動
	spdlog::debug("candidate xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}


// 適用
void DefaultComponentPlacer::Apply(HWND hwnd)
{
	UNREFERENCED_PARAMETER(hwnd);

	in->mPlacement->GetIconLabel()->ShowWindow(SW_SHOW);

	auto descLabel = in->mPlacement->GetDescriptionLabel();
	descLabel->InvalidateRect(nullptr);
	descLabel->UpdateWindow();

	auto guideLabel = in->mPlacement->GetGuideLabel();
	guideLabel->ShowWindow(SW_SHOW);
	guideLabel->InvalidateRect(nullptr);
	guideLabel->UpdateWindow();

	auto edit = in->mPlacement->GetEdit();
	edit->InvalidateRect(nullptr);
	edit->UpdateWindow();

	auto list = in->mPlacement->GetCandidateList();
	list->InvalidateRect(nullptr);
	list->UpdateWindow();
}

// それより以下のリサイズを許容しない最小限の高さを取得する
int DefaultComponentPlacer::GetMinimumHeight()
{
	int margin = 2;
	auto& p = in->mPlacement;
	return p->GetMarginTop() + 
	       p->GetDescriptionWindowHeight() +
	       margin + 
	       p->GetGuideWindowHeight() +
	       margin + 
	       p->GetEditWindowHeight() +
	       margin + 
	       p->GetMarginTop() + 1;
}

// 最低限の候補欄の高さを取得する
int DefaultComponentPlacer::GetMinimumCandidateHeight()
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


