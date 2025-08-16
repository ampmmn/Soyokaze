#include "pch.h"
#include "ClipboardPreviewWindow.h"
#include "SharedHwnd.h"
#include "gui/WindowPosition.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace clipboardhistory {

constexpr UINT TIMERID_HIDE = 1;

class PreviewDialog : public CDialog
{
public:
	PreviewDialog()
	{
	}
	~PreviewDialog()
	{
	}

	void DoDataExchange(CDataExchange* pDX) override
	{
		__super::DoDataExchange(pDX);
		DDX_Text(pDX, IDC_STATIC_INFO, mInfoText);
		DDX_Text(pDX, IDC_EDIT_PREVIEW, mPreviewText);
	}
	BOOL OnInitDialog() override
	{
		__super::OnInitDialog();

		LONG_PTR styleEx = WS_EX_NOACTIVATE;
		SetWindowLongPtr(GetSafeHwnd(), GWL_EXSTYLE, styleEx);

		return TRUE;
	}

	DECLARE_MESSAGE_MAP()
	// クライアント領域をドラッグしてウインドウを移動させるための処理
	afx_msg LRESULT OnNcHitTest(CPoint point)
	{
		RECT rect;
		GetClientRect(&rect);

		CPoint ptClient(point);
		ScreenToClient(&ptClient);

		if (PtInRect(&rect, ptClient) && (GetAsyncKeyState( VK_LBUTTON ) & 0x8000) )
		{
			return HTCAPTION;
		}
		return __super::OnNcHitTest(point);
	}

	afx_msg void OnTimer(UINT_PTR timerId)
	{
		KillTimer(timerId);

		// 非表示にする
		ShowWindow(SW_HIDE);
	}



public:
	void UpdateScrollbar()
	{
		CWnd* edit = GetDlgItem(IDC_EDIT_PREVIEW);

		CRect rc;
		edit->GetClientRect(&rc);
		int clientHeight = rc.Height();

		int lineCount = (int)edit->SendMessage(EM_GETLINECOUNT, 0, 0);

		CClientDC dc(edit);
		CFont* pFont = edit->GetFont();
		CFont* pOldFont = dc.SelectObject(pFont);

		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);

		dc.SelectObject(pOldFont);

		int lineHeight = tm.tmHeight;

		int totalTextHeight = lineCount * lineHeight;

		auto hEdit = edit->GetSafeHwnd();

		LONG style = ::GetWindowLong(hEdit, GWL_STYLE);

		bool needsScroll = totalTextHeight > clientHeight;
		if (needsScroll && !(style & WS_VSCROLL)) {
			// スクロールバーを表示する
			::SetWindowLong(hEdit, GWL_STYLE, style | WS_VSCROLL);
			::SetWindowPos(hEdit, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		} else if (!needsScroll && (style & WS_VSCROLL)) {
			// スクロールバーを非表示にする
			::SetWindowLong(hEdit, GWL_STYLE, style & ~WS_VSCROLL);
			::SetWindowPos(hEdit, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
	}

public:
	// 日付、サイズなど
	CString mInfoText;
	// クリップボードのテキスト
	CString mPreviewText;
};

BEGIN_MESSAGE_MAP(PreviewDialog, CDialog)
	ON_WM_NCHITTEST()
	ON_WM_TIMER()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct PreviewWindow::PImpl
{
	bool Create();

	static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

	PreviewDialog mWindow;
	std::unique_ptr<WindowPosition> mWindowPos;
	bool mIsEnable{true};
};

bool PreviewWindow::PImpl::Create()
{
	SharedHwnd mainWnd;
	if (mWindow.Create(IDD_CLIPBOARD_PREVIEW, CWnd::FromHandle(mainWnd.GetHwnd())) == FALSE) {
		return false;
	}

	// 位置情報を復元する(ある場合は)
	mWindowPos.reset(new WindowPosition(_T("ClipboardPreview")));
	if (mWindowPos->Restore(mWindow.GetSafeHwnd()) == false) {
		// 初回は復元できないので、メインウインドウの右側に表示する
		CRect rc;
		GetWindowRect(mainWnd.GetHwnd(), &rc);
		SetWindowPos(mWindow.GetSafeHwnd(), nullptr, rc.right, rc.top, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	}

	return true;
}

LRESULT CALLBACK PreviewWindow::PImpl::OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	return DefWindowProc(hwnd, msg, wp, lp);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

PreviewWindow::PreviewWindow() : in(new PImpl)
{
}

PreviewWindow::~PreviewWindow()
{
}

PreviewWindow* PreviewWindow::Get()
{
	static PreviewWindow inst;
	return &inst;
}

bool PreviewWindow::Show()
{
	if (in->mIsEnable == false)  {
		return false;
	}

	if (in->mWindow.GetSafeHwnd() == nullptr) {
		if (in->Create() == false) {
			return false;
		}
	}

	in->mWindow.KillTimer(TIMERID_HIDE);
	in->mWindow.ShowWindow(SW_SHOWNOACTIVATE);

	return true;
}

bool PreviewWindow::Hide()
{
	if (in->mWindow.GetSafeHwnd() == nullptr) {
		return true;
	}

	// 位置情報を更新
	in->mWindowPos->Update(in->mWindow.GetSafeHwnd());

	// ここでウインドウを直接非表示にすると、コマンドの絞り込み時に画面がちらつくので
	// Timer経由でワンテンポ遅らせる。すぐに次の表示がきたら、タイマーをキャンセルする
	in->mWindow.SetTimer(TIMERID_HIDE, 250, 0);

	return true;
}

void PreviewWindow::SetPreviewText(const CString& text, uint64_t date)
{
	CTime tm(FILETIME{date & 0xFFFFFFFF, (date >> 32)});
	auto s = tm.Format(_T("%c"));

	std::locale loc("en_US.UTF-8");
	auto sizeStr = fmt::format(loc, _T("{:L}"), text.GetLength());
	in->mWindow.mInfoText.Format(_T("%s %s byte"), (LPCTSTR)s, sizeStr.c_str());
	in->mWindow.mPreviewText = text;

	// タブサイズ8は大きいので、4にみせかける
	in->mWindow.mPreviewText.Replace(_T("\t"), _T("    "));

	if (in->mWindow.GetSafeHwnd()) {
		in->mWindow.UpdateData(FALSE);
		in->mWindow.UpdateScrollbar();
		in->mWindow.UpdateWindow();
	}
}

// ウインドウを破棄する
void PreviewWindow::Destroy()
{
	if (in->mWindow.GetSafeHwnd()) {
		// 次回のためにウインドウ位置情報を記憶する
		in->mWindowPos->Save();
	}
}

void PreviewWindow::SetEnable(bool isEnable)
{
	in->mIsEnable = isEnable;
}

void PreviewWindow::Disable()
{
	in->mIsEnable = false;
}

}}} // end of namespace launcherapp::commands::clipboardhistory
