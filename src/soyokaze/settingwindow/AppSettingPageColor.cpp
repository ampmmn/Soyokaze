// あ
#include "pch.h"
#include "framework.h"
#include "AppSettingPageColor.h"
#include "setting/Settings.h"
#include "setting/AppPreference.h"
#include "utility/Accessibility.h"
#include "gui/ColorSettings.h"
#include "gui/SystemColorScheme.h"
#include "gui/ImageLabel.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class ColorSettingDialog : public CDialog
{
public:
	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	COLORREF GetWindowTextColor();
	COLORREF GetWindowBkColor();
	COLORREF GetEditTextColor();
	COLORREF GetEditBkColor();
	COLORREF GetListTextColor();
	COLORREF GetListBkColor();
	COLORREF GetListBkAltColor();
	COLORREF GetListTextHLColor();
	COLORREF GetListBkHLColor();

	void DisableAllControls();
	bool UpdateStatus();
	void ApplyToCtrl();
	void DrawPreview();

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	void OnButtonRestore();
	void OnUpdateStatus();

	BOOL mIsUseSystemSettings{TRUE};

	COLORREF mWindowTextColor{0};
	COLORREF mWindowBkColor{0};
	COLORREF mEditTextColor{0};
	COLORREF mEditBkColor{0};
	COLORREF mListTextColor{0};
	COLORREF mListBkColor{0};
	COLORREF mListBkAltColor{0};
	COLORREF mListTextHLColor{0};
	COLORREF mListBkHLColor{0};

	ImageLabel mPreview;

	SystemColorScheme mSysScheme;

	Settings* mSettingsPtr{nullptr};
};

COLORREF ColorSettingDialog::GetWindowTextColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetTextColor() : mWindowTextColor;
}

COLORREF ColorSettingDialog::GetWindowBkColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetBackgroundColor() : mWindowBkColor;
}

COLORREF ColorSettingDialog::GetEditTextColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetEditTextColor() : mEditTextColor;
}

COLORREF ColorSettingDialog::GetEditBkColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetEditBackgroundColor() : mEditBkColor;
}

COLORREF ColorSettingDialog::GetListTextColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListTextColor() : mListTextColor;
}

COLORREF ColorSettingDialog::GetListBkColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListBackgroundColor() : mListBkColor;
}

COLORREF ColorSettingDialog::GetListBkAltColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListBackgroundAltColor() : mListBkAltColor;
}

COLORREF ColorSettingDialog::GetListTextHLColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListHighlightTextColor() : mListTextHLColor;
}

COLORREF ColorSettingDialog::GetListBkHLColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListHighlightBackgroundColor() : mListBkHLColor;
}

bool ColorSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool ColorSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void ColorSettingDialog::OnOK()
{
	auto settingsPtr = mSettingsPtr;

	UpdateData();

	mWindowTextColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWTEXT))->GetColor();
	mWindowBkColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWBACKGROUND))->GetColor();
	mEditTextColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITTEXT))->GetColor();
	mEditBkColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITBACKGROUND))->GetColor();
	mListTextColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXT))->GetColor();
	mListBkColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUND))->GetColor();
	mListBkAltColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDALT))->GetColor();
	mListTextHLColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXTHIGHLIGHT))->GetColor();
	mListBkHLColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDHIGHLIGHT))->GetColor();

	settingsPtr->Set(_T("ColorSettings:UseSystemColor"), mIsUseSystemSettings ? true : false);
	settingsPtr->Set(_T("CurrentColor:WindowText"), (int)mWindowTextColor);
	settingsPtr->Set(_T("CurrentColor:WindowBackground"), (int)mWindowBkColor);
	settingsPtr->Set(_T("CurrentColor:EditText"), (int)mEditTextColor);
	settingsPtr->Set(_T("CurrentColor:EditBackground"), (int)mEditBkColor);
	settingsPtr->Set(_T("CurrentColor:ListText"), (int)mListTextColor);
	settingsPtr->Set(_T("CurrentColor:ListBackground"), (int)mListBkColor);
	settingsPtr->Set(_T("CurrentColor:ListBackgroundAlt"), (int)mListBkAltColor);
	settingsPtr->Set(_T("CurrentColor:ListHighlightText"), (int)mListTextHLColor);
	settingsPtr->Set(_T("CurrentColor:ListHighlightBackground"), (int)mListBkHLColor);

	__super::OnOK();
}

void ColorSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_USEDEFAULT, mIsUseSystemSettings);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(ColorSettingDialog, CDialog)
	ON_COMMAND(IDC_BUTTON_RESTORE, OnButtonRestore)
	ON_COMMAND(IDC_CHECK_USEDEFAULT, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_WINDOWTEXT, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_WINDOWBACKGROUND, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_EDITTEXT, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_EDITBACKGROUND, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_LISTTEXT, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_LISTBACKGROUND, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_LISTBACKGROUNDALT, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_LISTTEXTHIGHLIGHT, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_LISTBACKGROUNDHIGHLIGHT, OnUpdateStatus)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL ColorSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	mPreview.SubclassDlgItem(IDC_STATIC_PREVIEW, this);

	if (::utility::IsHighContrastMode()) {
		DisableAllControls();
	}

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

void ColorSettingDialog::DisableAllControls()
{
	::ShowWindow(GetDlgItem(IDC_STATIC_WARNING)->GetSafeHwnd(), SW_SHOW);

	std::array<UINT, 11> ids{
		IDC_CHECK_USEDEFAULT,
		IDC_BUTTON_WINDOWTEXT,
		IDC_BUTTON_WINDOWBACKGROUND,
		IDC_BUTTON_EDITTEXT,
		IDC_BUTTON_EDITBACKGROUND,
		IDC_BUTTON_LISTTEXT,
		IDC_BUTTON_LISTBACKGROUND,
		IDC_BUTTON_LISTBACKGROUNDALT,
		IDC_BUTTON_LISTTEXTHIGHLIGHT,
		IDC_BUTTON_LISTBACKGROUNDHIGHLIGHT,
		IDC_STATIC_PREVIEW,
	};

	for (auto id : ids) {
		GetDlgItem(id)->EnableWindow(FALSE);
	}
}

bool ColorSettingDialog::UpdateStatus()
{
	BOOL isEnableColor = mIsUseSystemSettings == FALSE;
	spdlog::debug("enablecolor {}", isEnableColor != FALSE);

	// システム設定を使わない場合はカラー設定できるようにする
	GetDlgItem(IDC_BUTTON_RESTORE)->EnableWindow(isEnableColor);
	GetDlgItem(IDC_BUTTON_WINDOWTEXT)->EnableWindow(isEnableColor);
	GetDlgItem(IDC_BUTTON_WINDOWBACKGROUND)->EnableWindow(isEnableColor);
	GetDlgItem(IDC_BUTTON_EDITTEXT)->EnableWindow(isEnableColor);
	GetDlgItem(IDC_BUTTON_EDITBACKGROUND)->EnableWindow(isEnableColor);
	GetDlgItem(IDC_BUTTON_LISTTEXT)->EnableWindow(isEnableColor);
	GetDlgItem(IDC_BUTTON_LISTBACKGROUND)->EnableWindow(isEnableColor);
	GetDlgItem(IDC_BUTTON_LISTBACKGROUNDALT)->EnableWindow(isEnableColor);
	GetDlgItem(IDC_BUTTON_LISTTEXTHIGHLIGHT)->EnableWindow(isEnableColor);
	GetDlgItem(IDC_BUTTON_LISTBACKGROUNDHIGHLIGHT)->EnableWindow(isEnableColor);

	mWindowTextColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWTEXT))->GetColor();
	mWindowBkColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWBACKGROUND))->GetColor();
	mEditTextColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITTEXT))->GetColor();
	mEditBkColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITBACKGROUND))->GetColor();
	mListTextColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXT))->GetColor();
	mListBkColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUND))->GetColor();
	mListBkAltColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDALT))->GetColor();
	mListTextHLColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXTHIGHLIGHT))->GetColor();
	mListBkHLColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDHIGHLIGHT))->GetColor();

	// プレビューを更新
	DrawPreview();
	return true;
}

