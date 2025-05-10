#include "pch.h"
#include "framework.h"
#include "RegExpCommandEditDialog.h"
#include "gui/FolderDialog.h"
#include "icon/IconLabel.h"
#include "commands/common/CommandEditValidation.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/ExecutablePath.h"
#include "utility/ShortcutFile.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/Accessibility.h"
#include "icon/IconLoader.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace regexp {

using namespace launcherapp::commands::common;

CommandEditDialog::CommandEditDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_REGEXPCOMMAND, parentWnd),
	mIconLabelPtr(std::make_unique<IconLabel>()),
	mIcon(nullptr)
{
	SetHelpPageId(_T("RegExpEdit"));
}

CommandEditDialog::~CommandEditDialog()
{
}

void CommandEditDialog::SetName(const CString& name)
{
	mParam.mName = name;
}

void CommandEditDialog::SetOriginalName(const CString& name)
{
	mOrgName = name;
}

void CommandEditDialog::SetParam(const CommandParam& param)
{
	mParam = param;
}

const CommandParam& CommandEditDialog::GetParam()
{
	return mParam;
}

void CommandEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mParam.mDescription);
	DDX_Check(pDX, IDC_CHECK_RUNASADMIN, mParam.mRunAs);
	DDX_CBIndex(pDX, IDC_COMBO_SHOWTYPE, mShowType);
	DDX_Text(pDX, IDC_EDIT_PATH, mParam.mNormalAttr.mPath);
	DDX_Text(pDX, IDC_EDIT_PARAM, mParam.mNormalAttr.mParam);
	DDX_Text(pDX, IDC_EDIT_DIR, mParam.mNormalAttr.mDir);
	DDX_Text(pDX, IDC_EDIT_PATTERNSTR, mParam.mPatternStr);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CommandEditDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnEditNameChanged)
	ON_EN_CHANGE(IDC_EDIT_PATH, OnEditPathChanged)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE1, OnButtonBrowseFile1Clicked)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR1, OnButtonBrowseDir1Clicked)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR3, OnButtonBrowseDir3Clicked)
	ON_COMMAND(IDC_CHECK_USE0, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_RESOLVESHORTCUT, OnButtonResolveShortcut)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_APP + 11, OnUserMessageIconChanged)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL CommandEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	int type = mParam.mNormalAttr.mShowType;
	if (type == SW_SHOWMINIMIZED) {
		mShowType = 2;
	}
	else if (type == SW_MAXIMIZE) {
		mShowType = 1;
	}
	else {
		mShowType = 0;
	}

	SetIcon(IconLoader::Get()->LoadDefaultIcon(), FALSE);

	mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);
	mIconLabelPtr->EnableIconChange();

	if (mParam.mIconData.empty()) {
		CString resolvedPath(mParam.mNormalAttr.mPath);
		ExpandMacros(resolvedPath);
		mIcon = IconLoader::Get()->LoadIconFromPath(resolvedPath);
	}
	else {
		mIcon = IconLoader::Get()->LoadIconFromStream(mParam.mIconData);
	}

	CString caption;
  GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)mOrgName);

	caption += suffix;
	SetWindowText(caption);

	// File&Folder Select Button
	GetDlgItem(IDC_BUTTON_BROWSEFILE1)->SetWindowTextW(L"\U0001F4C4");
	GetDlgItem(IDC_BUTTON_BROWSEDIR1)->SetWindowTextW(L"\U0001F4C2");
	GetDlgItem(IDC_BUTTON_BROWSEDIR3)->SetWindowTextW(L"\U0001F4C2");

	UpdateStatus();
	UpdateData(FALSE);

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	return TRUE;
}

