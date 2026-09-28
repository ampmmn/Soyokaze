#include "pch.h"
#include "StandardCandidateListRenderer.h"
#include "CandidateList.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreference.h"
#include "utility/Accessibility.h"
#include "utility/ScopedDCState.h"
#include "control/ColorSettings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;

constexpr int ITEM_MARGIN = 4;
// 二行表示の描画領域を少し重ねて、項目の高さを抑える。
constexpr int TWO_LINE_OVERLAP = 2;

struct StandardCandidateListRenderer::PImpl
{
	void DrawItemIcon(CListCtrl* listWnd, CDC* pDC, int itemId);
	void DrawItemName(CListCtrl* listWnd, CDC* pDC, int itemId);
	void DrawItemCategory(CListCtrl* listWnd, CDC* pDC, int itemId);

	CandidateList* mCandidates{nullptr};
	bool mIsEmpty{false};
	bool mIsAlternateColor{false};
	bool mIsShowCommandType{false};
	bool mIsDrawIcon{true};
	bool mIsDrawBackground{true};
	bool mIsTwoLine{false};
	int mItemsInPage{0};
	int mTextHeight{16};
	int mIconSize{16};
	std::unique_ptr<CImageList> mIconList;
	CImageList mIconListDummy;
	std::map<HICON, int> mIconIndexMap;
	CFont mDescriptionFont;
	HFONT mDescriptionFontSource{nullptr};
};

StandardCandidateListRenderer::StandardCandidateListRenderer() : in(new PImpl)
{
}

StandardCandidateListRenderer::~StandardCandidateListRenderer()
{
}

void StandardCandidateListRenderer::SetCandidateList(CandidateList* candidates)
{
	in->mCandidates = candidates;
}

void StandardCandidateListRenderer::SetIsAlternateColor(bool isAlternateColor)
{
	in->mIsAlternateColor = isAlternateColor;
}

void StandardCandidateListRenderer::SetIsShowCommandType(bool isShowCommandType)
{
	in->mIsShowCommandType = isShowCommandType;
}

void StandardCandidateListRenderer::SetIsDrawIcon(bool isDrawIcon)
{
	in->mIsDrawIcon = isDrawIcon;
}

void StandardCandidateListRenderer::SetIsDrawBackground(bool isDrawBackground)
{
	in->mIsDrawBackground = isDrawBackground;
}

/**
  項目名を二行表示するかどうかを設定する
  @param[in] isTwoLine 二行表示する場合はtrue
*/
void StandardCandidateListRenderer::SetIsTwoLine(bool isTwoLine)
{
	in->mIsTwoLine = isTwoLine;
}

void StandardCandidateListRenderer::SetTextMetrics(int textHeight, int textLineHeight, int iconSize)
{
	in->mTextHeight = textHeight;
	UNREFERENCED_PARAMETER(textLineHeight);
	in->mIconSize = iconSize;

	if (in->mIconListDummy.m_hImageList == nullptr) {
		in->mIconListDummy.Create(1, 1, ILC_COLOR, 0, 0);
	}
	in->mIconList = std::make_unique<CImageList>();
	in->mIconList->Create(in->mIconSize, in->mIconSize, ILC_COLOR24 | ILC_MASK, 0, 0);
	in->mIconIndexMap.clear();
}

CImageList* StandardCandidateListRenderer::GetImageList()
{
	return in->mIsDrawIcon ? in->mIconList.get() : &in->mIconListDummy;
}

void StandardCandidateListRenderer::UpdateSize(int cx, int cy)
{
	UNREFERENCED_PARAMETER(cx);
	UNREFERENCED_PARAMETER(cy);
}

void StandardCandidateListRenderer::SetIsEmpty(bool isEmpty)
{
	in->mIsEmpty = isEmpty;
}

int StandardCandidateListRenderer::GetItemCountInPage() const
{
	return in->mItemsInPage;
}

