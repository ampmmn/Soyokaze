#include "pch.h"
#include "framework.h"
#include "KeywordEdit.h"
#include <imm.h>

#pragma comment(lib, "imm32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


struct KeywordEdit::PImpl
{
	HIMC GetImmContext(CWnd* wnd)
	{
		if (mImcHandle) {
			return mImcHandle;
		}

		HWND h = wnd->GetSafeHwnd();
		mImcHandle = ImmGetContext(h);
		return mImcHandle;
	}

	bool IsIMEOn(CWnd* wnd) {
		return ImmGetOpenStatus(GetImmContext(wnd)) != FALSE;
	}

	HBITMAP GetCurrentCaretBitmap(CWnd* wnd)
	{
		return GetCaret(wnd, IsIMEOn(wnd));
	}

	HBITMAP GetCaret(CWnd* wnd, bool isIMEOn)
	{
		if (mIsFirst) {
			CFont* currentFont = wnd->GetFont();
			LOGFONT lf;
			currentFont->GetLogFont(&lf);

			int cx = GetSystemMetrics(SM_CXBORDER);
			int cy = abs((int)lf.lfHeight);

			CClientDC dc(wnd);
			CDC memDC;
			memDC.CreateCompatibleDC(&dc);

			mCaretIMEON = CreateCompatibleBitmap(dc.GetSafeHdc(), cx + 1, cy);
				// IME=ONのキャレットは強調のため少しだ太くする
			mCaretNormal = CreateCompatibleBitmap(dc.GetSafeHdc(), cx, cy);

			CBrush br(RGB(255-160, 255-32, 255-240));
			auto oldBr = memDC.SelectObject(&br);
			auto oldBmp = memDC.SelectObject(mCaretIMEON);
			memDC.PatBlt(0, 0, cx + 1, cy, PATCOPY);

			CBrush br2(RGB(255, 255, 255));
			memDC.SelectObject(&br2);
			memDC.SelectObject(mCaretNormal);
			memDC.PatBlt(0, 0, cx, cy, PATCOPY);

			memDC.SelectObject(oldBr);
			memDC.SelectObject(oldBmp);

			mIsFirst = false;
		}

		return isIMEOn ? mCaretIMEON : mCaretNormal;
	}


	bool mIsFirst;
	bool mIsFocus;

	HIMC mImcHandle;

	HBITMAP mCaretNormal;
	HBITMAP mCaretIMEON;

};

KeywordEdit::KeywordEdit(CWnd* pParent) : in(std::make_unique<PImpl>())
{
	in->mIsFirst = true;
	in->mIsFocus = false;
	in->mCaretNormal = NULL;
	in->mCaretIMEON = NULL;
	in->mImcHandle = NULL;

}

KeywordEdit::~KeywordEdit()
{
	if (in->mCaretNormal) {
		DeleteObject(in->mCaretNormal);
		in->mCaretNormal = NULL;
	}
	if (in->mCaretIMEON) {
		DeleteObject(in->mCaretIMEON);
		in->mCaretIMEON = NULL;
	}

}

BEGIN_MESSAGE_MAP(KeywordEdit, CEdit)
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

LRESULT KeywordEdit::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_IME_NOTIFY && wp == IMN_SETOPENSTATUS) {

		if (in->mIsFocus) {
			HideCaret();
			DestroyCaret();

			HBITMAP h = in->GetCurrentCaretBitmap(this);
			::CreateCaret(GetSafeHwnd(), h, 0, 0);
			ShowCaret();
		}
	}

	return __super::WindowProc(msg, wp, lp);
}


void KeywordEdit::SetCaretToEnd()
{
	int n = GetLimitText();
	SetSel(n, n, FALSE);
}

void KeywordEdit::SetIMEOff()
{
		ImmSetOpenStatus(in->GetImmContext(this), FALSE);
}

void KeywordEdit::OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags)
{
	// 親ウインドウ(LauncherMainWindow)にキー入力を通知
	if (GetParent()->SendMessage(WM_APP + 1, nChar, 0) != 0) {
		return ;
	}
	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}

UINT KeywordEdit::OnGetDlgCode()
{
	UINT ret = __super::OnGetDlgCode();

	const MSG* msg = CWnd::GetCurrentMessage();
	msg = (const MSG*)msg->lParam;

	// Tabキーの入力もWM_KEYDOWNで処理できるようにする
	if (msg && msg->message == WM_KEYDOWN && msg->wParam == VK_TAB) {
		ret |= DLGC_WANTMESSAGE;
	}

	return ret;
}

void KeywordEdit::OnSetFocus(CWnd* oldWindow)
{
	__super::OnSetFocus(oldWindow);
	::CreateCaret(GetSafeHwnd(), in->GetCurrentCaretBitmap(this), 0, 0);
	ShowCaret();
	in->mIsFocus = true;
}

void KeywordEdit::OnKillFocus(CWnd* newWindow)
{
	in->mIsFocus = false;
	HideCaret();
	DestroyCaret();
	__super::OnKillFocus(newWindow);
}


void KeywordEdit::OnDestroy()
{
	ImmReleaseContext(GetSafeHwnd(), in->mImcHandle);
	in->mImcHandle = NULL;

	__super::OnDestroy();
}

