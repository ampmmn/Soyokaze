#include "pch.h"
#include "ExtraCandidateListCtrl.h"
#include "control/ColorSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

constexpr int MAX_VISIBLE_ROWS = 16;

}

class ExtraCandidatePopupWnd : public CWnd
{
public:
	ExtraCandidatePopupWnd(CWnd* owner) :
		mOwner(owner)
	{
	}

	bool CreatePopup()
	{
		CRect rect(0, 0, 320, 100);
		return CWnd::CreateEx(
			WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
			AfxRegisterWndClass(0),
			_T(""),
			WS_POPUP,
			rect,
			mOwner,
			0) != FALSE;
	}

	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* result) override
	{
		// 子リストが処理する通知を先にリフレクションする
		if (CWnd::OnNotify(wParam, lParam, result)) {
			return TRUE;
		}
		if (mOwner != nullptr) {
			*result = mOwner->SendMessage(WM_NOTIFY, wParam, lParam);
			return TRUE;
		}
		return CWnd::OnNotify(wParam, lParam, result);
	}

protected:
	DECLARE_MESSAGE_MAP()

private:
	CWnd* mOwner;
};

BEGIN_MESSAGE_MAP(ExtraCandidatePopupWnd, CWnd)
END_MESSAGE_MAP()

ExtraCandidateListCtrl::ExtraCandidateListCtrl()
{
}

ExtraCandidateListCtrl::~ExtraCandidateListCtrl()
{
	if (mPopupWnd != nullptr && mPopupWnd->GetSafeHwnd() != nullptr) {
		mPopupWnd->DestroyWindow();
	}
}

