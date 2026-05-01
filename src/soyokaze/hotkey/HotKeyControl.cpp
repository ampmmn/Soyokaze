#include "pch.h"
#include "HotKeyControl.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct HotKeyControl::PImpl
{
	UINT mNotifyId{0};
	CWnd* mDialog{nullptr};
	bool mIsCueSet{false};

};

HotKeyControl::HotKeyControl(CWnd* pParent) : in(new PImpl)
{
	UNREFERENCED_PARAMETER(pParent);
}

HotKeyControl::~HotKeyControl()
{
}

void HotKeyControl::SetNotifyId(UINT notifyId)
{
	in->mNotifyId = notifyId; 
}

void HotKeyControl::UpdateContent(CString text)
{
	if (in->mIsCueSet == false) {
		// 初回呼出し時にキューバナーを設定
		SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)(LPCTSTR)_T("未設定(クリックしてキーを設定)"));
		in->mIsCueSet = true;
	}
	SetWindowText(text);
}

bool HotKeyControl::EditHotKey(const CString& name, CommandHotKeyAttribute& attr, CWnd* parentWnd)
{
	CommandHotKeyDialog dlg(attr, parentWnd);

	in->mDialog = &dlg;

	dlg.SetTargetName(name);
	if (dlg.DoModal() != IDOK) {
		in->mDialog = nullptr;
		return false;
	}

	dlg.GetAttribute(attr);
	in->mDialog = nullptr;
	return true;
}

BEGIN_MESSAGE_MAP(HotKeyControl, CEdit)
	ON_WM_LBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_CHAR()
END_MESSAGE_MAP()

void HotKeyControl::OnSetFocus(CWnd* oldWindow)
{
	UNREFERENCED_PARAMETER(oldWindow);

	if (in->mNotifyId != 0) {

		if (oldWindow == nullptr || oldWindow == in->mDialog) {
			return;
		}
		GetParent()->PostMessage(in->mNotifyId, 0, 0);
	}
}

void HotKeyControl::OnLButtonDown(UINT flags, CPoint pt)
{
	UNREFERENCED_PARAMETER(flags);
	UNREFERENCED_PARAMETER(pt);
	if (in->mNotifyId != 0) {
		GetParent()->PostMessage(in->mNotifyId, 0, 0);
	}
}

void HotKeyControl::OnKeyDown(UINT,UINT,UINT)
{
}

void HotKeyControl::OnSysKeyDown(UINT,UINT,UINT)
{
}

void HotKeyControl::OnChar(UINT,UINT,UINT)
{
}

