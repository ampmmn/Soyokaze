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
	int mItemsInPage{0};
	int mTextHeight{16};
	int mIconSize{16};
	std::unique_ptr<CImageList> mIconList;
	CImageList mIconListDummy;
	std::map<HICON, int> mIconIndexMap;
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

void StandardCandidateListRenderer::SetTextMetrics(int textHeight, int iconSize)
{
	in->mTextHeight = textHeight;
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
		mIconList->DrawEx(pDC, index, rcIcon.TopLeft(), CSize(mIconSize, mIconSize),
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

	pDC->DrawText(name, rcItem, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP);
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
	pDC->DrawText(cmd->GetTypeDisplayName(), rcItem, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP);
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
		HBRUSH p = brBk2;
		while (rcItem.top < rcCtrl.Height()) {
			p = (p == brBk1) ? brBk2 : brBk1;
			pDC->FillRect(rcItem, CBrush::FromHandle(p));
			rcItem.OffsetRect(0, rcItem.Height());
		}
		return;
	}

	HBRUSH hbr = (itemId % 2) ? brBk2 : brBk1;
	pDC->FillRect(rcItem, CBrush::FromHandle(hbr));

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

	if (itemId == in->mCandidates->GetSize() - 1) {
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
