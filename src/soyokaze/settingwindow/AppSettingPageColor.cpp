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
#include <tuple>
#include <map>

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
	void DrawPreview();
	void AppendUndo(CMFCColorButton* btn);

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL PreTranslateMessage(MSG* msg) override;
	BOOL OnInitDialog() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	void OnButtonRestore();
	void OnUpdateStatus();
	void OnButtonWindowTextColor();
	void OnButtonWindowBkColor();
	void OnButtonEditTextColor();
	void OnButtonEditBkColor();
	void OnButtonListTextColor();
	void OnButtonListBkColor();
	void OnButtonListBkAltColor();
	void OnButtonListTextHLColor();
	void OnButtonListBkHLColor();
	void OnUndo();

	BOOL mIsUseSystemSettings{TRUE};

	CMFCColorButton* mWindowTextColorButton;
	CMFCColorButton* mWindowBkColorButton;
	CMFCColorButton* mEditTextColorButton;
	CMFCColorButton* mEditBkColorButton;
	CMFCColorButton* mListTextColorButton;
	CMFCColorButton* mListBkColorButton;
	CMFCColorButton* mListBkAltColorButton;
	CMFCColorButton* mListTextHLColorButton;
	CMFCColorButton* mListBkHLColorButton;

	ImageLabel mPreview;

	SystemColorScheme mSysScheme;

	Settings* mSettingsPtr{nullptr};

	std::map<CMFCColorButton*,COLORREF> mCurrentColorMap;
	std::vector<std::tuple<CMFCColorButton*,COLORREF> > mUndo;
};

COLORREF ColorSettingDialog::GetWindowTextColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetTextColor() : mWindowTextColorButton->GetColor();
}

COLORREF ColorSettingDialog::GetWindowBkColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetBackgroundColor() : mWindowBkColorButton->GetColor();
}

COLORREF ColorSettingDialog::GetEditTextColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetEditTextColor() : mEditTextColorButton->GetColor();
}

COLORREF ColorSettingDialog::GetEditBkColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetEditBackgroundColor() : mEditBkColorButton->GetColor();
}

COLORREF ColorSettingDialog::GetListTextColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListTextColor() : mListTextColorButton->GetColor();
}

COLORREF ColorSettingDialog::GetListBkColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListBackgroundColor() : mListBkColorButton->GetColor();
}

COLORREF ColorSettingDialog::GetListBkAltColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListBackgroundAltColor() : mListBkAltColorButton->GetColor();
}

COLORREF ColorSettingDialog::GetListTextHLColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListHighlightTextColor() : mListTextHLColorButton->GetColor();
}

COLORREF ColorSettingDialog::GetListBkHLColor()
{
	return mIsUseSystemSettings ? mSysScheme.GetListHighlightBackgroundColor() : mListBkHLColorButton->GetColor();
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

	settingsPtr->Set(_T("ColorSettings:UseSystemColor"), mIsUseSystemSettings ? true : false);
	settingsPtr->Set(_T("CurrentColor:WindowText"), (int)mWindowTextColorButton->GetColor());
	settingsPtr->Set(_T("CurrentColor:WindowBackground"), (int)mWindowBkColorButton->GetColor());
	settingsPtr->Set(_T("CurrentColor:EditText"), (int)mEditTextColorButton->GetColor());
	settingsPtr->Set(_T("CurrentColor:EditBackground"), (int)mEditBkColorButton->GetColor());
	settingsPtr->Set(_T("CurrentColor:ListText"), (int)mListTextColorButton->GetColor());
	settingsPtr->Set(_T("CurrentColor:ListBackground"), (int)mListBkColorButton->GetColor());
	settingsPtr->Set(_T("CurrentColor:ListBackgroundAlt"), (int)mListBkAltColorButton->GetColor());
	settingsPtr->Set(_T("CurrentColor:ListHighlightText"), (int)mListTextHLColorButton->GetColor());
	settingsPtr->Set(_T("CurrentColor:ListHighlightBackground"), (int)mListBkHLColorButton->GetColor());

	UpdateData(FALSE);

	__super::OnOK();
}

void ColorSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_USEDEFAULT, mIsUseSystemSettings);
}

