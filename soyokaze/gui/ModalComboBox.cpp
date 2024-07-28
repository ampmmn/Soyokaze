#include "pch.h"
#include "ModalComboBox.h"
#include <afxcmn.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ModalComboBox::ModalComboBox()
{
}

ModalComboBox::~ModalComboBox()
{
}


BEGIN_MESSAGE_MAP(ModalComboBox, CComboBox)
END_MESSAGE_MAP()

BOOL
ModalComboBox::Create(
	CWnd* parentWndPtr,
	UINT ctrlID,
	DWORD style
)
{
	ASSERT(parentWndPtr && parentWndPtr->GetSafeHwnd());
	ASSERT(GetSafeHwnd() == NULL);

	if (ctrlID == 0xffff) {
		ctrlID = 0;
		for (UINT i = 1; i <= 0xffff; ++i) {
			if (parentWndPtr->GetDlgItem(i) == NULL) {
				ctrlID = i;
				break;
			}
		}
		if (ctrlID == 0) {
			return FALSE;
		}
	}

	// always invisible.
	style &= (~WS_VISIBLE);
	style |= WS_VSCROLL;

	return CComboBox::Create(style, CRect(0,0,1,1), parentWndPtr, ctrlID);
}

int ModalComboBox::DoModal(
	const CRect& rcItem,
	CString& text
)
{
	ASSERT(GetSafeHwnd());

	SetFont(GetParent()->GetFont());
	CWnd* oldFocus = SetFocus();

	int itemIndex = FindString(-1, text);
	if (itemIndex != CB_ERR) {
		SetCurSel(itemIndex);
	}

	SetWindowPos(NULL, rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height(), SWP_NOZORDER);
	// ドロップダウン時の実際の表示サイズはSetMinVisibleItemsメソッドで設定した項目数で決まる
	ShowWindow(SW_SHOW);

	ShowDropDown();

	bool isOK = DoPumpMessage(oldFocus);

	ReleaseCapture();
	ShowWindow(SW_HIDE);

	return isOK ? IDOK : IDCANCEL;
}

int ModalComboBox::DoModalOverListItem(
	CWnd* pParent, 
	int nItem, 
	int nSubItem
)
{

	HWND hWnd = pParent->GetSafeHwnd();
	TCHAR className[256 + 1];
	GetClassName(hWnd, className, 256 + 1);

	CString orgText;
	CRect rcItem;
	if (_tcscmp(className, _T("SysListView32")) == 0) {

		CListCtrl* p = (CListCtrl*)pParent;
		if (p->GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rcItem) == FALSE) {
			return IDCANCEL;
		}
		orgText = p->GetItemText(nItem, nSubItem);
		
	}
	else if (_tcscmp(className, _T("ListBox")) == 0) {
		CListBox* p = (CListBox*)pParent;
		if (p->GetItemRect(nItem, rcItem) == LB_ERR) {
			return IDCANCEL;
		}
		p->GetText(nItem, orgText);
	}
	else {
		// サポートしていない種別
		return IDCANCEL;
	}

	return DoModal(rcItem, orgText);
}

bool ModalComboBox::DoPumpMessage(CWnd* oldFocus)
{
	for(;;) {

		if (CWnd::GetFocus()->GetSafeHwnd() != GetSafeHwnd()) {
			// フォーカスを外れたらぬける
			return true;
		}

		MSG msg;
		if (PeekMessage(&msg, NULL, 0, 0, FALSE) == FALSE) {
			Sleep(0);
			continue;
		}

		BOOL IsMsgForMe = (msg.hwnd == GetSafeHwnd());

		TCHAR className[256+1];
		GetClassName(msg.hwnd, className, 256+1);
		if (msg.hwnd != GetSafeHwnd() && _tcscmp(className, _T("ComboLBox")) != 0)  {
			
			if (msg.message == WM_LBUTTONDOWN || msg.message == WM_NCLBUTTONDOWN) {
				if (oldFocus)
					oldFocus->SetFocus();

				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
				return true;
			}
		}

		GetMessage(&msg, NULL, 0, 0);
		if (IsMsgForMe)  {
			if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
				return false;
			}
			if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
				return true;
			}
		}
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}


