#include "pch.h"
#include "CandidateListCtrl.h"
#include "CandidateList.h"
#include "CandidateListRenderer.h"
#include "StandardCandidateListRenderer.h"
#include "BGImageCandidateListRenderer.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreference.h"
#include "utility/Accessibility.h"
#include "utility/ScopedDCState.h"
#include "control/ColorSettings.h"
#include "resource.h"
#include <algorithm>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;
constexpr int ITEM_MARGIN = 4;

struct CandidateListCtrl::PImpl
{
	CandidateList* mCandidates{nullptr};
	std::unique_ptr<CandidateListRenderer> mRenderer;

	bool mHasCommandTypeColumn{false};
	bool mShouldReinitColumns{true};

	bool mIsEmpty{false};
	int mTextHeight{16};
	int mIconSize{16};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CandidateListCtrl::CandidateListCtrl() : in(std::make_unique<PImpl>())
{
	AppPreference::Get()->RegisterListener(this, _T("CandidateListCtrl"));
}

CandidateListCtrl::~CandidateListCtrl()
{
	AppPreference::Get()->UnregisterListener(this);
}

BEGIN_MESSAGE_MAP(CandidateListCtrl, CListCtrl)
END_MESSAGE_MAP()

// 現在選択中のアイテムの領域を取得
bool CandidateListCtrl::GetCurrentItemRect(RECT* rect)
{
	if (GetSafeHwnd() == nullptr) {
		return false;
	}

	POSITION pos = GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return false;
	}

	int selItemIndex = GetNextSelectedItem(pos);
	CRect rcItem;
	GetItemRect(selItemIndex, &rcItem, LVIR_BOUNDS);

	if (rect) {
		*rect = rcItem;
	}

	return true;
}

void CandidateListCtrl::SetCandidateList(CandidateList* candidates)
{
	in->mCandidates = candidates;
	candidates->AddListener(this);
}

/**
 	コマンド種別の列サイズを決定する
 	@param[in]  hwnd         CListCtrlウインドウハンドル
 	@param[out] typeColWidth コマンド種別列の幅(pixel)
 	@param[out] textHeight   テキストの高さ(pixel)
*/
static void GetTypeColumnSize(HWND hwnd, int& typeColWidth, int& textHeight)
{
	HFONT hf = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
	CClientDC dc(CWnd::FromHandle(hwnd));
	ScopedDCState dcstate(&dc);

	dc.SelectObject(hf);

	std::vector<CString> displayNames;
	auto cmdRepo = CommandRepository::GetInstance();
	cmdRepo->EnumCommandDisplayNames(displayNames);

	int maxWidth = 0;

	// コマンド種別テキストをすべて取得して、最大のものを得る
	for (auto& typeName : displayNames) {
		CSize size;
		GetTextExtentPoint32(dc.GetSafeHdc(), typeName, typeName.GetLength(), &size);
		maxWidth = (std::max)(maxWidth, (int)size.cx);
	}

	TEXTMETRIC tm;
	GetTextMetrics(dc.GetSafeHdc(), &tm);

	spdlog::debug("typecol len:{}", maxWidth);

	typeColWidth = maxWidth;
	textHeight = tm.tmHeight;
}


void CandidateListCtrl::InitColumns()
{
	AppPreference* pref= AppPreference::Get();
	bool isShowCommandType = pref->IsShowCommandType();

	if (GetSafeHwnd() == nullptr) {
		return;
	}

	// 列幅、高さを計算する
	int typeColWidth = 140;
	GetTypeColumnSize(GetSafeHwnd(), typeColWidth, in->mIconSize);
	in->mTextHeight = in->mIconSize;
	in->mIconSize += ITEM_MARGIN;

	ModifyStyle(0, LVS_OWNERDATA);
	SetExtendedStyle(GetExtendedStyle()|LVS_EX_FULLROWSELECT| LVS_EX_DOUBLEBUFFER);

	// カラムをいったん全削除する
	int nCurrentCols = GetHeaderCtrl()->GetItemCount();
	for (int i = 0; i < nCurrentCols; ++i) {
		DeleteColumn(0);
	}

	// ヘッダー追加
	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	CRect rect;
	GetParent()->GetClientRect(rect);

	int cx = rect.Width();


	// 交互別色表示設定
	bool isAlternateColor = pref->IsAlternateColor();

	CString strHeader;
	strHeader.LoadString(IDS_NAME);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = isShowCommandType ? cx - (typeColWidth-25) : cx - 25;
	lvc.fmt = LVCFMT_LEFT;
	InsertColumn(0,&lvc);

	in->mHasCommandTypeColumn = false;
	if (isShowCommandType) {
		strHeader.LoadString(IDS_COMMANDTYPE);
		lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
		lvc.cx = typeColWidth;
		lvc.fmt = LVCFMT_LEFT;
		InsertColumn(1,&lvc);
		in->mHasCommandTypeColumn = true;
	}

	// 設定に応じたレンダラーを作り直す。
	std::unique_ptr<StandardCandidateListRenderer> renderer;
	bool isUseBGImage = pref->GetSettings().Get(_T("BGImage:Enable"), false);
	if (isUseBGImage) {
		renderer = std::make_unique<BGImageCandidateListRenderer>();
	}
	else {
		renderer = std::make_unique<StandardCandidateListRenderer>();
	}
	renderer->SetCandidateList(in->mCandidates);
	renderer->SetIsEmpty(in->mIsEmpty);
	renderer->SetIsAlternateColor(isAlternateColor);
	renderer->SetIsShowCommandType(isShowCommandType);
	renderer->SetIsDrawIcon(pref->IsDrawIconOnCandidate());
	renderer->SetTextMetrics(in->mTextHeight, in->mIconSize);
	SetImageList(renderer->GetImageList(), LVSIL_SMALL);
	in->mRenderer = std::move(renderer);

	in->mShouldReinitColumns = false;
}