/**
  表示形式に応じた候補項目の高さを取得する
  @return 候補項目の高さ
*/
int StandardCandidateListRenderer::GetItemHeight() const
{
	int textHeight = (std::max)(1, in->mTextHeight);
	if (in->mIsTwoLine == false) {
		return textHeight + ITEM_MARGIN;
	}

	int linePitch = (std::max)(1, textHeight - TWO_LINE_OVERLAP);
	return textHeight + linePitch + ITEM_MARGIN;
}

void StandardCandidateListRenderer::PImpl::DrawItemIcon(
	CListCtrl* listWnd,
	CDC* pDC,
	int itemId
)
{
	if (mIsDrawIcon == false) {
		return;
	}

	CRect rcIcon;
	listWnd->GetSubItemRect(itemId, 0, LVIR_ICON, rcIcon);
	CRect rcItem;
	listWnd->GetItemRect(itemId, &rcItem, LVIR_BOUNDS);

	auto cmd = mCandidates->GetCommand(itemId);
	if (cmd == nullptr) {
		return;
	}

	int index = -1;
	HICON h = cmd->GetIcon();
	auto it = mIconIndexMap.find(h);
	if (it == mIconIndexMap.end()) {
		index = mIconList->Add(h);
		mIconIndexMap[h] = index;
	}
	else {
		index = it->second;
	}

	if (index != -1) {
		int iconTop = rcItem.top + (rcItem.Height() - mIconSize) / 2;
		mIconList->DrawEx(pDC, index, CPoint(rcIcon.left, iconTop), CSize(mIconSize, mIconSize),
		                  CLR_NONE, CLR_DEFAULT, ILD_NORMAL);
	}
}