BOOL ColorSettingDialog::PreTranslateMessage(MSG* msg)
{
	if (msg->message == WM_KEYDOWN) {
		if ((GetKeyState(VK_CONTROL) & 0x8000) && msg->wParam == 'Z') {
			OnUndo();
			return TRUE;
		}
	}
	return __super::PreTranslateMessage(msg);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(ColorSettingDialog, CDialog)
	ON_COMMAND(IDC_BUTTON_RESTORE, OnButtonRestore)
	ON_COMMAND(IDC_CHECK_USEDEFAULT, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_WINDOWTEXT, OnButtonWindowTextColor)
	ON_COMMAND(IDC_BUTTON_WINDOWBACKGROUND, OnButtonWindowBkColor)
	ON_COMMAND(IDC_BUTTON_EDITTEXT, OnButtonEditTextColor)
	ON_COMMAND(IDC_BUTTON_EDITBACKGROUND, OnButtonEditBkColor)
	ON_COMMAND(IDC_BUTTON_LISTTEXT, OnButtonListTextColor)
	ON_COMMAND(IDC_BUTTON_LISTBACKGROUND, OnButtonListBkColor)
	ON_COMMAND(IDC_BUTTON_LISTBACKGROUNDALT, OnButtonListBkAltColor)
	ON_COMMAND(IDC_BUTTON_LISTTEXTHIGHLIGHT, OnButtonListTextHLColor)
	ON_COMMAND(IDC_BUTTON_LISTBACKGROUNDHIGHLIGHT, OnButtonListBkHLColor)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL ColorSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	mPreview.SubclassDlgItem(IDC_STATIC_PREVIEW, this);
	mWindowTextColorButton = (CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWTEXT);
	mWindowBkColorButton = (CMFCColorButton*)GetDlgItem(IDC_BUTTON_WINDOWBACKGROUND);
	mEditTextColorButton = (CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITTEXT);
	mEditBkColorButton = (CMFCColorButton*)GetDlgItem(IDC_BUTTON_EDITBACKGROUND);
	mListTextColorButton = (CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXT);
	mListBkColorButton = (CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUND);
	mListBkAltColorButton = (CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDALT);
	mListTextHLColorButton = (CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTTEXTHIGHLIGHT);
	mListBkHLColorButton = (CMFCColorButton*)GetDlgItem(IDC_BUTTON_LISTBACKGROUNDHIGHLIGHT);

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
	mWindowTextColorButton->EnableWindow(isEnableColor);
	mWindowBkColorButton->EnableWindow(isEnableColor);
	mEditTextColorButton->EnableWindow(isEnableColor);
	mEditBkColorButton->EnableWindow(isEnableColor);
	mListTextColorButton->EnableWindow(isEnableColor);
	mListBkColorButton->EnableWindow(isEnableColor);
	mListBkAltColorButton->EnableWindow(isEnableColor);
	mListTextHLColorButton->EnableWindow(isEnableColor);
	mListBkHLColorButton->EnableWindow(isEnableColor);

	//
	CList<COLORREF, COLORREF> lstColors;
	lstColors.AddTail(mWindowTextColorButton->GetColor());
	lstColors.AddTail(mWindowBkColorButton->GetColor());
	lstColors.AddTail(mEditTextColorButton->GetColor());
	lstColors.AddTail(mEditBkColorButton->GetColor());
	lstColors.AddTail(mListTextColorButton->GetColor());
	lstColors.AddTail(mListBkColorButton->GetColor());
	lstColors.AddTail(mListBkAltColorButton->GetColor());
	lstColors.AddTail(mListTextHLColorButton->GetColor());
	lstColors.AddTail(mListBkHLColorButton->GetColor());

	LPCTSTR label = _T("部品別");
	mWindowTextColorButton->SetDocumentColors(label, lstColors);
	mWindowBkColorButton->SetDocumentColors(label, lstColors);
	mEditTextColorButton->SetDocumentColors(label, lstColors);
	mEditBkColorButton->SetDocumentColors(label, lstColors);
	mListTextColorButton->SetDocumentColors(label, lstColors);
	mListBkColorButton->SetDocumentColors(label, lstColors);
	mListBkAltColorButton->SetDocumentColors(label, lstColors);
	mListTextHLColorButton->SetDocumentColors(label, lstColors);
	mListBkHLColorButton->SetDocumentColors(label, lstColors);

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

	mWindowTextColorButton->SetColor(settingsPtr->Get(_T("CurrentColor:WindowText"), (int)sysColor.GetTextColor()));
	mWindowBkColorButton->SetColor(settingsPtr->Get(_T("CurrentColor:WindowBackground"), (int)sysColor.GetBackgroundColor()));
	mEditTextColorButton->SetColor(settingsPtr->Get(_T("CurrentColor:EditText"), (int)sysColor.GetEditTextColor()));
	mEditBkColorButton->SetColor(settingsPtr->Get(_T("CurrentColor:EditBackground"), (int)sysColor.GetEditBackgroundColor()));
	mListTextColorButton->SetColor(settingsPtr->Get(_T("CurrentColor:ListText"), (int)sysColor.GetListTextColor()));
	mListBkColorButton->SetColor(settingsPtr->Get(_T("CurrentColor:ListBackground"), (int)sysColor.GetListBackgroundColor()));
	mListBkAltColorButton->SetColor(settingsPtr->Get(_T("CurrentColor:ListBackgroundAlt"), (int)sysColor.GetListBackgroundAltColor()));
	mListTextHLColorButton->SetColor(settingsPtr->Get(_T("CurrentColor:ListHighlightText"), (int)sysColor.GetListHighlightTextColor()));
	mListBkHLColorButton->SetColor(settingsPtr->Get(_T("CurrentColor:ListHighlightBackground"), (int)sysColor.GetListHighlightBackgroundColor()));

	// Undo用に色を覚えておく
	mCurrentColorMap[mWindowTextColorButton] = mWindowTextColorButton->GetColor();
	mCurrentColorMap[mWindowBkColorButton] = mWindowBkColorButton->GetColor();
	mCurrentColorMap[mEditTextColorButton] = mEditTextColorButton->GetColor();
	mCurrentColorMap[mEditBkColorButton] = mEditBkColorButton->GetColor();
	mCurrentColorMap[mListTextColorButton] = mListTextColorButton->GetColor();
	mCurrentColorMap[mListBkColorButton] = mListBkColorButton->GetColor();
	mCurrentColorMap[mListBkAltColorButton] = mListBkAltColorButton->GetColor();
	mCurrentColorMap[mListTextHLColorButton] = mListTextHLColorButton->GetColor();
	mCurrentColorMap[mListBkHLColorButton] = mListBkHLColorButton->GetColor();
	mUndo.clear();


	UpdateStatus();

	UpdateData(FALSE);
}