BEGIN_MESSAGE_MAP(ExtraCandidateListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()

/**
  本体候補欄と同じ配色で候補行を描画する
  @param[in] pNMHDR カスタムドロー通知情報
  @param[out] pResult 通知処理結果
*/
void ExtraCandidateListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (pNMHDR == nullptr || pResult == nullptr) {
		return;
	}

	auto* customDraw = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	if (customDraw->nmcd.dwDrawStage == CDDS_PREPAINT) {
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	}
	if (customDraw->nmcd.dwDrawStage != CDDS_ITEMPREPAINT) {
		*pResult = CDRF_DODEFAULT;
		return;
	}

	int itemId = static_cast<int>(customDraw->nmcd.dwItemSpec);
	auto colorScheme = ColorSettings::Get()->GetCurrentScheme();
	bool isSelected = (GetItemState(itemId, LVIS_SELECTED) & LVIS_SELECTED) != 0;
	if (isSelected) {
		customDraw->clrText = colorScheme->GetListHighlightTextColor();
		customDraw->clrTextBk = colorScheme->GetListHighlightBackgroundColor();
	}
	else if ((itemId % 2) != 0) {
		customDraw->clrText = colorScheme->GetListTextColor();
		customDraw->clrTextBk = colorScheme->GetListBackgroundAltColor();
	}
	else {
		customDraw->clrText = colorScheme->GetListTextColor();
		customDraw->clrTextBk = colorScheme->GetListBackgroundColor();
	}

	CRect itemRect;
	CRect clientRect;
	GetItemRect(itemId, &itemRect, LVIR_BOUNDS);
	GetClientRect(&clientRect);
	itemRect.right = clientRect.right;
	CBrush* backgroundBrush = nullptr;
	if (isSelected) {
		backgroundBrush = CBrush::FromHandle(colorScheme->GetListHighlightBackgroundBrush());
	}
	else if ((itemId % 2) != 0) {
		backgroundBrush = CBrush::FromHandle(colorScheme->GetListBackgroundAltBrush());
	}
	else {
		backgroundBrush = CBrush::FromHandle(colorScheme->GetListBackgroundBrush());
	}
	if (backgroundBrush != nullptr) {
		CDC* dc = CDC::FromHandle(customDraw->nmcd.hdc);
		dc->FillRect(itemRect, backgroundBrush);

		CRect textRect = itemRect;
		textRect.DeflateRect(4, 0);
		int oldBkMode = dc->SetBkMode(TRANSPARENT);
		COLORREF oldTextColor = dc->SetTextColor(customDraw->clrText);
		dc->DrawText(GetItemText(itemId, 0), textRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
		dc->SetTextColor(oldTextColor);
		dc->SetBkMode(oldBkMode);
	}

	*pResult = CDRF_SKIPDEFAULT;
}

bool ExtraCandidateListCtrl::CreatePopup(CWnd* owner)
{
	if (owner == nullptr || owner->GetSafeHwnd() == nullptr) {
		return false;
	}

	mPopupWnd = std::make_unique<ExtraCandidatePopupWnd>(owner);
	if (mPopupWnd->CreatePopup() == false) {
		mPopupWnd.reset();
		return false;
	}

	CRect rect(0, 0, 320, 100);
	DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SINGLESEL | LVS_SHOWSELALWAYS;
	if (CListCtrl::CreateEx(0, style, rect, mPopupWnd.get(), 1) == FALSE) {
		mPopupWnd->DestroyWindow();
		mPopupWnd.reset();
		return false;
	}
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	SetPopupFont(owner->GetFont());
	InsertColumn(0, _T(""), LVCFMT_LEFT, 300);
	return true;
}

void ExtraCandidateListCtrl::SetPopupFont(CFont* font)
{
	if (font == nullptr) {
		return;
	}

	if (mPopupWnd != nullptr && mPopupWnd->GetSafeHwnd() != nullptr) {
		mPopupWnd->SetFont(font);
	}
	if (GetSafeHwnd() != nullptr) {
		SetFont(font);

		CClientDC dc(this);
		CFont* oldFont = dc.SelectObject(font);
		TEXTMETRIC tm{};
		if (GetTextMetrics(dc.GetSafeHdc(), &tm) != FALSE) {
			mRowHeight = tm.tmHeight + 4;
		}
		dc.SelectObject(oldFont);

		if (IsPopupVisible()) {
			ResizePopup();
		}
	}
}

int ExtraCandidateListCtrl::GetRowHeight() const
{
	return mRowHeight;
}

/**
  Popupの高さと子リストのサイズを現在の候補数・行高に合わせて更新する
*/
void ExtraCandidateListCtrl::ResizePopup()
{
	if (mCandidates.empty() || mPopupWnd == nullptr || mPopupWnd->GetSafeHwnd() == nullptr) {
		return;
	}

	int visibleRows = (std::min)(static_cast<int>(mCandidates.size()), MAX_VISIBLE_ROWS);
	int height = visibleRows * mRowHeight + 4;
	mPopupWnd->SetWindowPos(nullptr, 0, 0, 320, height,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

	CRect clientRect;
	mPopupWnd->GetClientRect(&clientRect);
	MoveWindow(&clientRect, TRUE);
}

void ExtraCandidateListCtrl::SetCandidates(
	const std::vector<RefPtr<launcherapp::core::Command>>& candidates
)
{
	mCandidates = candidates;
	DeleteAllItems();
	for (int i = 0; i < static_cast<int>(mCandidates.size()); ++i) {
		InsertItem(i, mCandidates[i]->GetName());
	}
	if (mCandidates.empty() == false) {
		SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
}

void ExtraCandidateListCtrl::SelectByName(const CString& name)
{
	if (name.IsEmpty()) {
		return;
	}
	for (int i = 0; i < static_cast<int>(mCandidates.size()); ++i) {
		if (mCandidates[i]->GetName() == name) {
			SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			EnsureVisible(i, FALSE);
			return;
		}
	}
}

void ExtraCandidateListCtrl::ShowAt(const CPoint& screenPos)
{
	if (mCandidates.empty() || mPopupWnd == nullptr || mPopupWnd->GetSafeHwnd() == nullptr) {
		HidePopup();
		return;
	}
	ResizePopup();
	mPopupWnd->SetWindowPos(&wndTop, screenPos.x, screenPos.y, 0, 0,
	             SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSIZE);
}

void ExtraCandidateListCtrl::HidePopup()
{
	if (mPopupWnd != nullptr && mPopupWnd->GetSafeHwnd() != nullptr) {
		mPopupWnd->ShowWindow(SW_HIDE);
	}
}

bool ExtraCandidateListCtrl::IsPopupVisible() const
{
	return mPopupWnd != nullptr && mPopupWnd->GetSafeHwnd() != nullptr && mPopupWnd->IsWindowVisible() != FALSE;
}

void ExtraCandidateListCtrl::OffsetSelection(int offset)
{
	if (mCandidates.empty()) {
		return;
	}
	POSITION pos = GetFirstSelectedItemPosition();
	int current = pos ? GetNextSelectedItem(pos) : 0;
	int next = current + offset;
	if (next < 0) {
		next = static_cast<int>(mCandidates.size()) - 1;
	}
	if (next >= static_cast<int>(mCandidates.size())) {
		next = 0;
	}
	SetItemState(current, 0, LVIS_SELECTED | LVIS_FOCUSED);
	SetItemState(next, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	EnsureVisible(next, FALSE);
}

launcherapp::core::Command* ExtraCandidateListCtrl::GetCurrentCommand() const
{
	POSITION pos = GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return nullptr;
	}
	int index = GetNextSelectedItem(pos);
	return index >= 0 && index < static_cast<int>(mCandidates.size()) ? const_cast<launcherapp::core::Command*>(mCandidates[index].get()) : nullptr;
}
