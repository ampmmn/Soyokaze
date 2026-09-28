#include "pch.h"
#include "TwoLineCandidateListRenderer.h"
#include "CandidateList.h"
#include "utility/ScopedDCState.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

// 二行表示の描画領域を少し重ねて、項目の高さを抑える。
constexpr int TWO_LINE_OVERLAP = 2;

}

struct TwoLineCandidateListRenderer::PImpl
{
	CFont mDescriptionFont;
	HFONT mDescriptionFontSource{nullptr};

	/**
	  説明文を縮小フォントで描画する
	  @param[in] pDC 描画先デバイスコンテキスト
	  @param[in] description 説明文
	  @param[in] rect 描画領域
	*/
	void DrawDescription(CDC* pDC, const CString& description, CRect rect)
	{
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
			pDC->DrawText(description, rect,
			              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
		}
		else {
			pDC->DrawText(description, rect,
			              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
		}
	}
};

TwoLineCandidateListRenderer::TwoLineCandidateListRenderer() : in(new PImpl)
{
}

TwoLineCandidateListRenderer::~TwoLineCandidateListRenderer()
{
}

int TwoLineCandidateListRenderer::GetItemHeight() const
{
	int lineHeight = (std::max)(1, GetTextHeight());
	int linePitch = (std::max)(1, lineHeight - TWO_LINE_OVERLAP);
	return StandardCandidateListRenderer::GetItemHeight() + linePitch;
}

void TwoLineCandidateListRenderer::DrawItemName(CListCtrl* listWnd, CDC* pDC, int itemId)
{
	CandidateList* candidates = GetCandidateList();
	if (candidates == nullptr) {
		return;
	}
	auto cmd = candidates->GetCommand(itemId);
	if (cmd == nullptr) {
		return;
	}

	CRect rcItem;
	listWnd->GetSubItemRect(itemId, 0, LVIR_LABEL, rcItem);
	int lineHeight = (std::max)(1, GetTextHeight());
	int linePitch = (std::max)(1, lineHeight - TWO_LINE_OVERLAP);
	int textBlockHeight = lineHeight + linePitch;
	int top = rcItem.top + (rcItem.Height() - textBlockHeight) / 2;
	CRect rcName(rcItem.left, top, rcItem.right, top + lineHeight);
	CRect rcDescription(rcItem.left, top + linePitch, rcItem.right, top + linePitch + lineHeight);
	if (IsShowCommandType()) {
		CRect rcTypeColumn;
		listWnd->GetSubItemRect(itemId, 1, LVIR_LABEL, rcTypeColumn);
		rcDescription.right = rcTypeColumn.right;
	}

	CString name = cmd->GetName();
	name.Replace(_T("\r\n"), _T("\\n"));
	name.Replace(_T("\n"), _T("\\n"));
	name.Replace(_T("\t"), _T("  "));
	pDC->DrawText(name, rcName, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP);

	CString description = cmd->GetDescription();
	// 名前と説明が同じ場合は描画しない
	if (name != description) {
		description.Replace(_T("\r\n"), _T(" "));
		description.Replace(_T("\r"), _T(" "));
		description.Replace(_T("\n"), _T(" "));
		description.Replace(_T("\t"), _T("  "));
		in->DrawDescription(pDC, description, rcDescription);
	}
}

void TwoLineCandidateListRenderer::DrawItemCategory(CListCtrl* listWnd, CDC* pDC, int itemId)
{
	if (IsShowCommandType() == false) {
		return;
	}
	CandidateList* candidates = GetCandidateList();
	if (candidates == nullptr) {
		return;
	}
	auto cmd = candidates->GetCommand(itemId);
	if (cmd == nullptr) {
		return;
	}

	CRect rcItem;
	listWnd->GetSubItemRect(itemId, 1, LVIR_LABEL, rcItem);
	CRect rcBounds;
	listWnd->GetItemRect(itemId, &rcBounds, LVIR_BOUNDS);
	int lineHeight = (std::max)(1, GetTextHeight());
	int linePitch = (std::max)(1, lineHeight - TWO_LINE_OVERLAP);
	int textBlockHeight = lineHeight + linePitch;
	rcItem.top = rcBounds.top + (rcBounds.Height() - textBlockHeight) / 2;
	rcItem.bottom = rcItem.top + lineHeight;
	pDC->DrawText(cmd->GetTypeDisplayName(), rcItem,
	              DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP);
}
