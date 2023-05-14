#include "pch.h"
#include "framework.h"
#include "KeywordManagerListCtrl.h"


KeywordManagerListCtrl::KeywordManagerListCtrl()
{
}

KeywordManagerListCtrl::~KeywordManagerListCtrl()
{
}

BEGIN_MESSAGE_MAP(KeywordManagerListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnLvnItemChange)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
END_MESSAGE_MAP()

void KeywordManagerListCtrl::OnLvnItemChange(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	 NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;
	 GetParent()->SendMessage(WM_APP+1, 0, nm->iItem);
	 *pResult = 0;
}


void KeywordManagerListCtrl::OnNMDblclk(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	 NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;
	 GetParent()->SendMessage(WM_APP+2, 0, nm->iItem);
	 *pResult = 0;
}

CString KeywordManagerListCtrl::GetSelectedCommandName()
{
	POSITION pos = GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return _T("");
	}

	int itemIndex = GetNextSelectedItem(pos);
	return GetItemText(itemIndex, 0);
}
