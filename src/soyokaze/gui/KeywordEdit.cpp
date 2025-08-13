#include "pch.h"
#include "framework.h"
#include "KeywordEdit.h"
#include "mainwindow/controller/MainWindowController.h"
#include "gui/ColorSettings.h"
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
		CFont* currentFont = wnd->GetFont();
		LOGFONT lf;
		currentFont->GetLogFont(&lf);

		int fontSize = abs((int)lf.lfHeight);
		bool isFontSizeChanged = mFontSize != fontSize; 
		mFontSize = fontSize;

		if (mCaretIMEON == nullptr || isFontSizeChanged) {
			// 初回にキャレット用のビットマップを生成する

			int cx = GetSystemMetrics(SM_CXBORDER);
			int cy = fontSize;

			CClientDC dc(wnd);
			CDC memDC;
			memDC.CreateCompatibleDC(&dc);

			if (mCaretIMEON != nullptr) {
				DeleteObject(mCaretIMEON);
			}
			mCaretIMEON = CreateCompatibleBitmap(dc.GetSafeHdc(), cx + 1, cy);
				// IME=ONのキャレットは強調のため少しだけ太くする
			if (mCaretNormal != nullptr) {
				DeleteObject(mCaretNormal);
			}
			mCaretNormal = CreateCompatibleBitmap(dc.GetSafeHdc(), cx, cy);

			auto colorSettings = ColorSettings::Get();
			auto colorScheme = colorSettings->GetCurrentScheme();

			COLORREF crCaretIMEOn = colorScheme->GetCaretColor(true);
			COLORREF crCaretIMEOff = colorScheme->GetCaretColor(false);

			CBrush br(crCaretIMEOn);
			auto oldBr = memDC.SelectObject(&br);
			auto oldBmp = memDC.SelectObject(mCaretIMEON);
			memDC.PatBlt(0, 0, cx + 1, cy, PATCOPY);

			CBrush br2(crCaretIMEOff);
			memDC.SelectObject(&br2);
			memDC.SelectObject(mCaretNormal);
			memDC.PatBlt(0, 0, cx, cy, PATCOPY);

			memDC.SelectObject(oldBr);
			memDC.SelectObject(oldBmp);
		}

		return isIMEOn ? mCaretIMEON : mCaretNormal;
	}

	void AdjustLayout(KeywordEdit* thisWnd)
	{
		auto font = thisWnd->GetFont();
		CClientDC dc(thisWnd);
		auto orgFont = dc.SelectObject(font);

		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		int fontH = tm.tmHeight + tm.tmInternalLeading + tm.tmExternalLeading;
		dc.SelectObject(orgFont);

		CRect rc;
		thisWnd->GetClientRect(&rc);
		int rcOrgH = rc.Height();

		rc.top = (rcOrgH - fontH) / 2;
		rc.bottom = rc.top + fontH;
		rc.left = 2;

		thisWnd->SetRect(&rc);
	}

	bool mIsFocus{false};
	bool mIsNotify{true};

	// IMMのハンドル
	HIMC mImcHandle{nullptr};
	// Note: IMEの状態に応じてカーソルの色を変えるために使用する

	// キャレット用のビットマップ
	HBITMAP mCaretNormal{nullptr};
	HBITMAP mCaretIMEON{nullptr};
	int mFontSize{16};

	CString mPlaceHolderText;

};

KeywordEdit::KeywordEdit(CWnd* pParent) : in(std::make_unique<PImpl>())
{
	UNREFERENCED_PARAMETER(pParent);
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
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_DESTROY()
	ON_WM_CHAR()
	ON_WM_SIZE()
	ON_WM_PASTE()
	ON_WM_CONTEXTMENU()
	ON_WM_MBUTTONUP()
END_MESSAGE_MAP()


void KeywordEdit::Paste()
{
	PostMessage(WM_PASTE, 0, 0);
}

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
	else if (msg == WM_SETFONT) {
		LRESULT ret = __super::WindowProc(msg, wp, lp);
		in->AdjustLayout(this);
		return ret;
	}

	return __super::WindowProc(msg, wp, lp);
}