bool CommandEditDialog::UpdateStatus()
{
	mMessage.Empty();

	BOOL isShortcut = CString(_T(".lnk")).CompareNoCase(PathFindExtension(mParam.mNormalAttr.mPath)) == 0;
	GetDlgItem(IDC_BUTTON_RESOLVESHORTCUT)->ShowWindow(isShortcut? SW_SHOW : SW_HIDE);

	if (mParam.mIconData.empty()) {
		CString resolvedPath(mParam.mNormalAttr.mPath);
		ExpandMacros(resolvedPath);
		mIcon = IconLoader::Get()->LoadIconFromPath(resolvedPath);
	}

	if (mIcon) {
		mIconLabelPtr->DrawIcon(mIcon);
	}

	// 名前チェック
	bool canPressOK =
	 	launcherapp::commands::common::IsValidCommandName(mParam.mName, mOrgName, mMessage);
	//
	if (canPressOK && mParam.mNormalAttr.mPath.IsEmpty()) {
		mMessage.LoadString(IDS_ERR_PATHISEMPTY);
		canPressOK = false;
	}

	// パスチェック
	ExecutablePath path(mParam.mNormalAttr.mPath);
	if (path.IsExecutable() == false) {
		mMessage = _T("ファイルまたはディレクトリが存在しません。");
		canPressOK = false;
	}


	// 変換
	if (mShowType == 1) {
		mParam.mNormalAttr.mShowType = SW_MAXIMIZE;
	}
	else if (mShowType == 2) {
		mParam.mNormalAttr.mShowType = SW_SHOWMINIMIZED;
	}
	else {
		mParam.mNormalAttr.mShowType = SW_NORMAL;
	}

	GetDlgItem(IDOK)->EnableWindow(canPressOK ? TRUE : FALSE);

	return true;
}

void CommandEditDialog::OnEditNameChanged()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::OnEditPathChanged()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::OnEditPath0Changed()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::OnButtonBrowseFile1Clicked()
{
	UpdateData();
	CFileDialog dlg(TRUE, NULL, mParam.mNormalAttr.mPath, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mNormalAttr.mPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::OnButtonBrowseDir1Clicked()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mParam.mNormalAttr.mPath, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mNormalAttr.mPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::OnButtonBrowseDir3Clicked()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mParam.mNormalAttr.mDir, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mNormalAttr.mDir = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}


HBRUSH CommandEditDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (::utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}

void CommandEditDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	try {
		tregex regTmp((LPCTSTR)mParam.mPatternStr);
	}
	catch(std::regex_error& e) {
		CString msg((LPCTSTR)IDS_ERR_INVALIDREGEXP);
		msg += _T("\n");

		CStringA what(e.what());
		msg += _T("\n");
		msg += (CString)what;
		msg += _T("\n");
		msg += mParam.mPatternStr;
		AfxMessageBox(msg);
		return;
	}

	__super::OnOK();
}



void CommandEditDialog::ResolveShortcut(CString& path)
{
	UpdateData();
	auto resolvedPath = ShortcutFile::ResolvePath(path);
	if (resolvedPath.IsEmpty()) {
		CString msg((LPCTSTR)IDS_ERR_INVALIDSHORTCUT);
		msg += _T("\n");
		msg += path;
		AfxMessageBox(msg);
		return;
	}
	path = resolvedPath;
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::OnButtonResolveShortcut()
{
	ResolveShortcut(mParam.mNormalAttr.mPath);
}

LRESULT CommandEditDialog::OnUserMessageIconChanged(WPARAM wp, LPARAM lp)
{
	if (wp != 0) {
		// 変更
		LPCTSTR iconPath = (LPCTSTR)lp;
		if (IconLoader::GetStreamFromPath(iconPath, mParam.mIconData) == false) {
			AfxMessageBox(_T("指定されたファイルは有効なイメージファイルではありません"));
			return 0;
		}

		mIcon = IconLoader::Get()->LoadIconFromStream(mParam.mIconData);
	}
	else {
		// デフォルトに戻す
		CString resolvedPath(mParam.mNormalAttr.mPath);
		ExpandMacros(resolvedPath);
		mIcon = IconLoader::Get()->LoadIconFromPath(resolvedPath);
		mParam.mIconData.clear();
	}

	// 再描画
	if (mIcon) {
		mIconLabelPtr->DrawIcon(mIcon);
	}

	return 0;
}

// マニュアル表示
void CommandEditDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate(_T("MacroList"));
	*pResult = 0;
}


}
}
}

