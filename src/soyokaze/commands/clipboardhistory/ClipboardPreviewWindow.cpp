#include "pch.h"
#include "ClipboardPreviewWindow.h"
#include "SharedHwnd.h"
#include "commands/clipboardhistory/ClipboardPreviewHighlighter.h"
#include "commands/py_extension/ScintillaDLLLoader.h"
#include "setting/AppPreference.h"
#include "resource.h"
#include "Scintilla.h"

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
	}



public:
	void UpdateStyle()
	{
		CWnd* edit = GetDlgItem(IDC_EDIT_PREVIEW);
		edit->SendMessage(SCI_SETCODEPAGE, SC_CP_UTF8, 0);
		edit->SendMessage(SCI_SETWRAPMODE, SC_WRAP_CHAR, 0);
		edit->SendMessage(SCI_SETHSCROLLBAR, FALSE, 0);
		edit->SendMessage(SCI_SETVSCROLLBAR, TRUE, 0);

		auto pref = AppPreference::Get();
		CString fontName;
		pref->GetMainWindowFontName(fontName);
		std::string fontNameUtf8;
		UTF2UTF(std::wstring(fontName), fontNameUtf8);
		edit->SendMessage(SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)fontNameUtf8.c_str());
		edit->SendMessage(SCI_STYLESETSIZE, STYLE_DEFAULT, pref->GetMainWindowFontSize());
		edit->SendMessage(SCI_STYLESETFORE, STYLE_DEFAULT, RGB(0, 0, 0));
		edit->SendMessage(SCI_STYLESETBACK, STYLE_DEFAULT, GetSysColor(COLOR_WINDOW));
		edit->SendMessage(SCI_STYLECLEARALL, 0, 0);

		edit->SendMessage(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
		edit->SendMessage(SCI_STYLESETFORE, STYLE_LINENUMBER, GetSysColor(COLOR_GRAYTEXT));
		edit->SendMessage(SCI_STYLESETBACK, STYLE_LINENUMBER, GetSysColor(COLOR_BTNFACE));
		edit->SendMessage(SCI_SETMARGINWIDTHN, 0, 40);

		edit->SendMessage(SCI_INDICSETSTYLE, INDICATOR_CONTAINER, INDIC_ROUNDBOX);
		edit->SendMessage(SCI_INDICSETFORE, INDICATOR_CONTAINER, RGB(255, 196, 0));
		edit->SendMessage(SCI_INDICSETALPHA, INDICATOR_CONTAINER, 120);
		edit->SendMessage(SCI_SETREADONLY, TRUE, 0);
	}

	void UpdateLineNumberMargin()
	{
		CWnd* edit = GetDlgItem(IDC_EDIT_PREVIEW);
		auto lineCount = static_cast<int>(edit->SendMessage(SCI_GETLINECOUNT, 0, 0));
		auto lineCountText = std::to_string((std::max)(lineCount, 1));
		std::string lineNumber(lineCountText.length(), '9');
		auto textWidth = static_cast<int>(edit->SendMessage(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)lineNumber.c_str()));
		edit->SendMessage(SCI_SETMARGINWIDTHN, 0, textWidth + 10);
	}

	void SetText(const CString& text, const std::vector<CString>& keywords)
	{
		CWnd* edit = GetDlgItem(IDC_EDIT_PREVIEW);
		CString previewText(text);
		previewText.Replace(_T("\t"), _T("    "));

		std::string textUtf8;
		UTF2UTF(std::wstring(previewText), textUtf8);
		std::vector<std::string> keywordsUtf8;
		keywordsUtf8.reserve(keywords.size());
		for (const auto& keyword : keywords) {
			std::string keywordUtf8;
			UTF2UTF(std::wstring(keyword), keywordUtf8);
			keywordsUtf8.push_back(std::move(keywordUtf8));
		}

		edit->SendMessage(SCI_SETREADONLY, FALSE, 0);
		edit->SendMessage(SCI_SETTEXT, 0, (LPARAM)textUtf8.c_str());
		edit->SendMessage(SCI_SETINDICATORCURRENT, INDICATOR_CONTAINER, 0);
		edit->SendMessage(SCI_INDICATORCLEARRANGE, 0, edit->SendMessage(SCI_GETLENGTH, 0, 0));
		auto ranges = FindClipboardPreviewMatchRanges(textUtf8, keywordsUtf8);
		for (const auto& range : ranges) {
			edit->SendMessage(SCI_INDICATORFILLRANGE, range.mStart, range.mLength);
		}
		edit->SendMessage(SCI_GOTOPOS, 0, 0);
		edit->SendMessage(SCI_SETREADONLY, TRUE, 0);
		UpdateLineNumberMargin();
	}

public:
	// 日付、サイズなど
	CString mInfoText;
	// クリップボードのプレビュー内容
	CString mPreviewText;
	std::vector<CString> mKeywords;
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
	if (launcherapp::commands::py_extension::ScintillaDLLLoader::GetInstance()->Initialize() == false) {
		return false;
	}

	SharedHwnd mainWnd;
	HWND priorFocus = ::GetFocus();
	if (mWindow.Create(IDD_CLIPBOARD_PREVIEW, CWnd::FromHandle(mainWnd.GetHwnd())) == FALSE) {
		return false;
	}
	mWindow.UpdateStyle();
	mWindow.SetText(mWindow.mPreviewText, mWindow.mKeywords);
	mWindow.UpdateData(FALSE);
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

void PreviewWindow::SetPreviewText(const CString& text, uint64_t date, const std::vector<CString>& keywords)
{
	CTime tm(FILETIME{date & 0xFFFFFFFF, (date >> 32)});
	auto s = tm.Format(_T("%c"));

	std::locale loc("en_US.UTF-8");
	auto sizeStr = std::format(loc, _T("{}"), text.GetLength());
	in->mWindow.mInfoText.Format(_T("%s %s byte"), (LPCTSTR)s, sizeStr.c_str());
	in->mWindow.mPreviewText = text;
	in->mWindow.mKeywords = keywords;
	if (in->mWindow.GetSafeHwnd()) {
		in->mWindow.UpdateStyle();
		in->mWindow.SetText(in->mWindow.mPreviewText, in->mWindow.mKeywords);
		in->mWindow.UpdateData(FALSE);
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