void KeywordEdit::SetCaretToEnd()
{
	CString s;
	GetWindowText(s);
	int n = s.GetLength();
	SetSel(n, n, FALSE);
}

void KeywordEdit::SetIMEOff()
{
	ImmSetOpenStatus(in->GetImmContext(this), FALSE);
}

// プレースホルダーの文字列を設定する
void KeywordEdit::SetPlaceHolder(const CString& text)
{
	in->mPlaceHolderText = text;
	SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)(LPCTSTR)text);
}

void KeywordEdit::SetNotifyKeyEvent(bool isNotify)
{
	in->mIsNotify = isNotify;
}

void KeywordEdit::OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags)
{
	if (in->mIsNotify) {
		// 親ウインドウ(LauncherMainWindow)にキー入力を通知
		if (GetParent()->SendMessage(WM_APP + 1, nChar, 0) != 0) {
			return ;
		}
	}
	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}

UINT KeywordEdit::OnGetDlgCode()
{
	UINT ret = __super::OnGetDlgCode();

	if (in->mIsNotify) {
		const MSG* msg{CWnd::GetCurrentMessage()};
		msg = (const MSG*)msg->lParam;

		// Tabキーの入力もWM_KEYDOWNで処理できるようにする
		if (msg && msg->message == WM_KEYDOWN && msg->wParam == VK_TAB) {
			ret |= DLGC_WANTMESSAGE;
		}
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

void KeywordEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_TAB) {
		// Tabキーを押したときにBeep音が鳴るのを防ぐ目的
		return;
	}
	__super::OnChar(nChar, nRepCnt, nFlags);
}

void KeywordEdit::OnSize(UINT type, int cx, int cy)
{
	__super::OnSize(type, cx, cy);
	in->AdjustLayout(this);
}

// ペースト時の動作
void KeywordEdit::OnPaste()
{
	// 複数行のテキストが入力欄にペーストされたときに、先頭行のテキストだけを取得するようにする

	// クリップボードのテキストを取得
	CString text;
	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	mainWnd->GetClipboardString(text);

	// 先頭行のみにする
	int pos = text.FindOneOf(_T("\r\n"));
	if (pos != -1) {
		text = text.Left(pos);
	}
	SendMessage(EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)(LPCTSTR)text);
}

void KeywordEdit::OnPaint()
{
	// ES_MULTILINEの場合、EM_SETCUEBANNERによる描画が効かないため、自分で描画する
	LONG style = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
	if ((style & ES_MULTILINE) == 0) {
		__super::OnPaint();
		return;
	}

	CString s;
	GetWindowText(s);
	if (s.IsEmpty()) {
		// 描画領域を取得する
		CRect rc;
		GetRect(rc);

		// デバイスコンテキストを準備する
		CPaintDC dc(this);
		auto org = dc.SelectObject(GetFont());
		int bk = dc.SetBkMode(TRANSPARENT);
		COLORREF cr = dc.SetTextColor(RGB(128,128,128));

		// テキスト描画
		dc.DrawText(in->mPlaceHolderText, rc, DT_LEFT);

		// デバイスコンテキストの状態を元に戻す
		dc.SetTextColor(cr);
		dc.SetBkMode(bk);
		dc.SelectObject(org);
	}
	__super::OnPaint();
}

void KeywordEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
	auto parent = GetParent();
	if (parent) {
		WPARAM wp = (WPARAM)pWnd->GetSafeHwnd();
		LPARAM lp = MAKELPARAM(point.x, point.y);
		parent->SendMessage(WM_CONTEXTMENU, wp, lp);
	}
}

void KeywordEdit::OnMButtonUp(UINT flags, CPoint point)
{
	__super::OnMButtonUp(flags, point);
	Paste();
}