void ColorSettingDialog::OnButtonRestore()
{
	mWindowTextColorButton->SetColor(mSysScheme.GetTextColor());
	mWindowBkColorButton->SetColor(mSysScheme.GetBackgroundColor());
	mEditTextColorButton->SetColor(mSysScheme.GetEditTextColor());
	mEditBkColorButton->SetColor(mSysScheme.GetEditBackgroundColor());
	mListTextColorButton->SetColor(mSysScheme.GetListTextColor());
	mListBkColorButton->SetColor(mSysScheme.GetListBackgroundColor());
	mListBkAltColorButton->SetColor(mSysScheme.GetListBackgroundAltColor());
	mListTextHLColorButton->SetColor(mSysScheme.GetListHighlightTextColor());
	mListBkHLColorButton->SetColor(mSysScheme.GetListHighlightBackgroundColor());

	UpdateStatus();
}

void ColorSettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void ColorSettingDialog::OnButtonWindowTextColor()
{
	AppendUndo(mWindowTextColorButton);
	UpdateStatus();
}

void ColorSettingDialog::OnButtonWindowBkColor()
{
	AppendUndo(mWindowBkColorButton);
	UpdateStatus();
}

void ColorSettingDialog::OnButtonEditTextColor()
{
	AppendUndo(mEditTextColorButton);
	UpdateStatus();
}

void ColorSettingDialog::OnButtonEditBkColor()
{
	AppendUndo(mEditBkColorButton);
	UpdateStatus();
}

void ColorSettingDialog::OnButtonListTextColor()
{
	AppendUndo(mListTextColorButton);
	UpdateStatus();
}

void ColorSettingDialog::OnButtonListBkColor()
{
	AppendUndo(mListBkColorButton);
	UpdateStatus();
}

void ColorSettingDialog::OnButtonListBkAltColor()
{
	AppendUndo(mListBkAltColorButton);
	UpdateStatus();
}

void ColorSettingDialog::OnButtonListTextHLColor()
{
	AppendUndo(mListTextHLColorButton);
	UpdateStatus();
}

void ColorSettingDialog::OnButtonListBkHLColor()
{
	AppendUndo(mListBkHLColorButton);
	UpdateStatus();
}

void ColorSettingDialog::OnUndo()
{
	if (mIsUseSystemSettings && mUndo.empty()) {
		// システムの色設定を使う場合はUndoを無効化する
		return ;
	}

	auto& item = mUndo.back();

	auto btn = std::get<0>(item);
	auto cr = std::get<1>(item);

	btn->SetColor(cr);
	mCurrentColorMap[btn] = cr;

	mUndo.pop_back();

	UpdateStatus();
}

void ColorSettingDialog::AppendUndo(CMFCColorButton* btn)
{
	auto itFind = mCurrentColorMap.find(btn);
	if (itFind == mCurrentColorMap.end()) {
		return;
	}

	auto crBefore = itFind->second;
	auto crAfter = btn->GetColor();
	if (crBefore == crAfter) {
		// 変化なし
		return;
	}

	// 履歴に登録
	mUndo.push_back({btn, crBefore});
	mCurrentColorMap[btn] = crAfter;
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