void StandardCandidateListRenderer::PImpl::DrawItemName(
	CListCtrl* listWnd,
	CDC* pDC,
	int itemId
)
{
	CRect rcItem;
	listWnd->GetSubItemRect(itemId, 0, LVIR_LABEL, rcItem);
	auto cmd = mCandidates->GetCommand(itemId);

	CString name = cmd->GetName();
	name.Replace(_T("\r\n"), _T("\\n"));
	name.Replace(_T("\n"), _T("\\n"));
	name.Replace(_T("\t"), _T("  "));

	if (mIsTwoLine == false) {
		pDC->DrawText(name, rcItem, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP);
		return;
	}

	int lineHeight = (std::max)(1, mTextHeight);
	int linePitch = (std::max)(1, lineHeight - TWO_LINE_OVERLAP);
	int textBlockHeight = lineHeight + linePitch;
	int top = rcItem.top + (rcItem.Height() - textBlockHeight) / 2;
	CRect rcName(rcItem.left, top, rcItem.right, top + lineHeight);
	CRect rcDescription(rcItem.left, top + linePitch, rcItem.right, top + linePitch + lineHeight);
	if (mIsShowCommandType) {
		CRect rcTypeColumn;
		listWnd->GetSubItemRect(itemId, 1, LVIR_LABEL, rcTypeColumn);
		rcDescription.right = rcTypeColumn.right;
	}
	pDC->DrawText(name, rcName, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP);

	CString description = cmd->GetDescription();
	description.Replace(_T("\r\n"), _T(" "));
	description.Replace(_T("\r"), _T(" "));
	description.Replace(_T("\n"), _T(" "));
	description.Replace(_T("\t"), _T("  "));

	HFONT currentFont = static_cast<HFONT>(::GetCurrentObject(pDC->GetSafeHdc(), OBJ_FONT));
	if (mDescriptionFontSource != currentFont || mDescriptionFont.m_hObject == nullptr) {
		mDescriptionFont.DeleteObject();
		mDescriptionFontSource = nullptr;

		LOGFONT logFont{};
		if (currentFont != nullptr && ::GetObject(currentFont, sizeof(logFont), &logFont) > 0) {
			logFont.lfHeight = MulDiv(logFont.lfHeight, 85, 100);
			if (mDescriptionFont.CreateFontIndirect(&logFont)) {
				mDescriptionFontSource = currentFont;
			}
		}
	}

	if (mDescriptionFont.m_hObject != nullptr) {
		ScopedDCState dcState(pDC);
		pDC->SelectObject(&mDescriptionFont);
		pDC->DrawText(description, rcDescription,
		              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
	}
	else {
		pDC->DrawText(description, rcDescription,
		              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
	}
}

void StandardCandidateListRenderer::PImpl::DrawItemCategory(
	CListCtrl* listWnd,
	CDC* pDC,
	int itemId
)
{
	if (mIsShowCommandType == false) {
		return;
	}

	CRect rcItem;
	listWnd->GetSubItemRect(itemId, 1, LVIR_LABEL, rcItem);
	auto cmd = mCandidates->GetCommand(itemId);
	UINT format = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP;
	if (mIsTwoLine) {
		CRect rcBounds;
		listWnd->GetItemRect(itemId, &rcBounds, LVIR_BOUNDS);
		int lineHeight = (std::max)(1, mTextHeight);
		int linePitch = (std::max)(1, lineHeight - TWO_LINE_OVERLAP);
		int textBlockHeight = lineHeight + linePitch;
		rcItem.top = rcBounds.top + (rcBounds.Height() - textBlockHeight) / 2;
		rcItem.bottom = rcItem.top + lineHeight;
		format = DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP;
	}
	pDC->DrawText(cmd->GetTypeDisplayName(), rcItem, format);
}

void StandardCandidateListRenderer::DrawItem(CWnd* listWnd, LPDRAWITEMSTRUCT drawItemStruct)
{
	CListCtrl* candidateList = static_cast<CListCtrl*>(listWnd);
	CRect rcCtrl;
	candidateList->GetClientRect(&rcCtrl);

	CRect rcItem = drawItemStruct->rcItem;
	rcItem.right = rcCtrl.right;
	in->mItemsInPage = rcCtrl.Height() / rcItem.Height();

	int itemId = drawItemStruct->itemID;
	CDC* pDC = CDC::FromHandle(drawItemStruct->hDC);
	BOOL isSelect = (drawItemStruct->itemState & ODS_SELECTED);

	auto colorSettings = ColorSettings::Get();
	auto colorScheme = colorSettings->GetCurrentScheme();
	HBRUSH brBk1 = colorScheme->GetListBackgroundBrush();
	HBRUSH brBk2 = colorScheme->GetListBackgroundAltBrush();
	if (in->mIsAlternateColor == false) {
		brBk2 = brBk1;
	}

	if (in->mIsEmpty) {
		if (in->mIsDrawBackground == false) {
			return;
		}
		HBRUSH p = brBk2;
		while (rcItem.top < rcCtrl.Height()) {
			p = (p == brBk1) ? brBk2 : brBk1;
			pDC->FillRect(rcItem, CBrush::FromHandle(p));
			rcItem.OffsetRect(0, rcItem.Height());
		}
		return;
	}

	if (in->mIsDrawBackground) {
		HBRUSH hbr = (itemId % 2) ? brBk2 : brBk1;
		pDC->FillRect(rcItem, CBrush::FromHandle(hbr));
	}

	COLORREF crText = colorScheme->GetListTextColor();
	if (isSelect) {
		crText = colorScheme->GetListHighlightTextColor();
		CRect rcSelect;
		candidateList->GetItemRect(itemId, rcSelect, LVIR_BOUNDS);
		rcSelect.right = rcCtrl.right;
		pDC->FillRect(rcSelect, CBrush::FromHandle(colorScheme->GetListHighlightBackgroundBrush()));
	}

	int orgTextColor = pDC->SetTextColor(crText);
	in->DrawItemIcon(candidateList, pDC, itemId);
	in->DrawItemName(candidateList, pDC, itemId);
	in->DrawItemCategory(candidateList, pDC, itemId);

	if (in->mIsDrawBackground && itemId == in->mCandidates->GetSize() - 1) {
		rcItem.OffsetRect(0, rcItem.Height());
		HBRUSH p = (itemId % 2) ? brBk2 : brBk1;
		while (rcItem.top < rcCtrl.Height()) {
			p = (p == brBk1) ? brBk2 : brBk1;
			pDC->FillRect(rcItem, CBrush::FromHandle(p));
			rcItem.OffsetRect(0, rcItem.Height());
		}
	}

	pDC->SetTextColor(orgTextColor);
}