void ColorSettingDialog::DrawPreview()
{
	auto pref = AppPreference::Get();

	CFont font;
	LOGFONT lf;
	GetFont()->GetLogFont(&lf);

	CString fontName;
	if (pref->GetMainWindowFontName(fontName)) {
		_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, fontName, _TRUNCATE);
	}
	font.CreateFontIndirectW(&lf);

	CBitmap* bmp = mPreview.GetBitmap();

	CRect rc;
	mPreview.GetClientRect(&rc);

	CBrush brBk;
	brBk.CreateSolidBrush(GetWindowBkColor());

	CClientDC dc(this);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);


	auto orgBmp = dcMem.SelectObject(bmp);
	auto orgBr = dcMem.SelectObject(&brBk);
	auto orgFont = dcMem.SelectObject(&font);

	int itemHeight = 24;

	int offset = 0;
	dcMem.PatBlt(0, offset, rc.Width(), itemHeight, PATCOPY);

	// コメント欄のテキスト描画
	CRect rcItem(CPoint(32, 8), CSize(rc.Width(), itemHeight));
	dcMem.SetBkColor(GetWindowBkColor());
	dcMem.SetTextColor(GetWindowTextColor());
	dcMem.DrawText(pref->GetDefaultComment(), rcItem, DT_LEFT);
;
	offset += itemHeight;

	// テキスト領域描画
	CBrush brEditBk;
	brEditBk.CreateSolidBrush(GetEditBkColor());
	dcMem.SelectObject(&brEditBk);
	dcMem.PatBlt(0, offset, rc.Width(), itemHeight, PATCOPY);

	// テキスト描画
	rcItem = CRect(CPoint(8, offset + 8), CSize(rc.Width(), itemHeight));
	dcMem.SetBkColor(GetEditBkColor());
	dcMem.SetTextColor(GetEditTextColor());
	dcMem.DrawText(_T("(入力欄)"), rcItem, DT_LEFT);

	offset += itemHeight;

	// リスト領域描画
	CBrush brListBk;
	brListBk.CreateSolidBrush(GetListBkColor());
	CBrush brListBkAlt;
	brListBkAlt.CreateSolidBrush(GetListBkAltColor());
	CBrush brListBkHL;
	brListBkHL.CreateSolidBrush(GetListBkHLColor());

	CString labelText;
	int i = 0;
	while (offset < rc.bottom) {
		if (i == 0) {
			dcMem.SelectObject(&brListBkHL);
			dcMem.SetBkColor(GetListBkHLColor());
			dcMem.SetTextColor(GetListTextHLColor());
		}
		else {
			dcMem.SelectObject((i % 2) ? &brListBkAlt : &brListBk);
			dcMem.SetBkColor((i % 2) ? GetListBkAltColor() : GetListBkColor());
			dcMem.SetTextColor(GetListTextColor());
		}
		dcMem.PatBlt(0, offset, rc.Width(), itemHeight, PATCOPY);

		// テキスト描画
		labelText.Format(_T("候補%d"), i + 1);
		rcItem = CRect(CPoint(8, offset + 8), CSize(rc.Width(), itemHeight));
		dcMem.DrawText(labelText, rcItem, DT_LEFT);
		
		offset += itemHeight;
		i++;
	}

	std::array<CPoint, 5> points{
	 	CPoint(0, 0), CPoint(rc.Width()-1, 0),
		CPoint(rc.Width()-1, rc.Height()-1), CPoint(0, rc.Height()-1),
	 	CPoint(0,0)
	};
	dcMem.Polyline(points.data(), 5);

	dcMem.SelectObject(orgFont);
	dcMem.SelectObject(orgBr);
	dcMem.SelectObject(orgBmp);

	mPreview.InvalidateRect(nullptr);
}

void ColorSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;

	mIsUseSystemSettings = settingsPtr->Get(_T("ColorSettings:UseSystemColor"), true) ? TRUE : FALSE;

	SystemColorScheme sysColor;

	mWindowTextColor = settingsPtr->Get(_T("CurrentColor:WindowText"), (int)sysColor.GetTextColor());
	mWindowBkColor = settingsPtr->Get(_T("CurrentColor:WindowBackground"), (int)sysColor.GetBackgroundColor());
	mEditTextColor = settingsPtr->Get(_T("CurrentColor:EditText"), (int)sysColor.GetEditTextColor());
	mEditBkColor = settingsPtr->Get(_T("CurrentColor:EditBackground"), (int)sysColor.GetEditBackgroundColor());
	mListTextColor = settingsPtr->Get(_T("CurrentColor:ListText"), (int)sysColor.GetListTextColor());
	mListBkColor = settingsPtr->Get(_T("CurrentColor:ListBackground"), (int)sysColor.GetListBackgroundColor());
	mListBkAltColor = settingsPtr->Get(_T("CurrentColor:ListBackgroundAlt"), (int)sysColor.GetListBackgroundAltColor());
	mListTextHLColor = settingsPtr->Get(_T("CurrentColor:ListHighlightText"), (int)sysColor.GetListHighlightTextColor());
	mListBkHLColor = settingsPtr->Get(_T("CurrentColor:ListHighlightBackground"), (int)sysColor.GetListHighlightBackgroundColor());


	// カラーをコントロールに設定する
	ApplyToCtrl();

	UpdateStatus();

	UpdateData(FALSE);
}

void ColorSettingDialog::ApplyToCtrl()
{
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWTEXT))->SetColor(mWindowTextColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWBACKGROUND))->SetColor(mWindowBkColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITTEXT))->SetColor(mEditTextColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITBACKGROUND))->SetColor(mEditBkColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXT))->SetColor(mListTextColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUND))->SetColor(mListBkColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDALT))->SetColor(mListBkAltColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXTHIGHLIGHT))->SetColor(mListTextHLColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDHIGHLIGHT))->SetColor(mListBkHLColor);
}

void ColorSettingDialog::OnButtonRestore()
{
	mWindowTextColor = mSysScheme.GetTextColor();
	mWindowBkColor = mSysScheme.GetBackgroundColor();
	mEditTextColor = mSysScheme.GetEditTextColor();
	mEditBkColor = mSysScheme.GetEditBackgroundColor();
	mListTextColor = mSysScheme.GetListTextColor();
	mListBkColor = mSysScheme.GetListBackgroundColor();
	mListBkAltColor = mSysScheme.GetListBackgroundAltColor();
	mListTextHLColor = mSysScheme.GetListHighlightTextColor();
	mListBkHLColor = mSysScheme.GetListHighlightBackgroundColor();

	ApplyToCtrl();

	UpdateStatus();
}

void ColorSettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageColor::PImpl
{
	ColorSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageColor)

AppSettingPageColor::AppSettingPageColor() : 
	AppSettingPageBase(_T("表示"), _T("配色")),
	in(new PImpl)
{
}

AppSettingPageColor::~AppSettingPageColor()
{
}

// ウインドウを作成する
bool AppSettingPageColor::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_COLOR, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageColor::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageColor::GetOrder()
{
	return 50;
}
// 
bool AppSettingPageColor::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageColor::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageColor::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageColor::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageColor::GetHelpPageId(String& id)
{
	id = "ColorSetting";
	return true;
}

