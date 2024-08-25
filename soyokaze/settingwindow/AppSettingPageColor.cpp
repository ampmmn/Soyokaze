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


struct AppSettingPageColor::PImpl
{
	BOOL mIsUseSystemSettings = TRUE;

	COLORREF GetWindowTextColor();
	COLORREF GetWindowBkColor();
	COLORREF GetEditTextColor();
	COLORREF GetEditBkColor();
	COLORREF GetListTextColor();
	COLORREF GetListBkColor();
	COLORREF GetListBkAltColor();
	COLORREF GetListTextHLColor();
	COLORREF GetListBkHLColor();


	COLORREF mWindowTextColor = 0;
	COLORREF mWindowBkColor = 0;
	COLORREF mEditTextColor = 0;
	COLORREF mEditBkColor = 0;
	COLORREF mListTextColor = 0;
	COLORREF mListBkColor = 0;
	COLORREF mListBkAltColor = 0;
	COLORREF mListTextHLColor = 0;
	COLORREF mListBkHLColor = 0;

	ImageLabel mPreview;

	SystemColorScheme mSysScheme;

};

COLORREF AppSettingPageColor::PImpl::GetWindowTextColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetTextColor() : mWindowTextColor;
}

COLORREF AppSettingPageColor::PImpl::GetWindowBkColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetBackgroundColor() : mWindowBkColor;
}

COLORREF AppSettingPageColor::PImpl::GetEditTextColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetEditTextColor() : mEditTextColor;
}

COLORREF AppSettingPageColor::PImpl::GetEditBkColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetEditBackgroundColor() : mEditBkColor;
}

COLORREF AppSettingPageColor::PImpl::GetListTextColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListTextColor() : mListTextColor;
}

COLORREF AppSettingPageColor::PImpl::GetListBkColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListBackgroundColor() : mListBkColor;
}

COLORREF AppSettingPageColor::PImpl::GetListBkAltColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListBackgroundAltColor() : mListBkAltColor;
}

COLORREF AppSettingPageColor::PImpl::GetListTextHLColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListHighlightTextColor() : mListTextHLColor;
}

COLORREF AppSettingPageColor::PImpl::GetListBkHLColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListHighlightBackgroundColor() : mListBkHLColor;
}




AppSettingPageColor::AppSettingPageColor(CWnd* parentWnd) : 
	SettingPage(_T("配色"), IDD_APPSETTING_COLOR, parentWnd),
	in(new PImpl)
{
}

AppSettingPageColor::~AppSettingPageColor()
{
}

BOOL AppSettingPageColor::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL AppSettingPageColor::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void AppSettingPageColor::OnOK()
{
	UpdateData();

	in->mWindowTextColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWTEXT))->GetColor();
	in->mWindowBkColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWBACKGROUND))->GetColor();
	in->mEditTextColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITTEXT))->GetColor();
	in->mEditBkColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITBACKGROUND))->GetColor();
	in->mListTextColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXT))->GetColor();
	in->mListBkColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUND))->GetColor();
	in->mListBkAltColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDALT))->GetColor();
	in->mListTextHLColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXTHIGHLIGHT))->GetColor();
	in->mListBkHLColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDHIGHLIGHT))->GetColor();

	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("ColorSettings:UseSystemColor"), in->mIsUseSystemSettings ? true : false);
	settingsPtr->Set(_T("CurrentColor:WindowText"), (int)in->mWindowTextColor);
	settingsPtr->Set(_T("CurrentColor:WindowBackground"), (int)in->mWindowBkColor);
	settingsPtr->Set(_T("CurrentColor:EditText"), (int)in->mEditTextColor);
	settingsPtr->Set(_T("CurrentColor:EditBackground"), (int)in->mEditBkColor);
	settingsPtr->Set(_T("CurrentColor:ListText"), (int)in->mListTextColor);
	settingsPtr->Set(_T("CurrentColor:ListBackground"), (int)in->mListBkColor);
	settingsPtr->Set(_T("CurrentColor:ListBackgroundAlt"), (int)in->mListBkAltColor);
	settingsPtr->Set(_T("CurrentColor:ListHighlightText"), (int)in->mListTextHLColor);
	settingsPtr->Set(_T("CurrentColor:ListHighlightBackground"), (int)in->mListBkHLColor);

	__super::OnOK();
}

void AppSettingPageColor::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_USEDEFAULT, in->mIsUseSystemSettings);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(AppSettingPageColor, SettingPage)
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

BOOL AppSettingPageColor::OnInitDialog()
{
	__super::OnInitDialog();

	in->mPreview.SubclassDlgItem(IDC_STATIC_PREVIEW, this);

	if (utility::IsHighContrastMode()) {
		DisableAllControls();
	}

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

void AppSettingPageColor::DisableAllControls()
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

bool AppSettingPageColor::UpdateStatus()
{
	BOOL isEnableColor = in->mIsUseSystemSettings == FALSE;
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

	in->mWindowTextColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWTEXT))->GetColor();
	in->mWindowBkColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWBACKGROUND))->GetColor();
	in->mEditTextColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITTEXT))->GetColor();
	in->mEditBkColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITBACKGROUND))->GetColor();
	in->mListTextColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXT))->GetColor();
	in->mListBkColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUND))->GetColor();
	in->mListBkAltColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDALT))->GetColor();
	in->mListTextHLColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXTHIGHLIGHT))->GetColor();
	in->mListBkHLColor = ((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDHIGHLIGHT))->GetColor();

	// プレビューを更新
	DrawPreview();
	return true;
}

