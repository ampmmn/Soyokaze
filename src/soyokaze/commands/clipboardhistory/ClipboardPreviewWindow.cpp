#include "pch.h"
#include "ClipboardPreviewWindow.h"
#include "SharedHwnd.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace clipboardhistory {

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
		GetClientRect(&mInitialClientRect);
		CWnd* info = GetDlgItem(IDC_STATIC_INFO);
		info->GetWindowRect(&mInfoRect);
		ScreenToClient(&mInfoRect);
		CWnd* edit = GetDlgItem(IDC_EDIT_PREVIEW);
		edit->GetWindowRect(&mPreviewRect);
		ScreenToClient(&mPreviewRect);
		mIsInitialized = true;

		return TRUE;
	}

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT type, int cx, int cy)
	{
		__super::OnSize(type, cx, cy);
		if (mIsInitialized == false) {
			return;
		}

		int deltaWidth = cx - mInitialClientRect.Width();
		int deltaHeight = cy - mInitialClientRect.Height();
		CWnd* info = GetDlgItem(IDC_STATIC_INFO);
		CWnd* edit = GetDlgItem(IDC_EDIT_PREVIEW);
		info->SetWindowPos(nullptr, mInfoRect.left, mInfoRect.top, mInfoRect.Width() + deltaWidth,
		                   mInfoRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		edit->SetWindowPos(nullptr, mPreviewRect.left, mPreviewRect.top, mPreviewRect.Width() + deltaWidth,
		                   mPreviewRect.Height() + deltaHeight, SWP_NOZORDER | SWP_NOACTIVATE);
		UpdateScrollbar();
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
	CRect mInitialClientRect;
	CRect mInfoRect;
	CRect mPreviewRect;
	bool mIsInitialized{false};
};

BEGIN_MESSAGE_MAP(PreviewDialog, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct PreviewWindow::PImpl
{
	bool Create();

	PreviewDialog mWindow;
	bool mIsEnable{true};
	bool mIsDestroyed{false};
};

bool PreviewWindow::PImpl::Create()
{
	SharedHwnd mainWnd;
	HWND priorFocus = ::GetFocus();
	if (mWindow.Create(IDD_CLIPBOARD_PREVIEW, CWnd::FromHandle(mainWnd.GetHwnd())) == FALSE) {
		return false;
	}
	if (IsWindow(priorFocus)) {
		::SetFocus(priorFocus);
	}

	return true;
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

void PreviewWindow::Enter()
{
	if (in->mIsEnable == false || in->mIsDestroyed) {
		return;
	}

	if (in->mWindow.GetSafeHwnd() == nullptr) {
		if (in->Create() == false) {
			return;
		}
	}

	in->mWindow.ShowWindow(SW_SHOW);
}

void PreviewWindow::Leave()
{
	if (in->mWindow.GetSafeHwnd() == nullptr) {
		return;
	}
	in->mWindow.ShowWindow(SW_HIDE);
}

HWND PreviewWindow::GetHwnd()
{
	if (in->mIsDestroyed) {
		return nullptr;
	}
	if (in->mWindow.GetSafeHwnd() == nullptr && in->Create() == false) {
		return nullptr;
	}
	return in->mWindow.GetSafeHwnd();
}

void PreviewWindow::SetPreviewText(const CString& text, uint64_t date)
{
	CTime tm(FILETIME{date & 0xFFFFFFFF, (date >> 32)});
	auto s = tm.Format(_T("%c"));

	std::locale loc("en_US.UTF-8");
	auto sizeStr = std::format(loc, _T("{}"), text.GetLength());
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
	in->mIsDestroyed = true;
	if (in->mWindow.GetSafeHwnd()) {
		in->mWindow.DestroyWindow();
	}
}

void PreviewWindow::SetEnable(bool isEnable)
{
	in->mIsEnable = isEnable;
	if (isEnable == false) {
		Leave();
	}
}

void PreviewWindow::Disable()
{
	in->mIsEnable = false;
	Leave();
}

bool PreviewWindow::IsEnable() const
{
	return in->mIsEnable && in->mIsDestroyed == false;
}

}}} // end of namespace launcherapp::commands::clipboardhistory
