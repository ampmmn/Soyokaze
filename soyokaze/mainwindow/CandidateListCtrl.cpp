#include "pch.h"
#include "CandidateListCtrl.h"
#include "CandidateList.h"
#include "setting/AppPreference.h"
#include "utility/Accessibility.h"
#include "gui/ColorSettings.h"
#include "resource.h"
#include <algorithm>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr int ICON_SIZE = 16;

struct CandidateListCtrl::PImpl
{
	void DrawItemIcon(CListCtrl* thisPtr, CDC* pDC, int itemId);
	void DrawItemName(CListCtrl* thisPtr, CDC* pDC, int itemId);
	void DrawItemCategory(CListCtrl* thisPtr, CDC* pDC, int itemId);
	CandidateList* mCandidates = nullptr;

	bool mHasCommandTypeColumn = false;

	bool mIsEmpty = false;

	// 背景色を交互に色を変える
	bool mIsAlternateColor = false;

	int mItemsInPage = 0;

	// アイコンを保持するためのイメージリスト
	CImageList mIconList;
	// アイコンを表示しない場合のイメージリスト
	CImageList mIconListDummy;
	bool mIsDrawIcon = true;

	// 種別を描画するか
	bool mIsShowCommandType = false;
	//
	std::map<HICON,int> mIconIndexMap;
};