void AppSettingPageColor::DrawPreview()
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

	CBitmap* bmp = in->mPreview.GetBitmap();

	CRect rc;
	in->mPreview.GetClientRect(&rc);

	CBrush brBk;
	brBk.CreateSolidBrush(in->GetWindowBkColor());

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
	dcMem.SetBkColor(in->GetWindowBkColor());
	dcMem.SetTextColor(in->GetWindowTextColor());
	dcMem.DrawText(pref->GetDefaultComment(), rcItem, DT_LEFT);
;
	offset += itemHeight;

	// テキスト領域描画
	CBrush brEditBk;
	brEditBk.CreateSolidBrush(in->GetEditBkColor());
	dcMem.SelectObject(&brEditBk);
	dcMem.PatBlt(0, offset, rc.Width(), itemHeight, PATCOPY);

	// テキスト描画
	rcItem = CRect(CPoint(8, offset + 8), CSize(rc.Width(), itemHeight));
	dcMem.SetBkColor(in->GetEditBkColor());
	dcMem.SetTextColor(in->GetEditTextColor());
	dcMem.DrawText(_T("(入力欄)"), rcItem, DT_LEFT);

	offset += itemHeight;

	// リスト領域描画
	CBrush brListBk;
	brListBk.CreateSolidBrush(in->GetListBkColor());
	CBrush brListBkAlt;
	brListBkAlt.CreateSolidBrush(in->GetListBkAltColor());
	CBrush brListBkHL;
	brListBkHL.CreateSolidBrush(in->GetListBkHLColor());

	CString labelText;
	int i = 0;
	while (offset < rc.bottom) {
		if (i == 0) {
			dcMem.SelectObject(&brListBkHL);
			dcMem.SetBkColor(in->GetListBkHLColor());
			dcMem.SetTextColor(in->GetListTextHLColor());
		}
		else {
			dcMem.SelectObject((i % 2) ? &brListBkAlt : &brListBk);
			dcMem.SetBkColor((i % 2) ? in->GetListBkAltColor() : in->GetListBkColor());
			dcMem.SetTextColor(in->GetListTextColor());
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

	in->mPreview.InvalidateRect(nullptr);
}

void AppSettingPageColor::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();

	in->mIsUseSystemSettings = settingsPtr->Get(_T("ColorSettings:UseSystemColor"), true) ? TRUE : FALSE;

	SystemColorScheme sysColor;

	in->mWindowTextColor = settingsPtr->Get(_T("CurrentColor:WindowText"), (int)sysColor.GetTextColor());
	in->mWindowBkColor = settingsPtr->Get(_T("CurrentColor:WindowBackground"), (int)sysColor.GetBackgroundColor());
	in->mEditTextColor = settingsPtr->Get(_T("CurrentColor:EditText"), (int)sysColor.GetEditTextColor());
	in->mEditBkColor = settingsPtr->Get(_T("CurrentColor:EditBackground"), (int)sysColor.GetEditBackgroundColor());
	in->mListTextColor = settingsPtr->Get(_T("CurrentColor:ListText"), (int)sysColor.GetListTextColor());
	in->mListBkColor = settingsPtr->Get(_T("CurrentColor:ListBackground"), (int)sysColor.GetListBackgroundColor());
	in->mListBkAltColor = settingsPtr->Get(_T("CurrentColor:ListBackgroundAlt"), (int)sysColor.GetListBackgroundAltColor());
	in->mListTextHLColor = settingsPtr->Get(_T("CurrentColor:ListHighlightText"), (int)sysColor.GetListHighlightTextColor());
	in->mListBkHLColor = settingsPtr->Get(_T("CurrentColor:ListHighlightBackground"), (int)sysColor.GetListHighlightBackgroundColor());


	// カラーをコントロールに設定する
	ApplyToCtrl();

	UpdateStatus();

	UpdateData(FALSE);
}

void AppSettingPageColor::ApplyToCtrl()
{
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWTEXT))->SetColor(in->mWindowTextColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWBACKGROUND))->SetColor(in->mWindowBkColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITTEXT))->SetColor(in->mEditTextColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITBACKGROUND))->SetColor(in->mEditBkColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXT))->SetColor(in->mListTextColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUND))->SetColor(in->mListBkColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDALT))->SetColor(in->mListBkAltColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXTHIGHLIGHT))->SetColor(in->mListTextHLColor);
	((CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDHIGHLIGHT))->SetColor(in->mListBkHLColor);
}

bool AppSettingPageColor::GetHelpPageId(CString& id)
{
	id = _T("ColorSetting");
	return true;
}

void AppSettingPageColor::OnButtonRestore()
{
	in->mWindowTextColor = in->mSysScheme.GetTextColor();
	in->mWindowBkColor = in->mSysScheme.GetBackgroundColor();
	in->mEditTextColor = in->mSysScheme.GetEditTextColor();
	in->mEditBkColor = in->mSysScheme.GetEditBackgroundColor();
	in->mListTextColor = in->mSysScheme.GetListTextColor();
	in->mListBkColor = in->mSysScheme.GetListBackgroundColor();
	in->mListBkAltColor = in->mSysScheme.GetListBackgroundAltColor();
	in->mListTextHLColor = in->mSysScheme.GetListHighlightTextColor();
	in->mListBkHLColor = in->mSysScheme.GetListHighlightBackgroundColor();

	ApplyToCtrl();

	UpdateStatus();
}

void AppSettingPageColor::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

