#include "pch.h"
#include "DefaultComponentPlacer.h"
#include "mainwindow/layout/MainWindowPlacement.h"
#include "setting/AppPreference.h"
#include "utility/HWNDRect.h"



namespace launcherapp {
namespace mainwindow {
namespace layout {

struct DefaultComponentPlacer::PImpl
{
	std::unique_ptr<MainWindowPlacement> mPlacement;
	bool mIsShowGuide{true};
	bool mIsShowIcon{true};

};

DefaultComponentPlacer::DefaultComponentPlacer(MainWindowPlacement* placement) : in(new PImpl)
{
	in->mPlacement.reset(placement);

	AppPreference* pref = AppPreference::Get();
	in->mIsShowGuide = pref->IsShowGuide();
	in->mIsShowIcon = pref->IsDrawIcon();
}

DefaultComponentPlacer::~DefaultComponentPlacer()
{
}


// アイコン欄のサイズ計算と配置
bool DefaultComponentPlacer::PlaceIcon(HWND elemHwnd, LauncherInput* status)
{
	UNREFERENCED_PARAMETER(status);

	if (in->mIsShowIcon == false) {
		return true;
	}

	int MARGIN_X = in->mPlacement->GetMarginLeft();
	int MARGIN_Y = in->mPlacement->GetMarginTop();

	// フォントサイズから説明欄(と入力欄)の高さを求める
	int fontH = in->mPlacement->GetFontPixelSize();
	int editH = fontH + 4;

	// 説明欄の高さ + 余白 + 入力欄の高さ
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
bool DefaultComponentPlacer::PlaceDescription(HWND elemHwnd, LauncherInput* status)
{
	UNREFERENCED_PARAMETER(status);
	int MARGIN_X{in->mPlacement->GetMarginLeft()};
	int MARGIN_Y{in->mPlacement->GetMarginTop()};
	int margin{2};

	// (アイコンを表示する場合)アイコン欄の右側に説明欄を配置する
	int iconXOffset = in->mIsShowIcon ? in->mPlacement->GetIconWindowWidth() + margin : 0;

	int x = MARGIN_X + iconXOffset;
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
bool DefaultComponentPlacer::PlaceGuide(HWND elemHwnd, LauncherInput* status)
{
	if (status == nullptr || status->HasKeyword() == false || in->mIsShowGuide == false) {
		// キーワード未入力状態のときはガイド欄を表示しない
		return true;
	}

	CRect rc;
	GetGuideRect(rc);

	// 説明欄の位置・サイズを移動
	spdlog::debug("comment xywh=({},{},{},{})", rc.left, rc.top, rc.Width(), rc.Height());
	SetWindowPos(elemHwnd, nullptr, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);

	return true;
}

void DefaultComponentPlacer::GetGuideRect(CRect& rc)
{
	if (in->mIsShowGuide == false) {
		rc.SetRect(0,0,0,0);
		return ;
	}

	int MARGIN_X{in->mPlacement->GetMarginLeft()};
	int MARGIN_Y{in->mPlacement->GetMarginTop()};

	int vscrool_w = GetSystemMetrics(SM_CXVSCROLL);

	// 親ウインドウの幅と、ガイド欄を配置した位置をもとに幅を決定
	int x = MARGIN_X;
	int cx = in->mPlacement->GetMainWindowWidth() - x - MARGIN_X - vscrool_w;

	// フォントサイズに応じて高さを決定する
	int fontH = in->mPlacement->GetFontPixelSize();
	int cy = fontH + 4;
	int y = in->mPlacement->GetMainWindowHeight() - cy - MARGIN_Y;

	rc = CRect(x, y, x+ cx, y + cy);

}

// 入力欄のサイズ計算と配置
bool DefaultComponentPlacer::PlaceEdit(HWND elemHwnd, LauncherInput* status)
{
	UNREFERENCED_PARAMETER(status);


	CRect rc;
	GetEditRect(rc);

	spdlog::debug("DefaultComponentPlacer edit xywh=({},{},{},{})", rc.left, rc.top, rc.Width(), rc.Height());
	SetWindowPos(elemHwnd, nullptr, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);

	return true;
}

void DefaultComponentPlacer::GetEditRect(CRect& rc)
{ 
	int MARGIN_X{in->mPlacement->GetMarginLeft()};
	int MARGIN_Y{in->mPlacement->GetMarginTop()};
	int margin{2};

	// (アイコンを表示する場合)アイコン欄の右側に説明欄を配置する
	int iconXOffset = in->mIsShowIcon ? in->mPlacement->GetIconWindowWidth() + margin : 0;

	int x = MARGIN_X + iconXOffset;

	// 説明欄の下に配置
	int y =	MARGIN_Y + in->mPlacement->GetDescriptionWindowHeight() + margin;

	// 親ウインドウの幅と、説明欄の配置した位置をもとに幅を決定
	int cx = in->mPlacement->GetMainWindowWidth() - x - MARGIN_X;

	// フォントサイズに応じて高さを決定する
	int fontH = in->mPlacement->GetFontPixelSize();
	int cy = fontH + 4;
	rc = CRect(x, y, x + cx, y + cy);
}


// 候補欄のサイズ計算と配置
bool DefaultComponentPlacer::PlaceCandidateList(HWND elemHwnd, LauncherInput* status)
{
	if (status == nullptr || status->HasKeyword() == false) {
		// キーワード未入力状態のときは表示しない
		return true;
	}

	int MARGIN_X{in->mPlacement->GetMarginLeft()};
	int MARGIN_Y{in->mPlacement->GetMarginTop()};
	int margin{in->mPlacement->GetMarginEditToList()};

	int x = MARGIN_X;


	// 入力欄の下、ガイド欄の上に配置する
	CRect rcEdit;
	GetEditRect(rcEdit);

	CRect rcGuide;
	GetGuideRect(rcGuide);

	int y =	rcEdit.bottom + margin;

	// 親ウインドウの幅と、ガイド欄を配置した位置をもとに幅を決定
	int cx = in->mPlacement->GetMainWindowWidth() - MARGIN_X * 2;

	int cy = in->mPlacement->GetMainWindowHeight() - rcEdit.bottom - margin - rcGuide.Height() - MARGIN_Y;

	// 説明欄の位置・サイズを移動
	spdlog::debug("candidate xywh=({},{},{},{})", x, y, cx, cy);

	SetWindowPos(elemHwnd, nullptr, x, y, cx, cy, SWP_NOZORDER);

	return true;
}


// 適用
void DefaultComponentPlacer::Apply(HWND hwnd, LauncherInput* status)
{
	UNREFERENCED_PARAMETER(hwnd);

	in->mPlacement->GetIconLabel()->ShowWindow(in->mIsShowIcon ? SW_SHOW : SW_HIDE);

	auto descLabel = in->mPlacement->GetDescriptionLabel();
	descLabel->InvalidateRect(nullptr);
	descLabel->UpdateWindow();

	auto guideLabel = in->mPlacement->GetGuideLabel();
	bool isShowGuide = in->mIsShowGuide && (status && status->HasKeyword());
	if (isShowGuide) {
		guideLabel->ShowWindow(SW_SHOW);
		guideLabel->InvalidateRect(nullptr);
		guideLabel->UpdateWindow();
	}
	else {
		guideLabel->ShowWindow(SW_HIDE);
	}

	auto edit = in->mPlacement->GetEdit();
	edit->InvalidateRect(nullptr);
	edit->UpdateWindow();

	auto list = in->mPlacement->GetCandidateList();
	if (status && status->HasKeyword()) {
		list->ShowWindow(SW_SHOW);
		list->InvalidateRect(nullptr);
		list->UpdateWindow();
	}
	else {
		list->ShowWindow(SW_HIDE);
	}
}

// それより以下のリサイズを許容しない最小限の高さを取得する
int DefaultComponentPlacer::GetMinimumHeight()
{
	int margin = 2;
	auto& p = in->mPlacement;
	int h = p->GetMarginTop() + 
	        p->GetDescriptionWindowHeight() +
	        margin + 
	        p->GetEditWindowHeight() +
	        margin + 
	        p->GetMarginTop() + 1;

	spdlog::debug("MinimumHeight is {0} ({1} {2})",
		 	h, p->GetDescriptionWindowHeight(),p->GetEditWindowHeight()); 
	return h;
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