/**
	アイコンの描画
	@param[in,out] thisPtr 
	@param[in,out] pDC     
	@param[in]     itemId  
*/
void CandidateListCtrl::PImpl::DrawItemIcon(
	CListCtrl* thisPtr,
	CDC* pDC,
	int itemId
)
{
	if (mIsDrawIcon == false) {
		return;
	}

	CRect rcIcon;
	thisPtr->GetSubItemRect(itemId, 0, LVIR_ICON, rcIcon);

	auto cmd = mCandidates->GetCommand(itemId);

	int index = -1;

	HICON h = cmd->GetIcon();
	auto it = mIconIndexMap.find(h);
	if (it == mIconIndexMap.end()) {
		index = mIconList.Add(h);
		mIconIndexMap[h] = index;
	}
	else {
		index = it->second;
	}

	if (index != -1) {
		mIconList.DrawEx(pDC, index, rcIcon.TopLeft(), CSize(ICON_SIZE, ICON_SIZE),
		                 CLR_NONE,  CLR_DEFAULT, ILD_NORMAL);
	}
}

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

	// 改行を文字化する
	CString name = cmd->GetName();
	name.Replace(_T("\n"), _T("\\n"));
	name.Replace(_T("\t"), _T("  "));

	pDC->DrawText(name, rcItem, DT_LEFT);
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
	if (mIsShowCommandType == false) {
		return;
	}
	CRect rcItem;
	thisPtr->GetSubItemRect(itemId, 1, LVIR_LABEL, rcItem);
	auto cmd = mCandidates->GetCommand(itemId);
	pDC->DrawText(cmd->GetTypeDisplayName(), rcItem, DT_LEFT);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CandidateListCtrl::CandidateListCtrl() : in(std::make_unique<PImpl>())
{
	AppPreference::Get()->RegisterListener(this);
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


void CandidateListCtrl::InitColumns()
{
	AppPreference* pref= AppPreference::Get();
	in->mIsShowCommandType = pref->IsShowCommandType();

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


	// 交互別色表示設定
	in->mIsAlternateColor = pref->IsAlternateColor();

	CString strHeader;
	strHeader.LoadString(IDS_NAME);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = in->mIsShowCommandType ? cx - 165 : cx - 25;
	lvc.fmt = LVCFMT_LEFT;
	InsertColumn(0,&lvc);

	in->mHasCommandTypeColumn = false;
	if (in->mIsShowCommandType) {
		strHeader.LoadString(IDS_COMMANDTYPE);
		lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
		lvc.cx = 140;  //  #dummy-col-width
		lvc.fmt = LVCFMT_LEFT;
		InsertColumn(1,&lvc);
		in->mHasCommandTypeColumn = true;
	}

	// イメージリストの初期化
	if (in->mIconList.m_hImageList == NULL) {
		in->mIconList.Create(ICON_SIZE, ICON_SIZE, ILC_COLOR24 | ILC_MASK, 0, 0);
		in->mIconListDummy.Create(1, 1, ILC_COLOR, 0, 0);
	}
	ASSERT(in->mIconList.m_hImageList);

	in->mIsDrawIcon = pref->IsDrawIconOnCandidate();
	SetImageList(in->mIsDrawIcon ? &in->mIconList : &in->mIconListDummy, LVSIL_SMALL);
	// Note: ひとたび、SetImageListでイメージリストを設定すると、
	// そのリストウインドウが生きている間はイメージリスト非設定状態に戻せない(イメージリストの幅のぶんだけラベルがずれる)ため、
	// 幅1pixelの別のイメージリストを設定することでごまかす
}

void CandidateListCtrl::UpdateSize(int cx, int cy)
{
	UNREFERENCED_PARAMETER(cy);

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

	int count = candidates->GetSize();

	// アイテム数が0のときでも背景を交互で描画できるようにするため、ダミーの項目数を1つだけ挟む
	in->mIsEmpty = candidates->IsEmpty();
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
	CRect rcCtrl;
	GetClientRect(&rcCtrl);

	CRect rcItem = lpDrawItemStruct->rcItem;

	// 画面内のアイテム数
	in->mItemsInPage = rcCtrl.Height() / rcItem.Height();

	int itemID = lpDrawItemStruct->itemID;
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL isSelect = (lpDrawItemStruct->itemState & ODS_SELECTED);


	// 色の定義
	auto colorSettings = ColorSettings::Get();
	auto colorScheme = colorSettings->GetCurrentScheme();


	HBRUSH brBk1 = colorScheme->GetListBackgroundBrush();
	HBRUSH brBk2 = colorScheme->GetListBackgroundAltBrush();
	if (in->mIsAlternateColor == false) {
		brBk2 = brBk1;
	}

	if (in->mIsEmpty) {
		// ToDo: 末尾の塗りつぶしとここの塗りつぶしの処理を関数化する

		// 要素数が空の場合の塗りつぶし処理
		HBRUSH p = brBk2;
		while (rcItem.top < rcCtrl.Height()) {
			p = (p == brBk1)? brBk2 : brBk1;
			pDC->FillRect(rcItem, CBrush::FromHandle(p));
			rcItem.OffsetRect(0, rcItem.Height());
		}
		return;
	}

	// 背景の消去
	HBRUSH hbr = (itemID%2) ? brBk2 : brBk1;
	pDC->FillRect(rcItem, CBrush::FromHandle(hbr));

	// 選択領域の塗りつぶし
	COLORREF crText = colorScheme->GetListTextColor();
	if (isSelect) {
		crText = colorScheme->GetListHighlightTextColor();

		CRect rcSelect;
		GetItemRect(itemID, rcSelect, LVIR_BOUNDS);

		// // アイコン領域取得
		// const int c_nMargin = 3;
		// CRect rcIcon;
		// GetItemRect(nItem,rcIcon,LVIR_ICON);
		// rcSelect.left = rcIcon.right-c_nMargin; // アイコン領域は含めない

		pDC->FillRect(rcSelect, CBrush::FromHandle(colorScheme->GetListHighlightBackgroundBrush()));
	}

	int orgTextColor = pDC->SetTextColor(crText);

	in->DrawItemIcon(this, pDC, itemID);
	in->DrawItemName(this, pDC, itemID);
	in->DrawItemCategory(this, pDC, itemID);

	if (itemID == in->mCandidates->GetSize()-1) {
		// 末尾の要素に達したら、リストの最後まで背景を交互にぬる
		rcItem.OffsetRect(0, rcItem.Height());

		HBRUSH p = (itemID % 2) ? brBk2 : brBk1;
		while (rcItem.top < rcCtrl.Height()) {
			p = (p == brBk1)? brBk2 : brBk1;
			pDC->FillRect(rcItem, CBrush::FromHandle(p));
			rcItem.OffsetRect(0, rcItem.Height());
		}
	}

	pDC->SetTextColor(orgTextColor);
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
	// カラムを再生成する
	InitColumns();
}

void CandidateListCtrl::OnAppExit()
{
}

