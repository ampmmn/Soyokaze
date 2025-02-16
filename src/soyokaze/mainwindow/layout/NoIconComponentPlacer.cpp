#include "pch.h"
#include "NoIconComponentPlacer.h"
#include "mainwindow/layout/MainWindowPlacement.h"
#include "utility/HWNDRect.h"


namespace launcherapp {
namespace mainwindow {
namespace layout {

struct NoIconComponentPlacer::PImpl
{
	std::unique_ptr<MainWindowPlacement> mPlacement;
};

NoIconComponentPlacer::NoIconComponentPlacer(MainWindowPlacement* placement) : in(new PImpl)
{
	in->mPlacement.reset(placement);
}

NoIconComponentPlacer::~NoIconComponentPlacer()
{
}


// アイコン欄のサイズ計算と配置
bool NoIconComponentPlacer::PlaceIcon(HWND elemHwnd)
{
	// アイコンは表示しない
	UNREFERENCED_PARAMETER(elemHwnd);
	return true;
}

// 説明欄のサイズ計算と配置
bool NoIconComponentPlacer::PlaceDescription(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement->GetMarginLeft();
	int MARGIN_Y = in->mPlacement->GetMarginTop();

	// アイコンを表示しないので左端に表示
	int x = MARGIN_X;
	int y =	MARGIN_Y;

	// 親ウインドウの幅と、説明欄の配置した位置をもとに幅を決定
	int cx = in->mPlacement->GetMainWindowWidth() - x - MARGIN_X;

	// フォントサイズから説明欄(と入力欄)の高さを求める
	int fontH = in->mPlacement->GetFontPixelSize();
	int cy = fontH + 4;

	// 説明欄の位置・サイズを移動
	spdlog::debug("desc xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}

// ガイド欄のサイズ計算と配置
bool NoIconComponentPlacer::PlaceGuide(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement->GetMarginLeft();
	int MARGIN_Y = in->mPlacement->GetMarginTop();

	int margin = 2;

	// アイコンを表示しないので左端に表示
	int x = MARGIN_X;
	// 説明欄の下
	int y =	MARGIN_Y + in->mPlacement->GetDescriptionWindowHeight() + margin;

	// 親ウインドウの幅と、ガイド欄を配置した位置をもとに幅を決定
	int cx = in->mPlacement->GetMainWindowWidth() - x - MARGIN_X;

	// フォントサイズから説明欄(と入力欄)の高さを求める
	int fontH = in->mPlacement->GetFontPixelSize();
	int cy = fontH + 4;

	// 説明欄の位置・サイズを移動
	spdlog::debug("comment xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
	// ガイド欄は表示しない
	ShowWindow(elemHwnd, SW_HIDE);
	return true;
}

// 入力欄のサイズ計算と配置
bool NoIconComponentPlacer::PlaceEdit(HWND elemHwnd)
{
	int MARGIN_X = in->mPlacement->GetMarginLeft();
	
	int margin = 2;

	// アイコンを表示しないので左端に表示
	int x = MARGIN_X;

	// ガイド欄の下に配置
	HWNDRect rcGuide(in->mPlacement->GetGuideLabel()->GetSafeHwnd());
	rcGuide.MapToParent();
	int y =	rcGuide->bottom + margin;

	// 親ウインドウの幅と、説明欄の配置した位置をもとに幅を決定
	int cx = in->mPlacement->GetMainWindowWidth() - MARGIN_X * 2;

	// フォントサイズから説明欄(と入力欄)の高さを求める
	int fontH = in->mPlacement->GetFontPixelSize();
	int cy = fontH + 4;

	// 説明欄の位置・サイズを移動
	spdlog::debug("edit xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}

// 候補欄のサイズ計算と配置
bool NoIconComponentPlacer::PlaceCandidateList(HWND elemHwnd)
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
void NoIconComponentPlacer::Apply(HWND hwnd)
{
	UNREFERENCED_PARAMETER(hwnd);

	in->mPlacement->GetIconLabel()->ShowWindow(SW_HIDE);

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
int NoIconComponentPlacer::GetMinimumHeight()
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
	       p->GetMarginTop();
}

// 最低限の候補欄の高さを取得する
int NoIconComponentPlacer::GetMinimumCandidateHeight()
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


