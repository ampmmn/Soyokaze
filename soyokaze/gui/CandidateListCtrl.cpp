#include "pch.h"
#include "CandidateListCtrl.h"
#include "CandidateList.h"
#include "AppPreference.h"
#include "resource.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static bool IsHighContrastMode()
{
	HIGHCONTRAST hc;
	hc.cbSize = sizeof(HIGHCONTRAST);
	SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRAST), &hc, 0);
	return (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;
}
struct CandidateListCtrl::PImpl
{
	void DrawItemName(CListCtrl* thisPtr, CDC* pDC, int itemId);
	void DrawItemCategory(CListCtrl* thisPtr, CDC* pDC, int itemId);
	CandidateList* mCandidates = nullptr;

	bool mHasCommandTypeColumn = false;

	int mItemsInPage;
};

/**
	名前列の描画
	@param[in,out] thisPtr 
	@param[in,out] pDC     
	@param[in]     itemId  
*/
void CandidateListCtrl::PImpl::DrawItemName(
	CListCtrl* thisPtr,
	CDC* pDC,
	int itemId
)
{
	CRect rcItem;
	thisPtr->GetSubItemRect(itemId, 0, LVIR_LABEL, rcItem);
	auto cmd = mCandidates->GetCommand(itemId);
	pDC->DrawText(cmd->GetName(), rcItem, DT_LEFT);
}

/**
 	種別の描画
 	@param[in,out] thisPtr 
 	@param[in,out] pDC     
 	@param[in]     itemId  
*/
void CandidateListCtrl::PImpl::DrawItemCategory(
	CListCtrl* thisPtr,
	CDC* pDC,
	int itemId
)
{
	CRect rcItem;
	thisPtr->GetSubItemRect(itemId, 1, LVIR_LABEL, rcItem);
	auto cmd = mCandidates->GetCommand(itemId);
	pDC->DrawText(cmd->GetTypeDisplayName(), rcItem, DT_LEFT);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CandidateListCtrl::CandidateListCtrl() : in(new PImpl)
{
	AppPreference::Get()->RegisterListener(this);
}

CandidateListCtrl::~CandidateListCtrl()
{
}

BEGIN_MESSAGE_MAP(CandidateListCtrl, CListCtrl)


END_MESSAGE_MAP()

void CandidateListCtrl::SetCandidateList(CandidateList* candidates)
{
	in->mCandidates = candidates;
	candidates->AddListener(this);
}


void CandidateListCtrl::InitColumns()
{
	if (GetSafeHwnd() == nullptr) {
		return;
	}

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

	AppPreference* pref= AppPreference::Get();

	CString strHeader;
	strHeader.LoadString(IDS_NAME);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = pref->IsShowCommandType() ? cx - 165 : cx - 25;
	lvc.fmt = LVCFMT_LEFT;
	InsertColumn(0,&lvc);

	in->mHasCommandTypeColumn = false;
	if (pref->IsShowCommandType()) {
		strHeader.LoadString(IDS_COMMANDTYPE);
		lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
		lvc.cx = 140;  //  #dummy-col-width
		lvc.fmt = LVCFMT_LEFT;
		InsertColumn(1,&lvc);
		in->mHasCommandTypeColumn = true;
	}
}

void CandidateListCtrl::UpdateSize(int cx, int cy)
{
	if (in->mHasCommandTypeColumn) {
		SetColumnWidth(0, cx-165);
		SetColumnWidth(1, 140);
	}
	else {
		SetColumnWidth(0, cx-25);
	}
}

int CandidateListCtrl::GetItemCountInPage()
{
	return in->mItemsInPage;
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

	SetItemCountEx(candidates->GetSize());
	if (candidates->GetSize() > 0) {
		SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	}

	Invalidate();
}


void CandidateListCtrl::DrawItem(
	LPDRAWITEMSTRUCT lpDrawItemStruct
)
{
	CRect rcCtrl;
	GetClientRect(&rcCtrl);

	CRect rcItem = lpDrawItemStruct->rcItem;

	// 画面内のアイテム数
	in->mItemsInPage = rcCtrl.Height() / rcItem.Height();

	int itemID = lpDrawItemStruct->itemID;
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL isSelect = (lpDrawItemStruct->itemState & ODS_SELECTED);

	// 色の定義
	COLORREF crText = GetSysColor(COLOR_WINDOWTEXT);
	COLORREF crBk = GetSysColor(COLOR_WINDOW);
	BYTE rgb[] = { GetRValue(crBk), GetGValue(crBk), GetBValue(crBk) };

	// 背景色を行単位で交互に変えるときの一方の色を決める
	// (基準とする色から6%ほど弱めた感じにしてみる)
	if (*(std::max_element(rgb, rgb+3)) > 128) {
		// 明るい寄り(というかたいてい白のはず..)の場合は黒方向に近づける
		rgb[0] = BYTE(rgb[0] * 0.94);
		rgb[1] = BYTE(rgb[1] * 0.94);
		rgb[2] = BYTE(rgb[2] * 0.94);
	}
	else {
		// 暗い寄り(ハイコントラストモードで動いている場合とか..)の場合は白方向に近づける
		rgb[0] = BYTE(rgb[0] + BYTE((255 - rgb[0]) * 0.06));
		rgb[1] = BYTE(rgb[1] + BYTE((255 - rgb[1]) * 0.06));
		rgb[2] = BYTE(rgb[2] + BYTE((255 - rgb[2]) * 0.06));
	}
	COLORREF crBk2 = RGB(rgb[0], rgb[1], rgb[2]);

	// 背景の消去
	CBrush brBk;
	brBk.CreateSolidBrush((itemID%2) ? crBk2 : crBk);
	pDC->FillRect(rcItem,&brBk);

	// 選択領域の塗りつぶし
	if (isSelect) {
		CBrush brSelect;
		brSelect.CreateSysColorBrush(COLOR_HIGHLIGHT);
		crText = ::GetSysColor(COLOR_HIGHLIGHTTEXT); // or GetSysColor(COLOR_WINDOW);

		CRect rcSelect;
		GetItemRect(itemID, rcSelect, LVIR_BOUNDS);

		// // アイコン領域取得
		// const int c_nMargin = 3;
		// CRect rcIcon;
		// GetItemRect(nItem,rcIcon,LVIR_ICON);
		// rcSelect.left = rcIcon.right-c_nMargin; // アイコン領域は含めない

		pDC->FillRect(rcSelect,&brSelect);
	}

	int orgTextColor = pDC->SetTextColor(crText);

	in->DrawItemName(this, pDC, itemID);
	in->DrawItemCategory(this, pDC, itemID);

	pDC->SetTextColor(orgTextColor);
}

void CandidateListCtrl::OnAppFirstBoot()
{
	// 初回起動時、これが呼ばれる時点でこのクラスはたぶんインスタンス化されてない
}

void CandidateListCtrl::OnAppPreferenceUpdated()
{
	// カラムを再生成する
	InitColumns();
}

void CandidateListCtrl::OnAppExit()
{
	AppPreference::Get()->UnregisterListener(this);
}
