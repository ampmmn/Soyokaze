#pragma once

class ModalComboBox : public CComboBox
{
public:
	ModalComboBox();
	virtual ~ModalComboBox();

	BOOL Create(CWnd* pParent, UINT nID = 0xffff, DWORD nStyle = WS_CHILD | CBS_DROPDOWNLIST);
	int DoModal(const CRect& rectItem, CString& rstrText);
	int DoModalOverListItem(CWnd* pParent, int nItem, int nSubItem = 0);
protected:
	DECLARE_MESSAGE_MAP()
};