/**
 	
 	@param[in] cx 親ウインドウの幅
 	@param[in] cy  親ウインドウの高さ
*/
void CandidateListCtrl::UpdateSize(int cx, int cy)
{
	if (in->mShouldReinitColumns) {
		InitColumns();
	}
	if (in->mRenderer) {
		in->mRenderer->UpdateSize(cx, cy);
	}
	// スクロールバーの幅
	int SCROLLBAR_WIDTH =  GetSystemMetrics(SM_CXVSCROLL);
	spdlog::debug("SCROLLBAR_WIDTH {}", SCROLLBAR_WIDTH);

	// コントロールの幅
	CRect rcClient;
	GetClientRect(rcClient);
	int width = rcClient.Width();

	if (in->mHasCommandTypeColumn) {

		// コマンド種別の列幅を得る
		int typeColWidth = 140;
		GetTypeColumnSize(GetSafeHwnd(), typeColWidth, in->mTextHeight);

		int nameColWidth = width - (typeColWidth + SCROLLBAR_WIDTH);
		if (nameColWidth < typeColWidth) {
			// コマンド名の列幅が種別の列幅より小さくなる場合は、種別の列幅を縮める
			std::swap(nameColWidth, typeColWidth);
		}
		SetColumnWidth(0, nameColWidth);
		SetColumnWidth(1, typeColWidth);

	}
	else {
		SetColumnWidth(0, width - SCROLLBAR_WIDTH);
	}
}

int CandidateListCtrl::GetItemCountInPage()
{
	return in->mRenderer ? in->mRenderer->GetItemCountInPage() : 0;
}

void CandidateListCtrl::OnMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight = in->mTextHeight + ITEM_MARGIN;
}

void CandidateListCtrl::OnUpdateSelect(void* sender)
{
	CandidateList* candidates = (CandidateList*)sender;
	int selIndex = candidates->GetCurrentSelect();

	SetItemState(selIndex, LVIS_SELECTED, LVIS_SELECTED);
	EnsureVisible(selIndex, FALSE);

	Invalidate();
}

void CandidateListCtrl::OnUpdateItems(void* sender)
{
	CandidateList* candidates = (CandidateList*)sender;

	int count = candidates->GetSize();

	// アイテム数が0のときでも背景を交互で描画できるようにするため、ダミーの項目数を1つだけ挟む
	in->mIsEmpty = candidates->IsEmpty();
	if (in->mRenderer) {
		in->mRenderer->SetIsEmpty(in->mIsEmpty);
	}
	SetItemCountEx(in->mIsEmpty ? 1 : count);
	if (count > 0) {
		SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

		// 選択項目が可視領域に収まるようにする
		CRect rcItem;
		GetItemRect(0, &rcItem, LVIR_BOUNDS);
		CSize sizeScroll(0, -rcItem.Height() * count);
		Scroll(sizeScroll);
		// Note: EnsureVisible(0, FALSE)の場合、アプリ起動後1回目の表示のときだけ
		//       先頭の候補が表示されなかったので、Scrollを使用している
	}

	Invalidate();
}


void CandidateListCtrl::DrawItem(
	LPDRAWITEMSTRUCT lpDrawItemStruct
)
{
	if (in->mRenderer) {
		in->mRenderer->DrawItem(this, lpDrawItemStruct);
	}
}

void CandidateListCtrl::OnAppFirstBoot()
{
	// 初回起動時、これが呼ばれる時点でこのクラスはたぶんインスタンス化されてない
}

void CandidateListCtrl::OnAppNormalBoot()
{
}

void CandidateListCtrl::OnAppPreferenceUpdated()
{
	in->mShouldReinitColumns = true;
}

void CandidateListCtrl::OnAppExit()
{
}

