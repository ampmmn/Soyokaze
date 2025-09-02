#include "pch.h"
#include "framework.h"
#include "RegExpCommandEditDialog.h"
#include "gui/FolderDialog.h"
#include "icon/IconLabel.h"
#include "commands/validation/CommandEditValidation.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/ExecutablePath.h"
#include "utility/ShortcutFile.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/Accessibility.h"
#include "icon/IconLoader.h"
#include "icon/CommandIcon.h"
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
using CommandIcon = launcherapp::icon::CommandIcon;

struct CommandEditDialog::PImpl
{
	// 編集開始時のコマンド名
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

	// アイコン(表示用)
	CommandIcon mIcon;
	// 表示方法
	int mShowType{0};

	CommandParam mParam;

	std::unique_ptr<IconLabel> mIconLabelPtr{std::make_unique<IconLabel>()};

};

CommandEditDialog::CommandEditDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_REGEXPCOMMAND, parentWnd),
	in(new PImpl)
{
	SetHelpPageId("RegExpEdit");
}

CommandEditDialog::~CommandEditDialog()
{
}

void CommandEditDialog::SetName(const CString& name)
{
	in->mParam.mName = name;
}

void CommandEditDialog::SetOriginalName(const CString& name)
{
	in->mOrgName = name;
}

void CommandEditDialog::SetParam(const CommandParam& param)
{
	in->mParam = param;
}

const CommandParam& CommandEditDialog::GetParam()
{
	return in->mParam;
}

void CommandEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, in->mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, in->mParam.mDescription);
	DDX_Check(pDX, IDC_CHECK_RUNASADMIN, in->mParam.mRunAs);
	DDX_CBIndex(pDX, IDC_COMBO_SHOWTYPE, in->mShowType);
	DDX_Text(pDX, IDC_EDIT_PATH, in->mParam.mNormalAttr.mPath);
	DDX_Text(pDX, IDC_EDIT_PARAM, in->mParam.mNormalAttr.mParam);
	DDX_Text(pDX, IDC_EDIT_DIR, in->mParam.mNormalAttr.mDir);
	DDX_Text(pDX, IDC_EDIT_PATTERNSTR, in->mParam.mPatternStr);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CommandEditDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_PATTERNSTR, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_PATH, OnUpdateStatus)
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

	int type = in->mParam.mNormalAttr.mShowType;
	if (type == SW_SHOWMINIMIZED) {
		in->mShowType = 2;
	}
	else if (type == SW_MAXIMIZE) {
		in->mShowType = 1;
	}
	else {
		in->mShowType = 0;
	}

	SetIcon(IconLoader::Get()->LoadDefaultIcon(), FALSE);

	in->mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);
	in->mIconLabelPtr->EnableIconChange();

	if (in->mParam.mIconData.empty()) {
		CString resolvedPath(in->mParam.mNormalAttr.mPath);
		ExpandMacros(resolvedPath);
		in->mIcon.LoadFromPath(resolvedPath);
	}
	else {
		in->mIcon.LoadFromStream(in->mParam.mIconData);
	}

	CString caption;
  GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

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
	in->mMessage.Empty();

	auto& param = in->mParam;

	BOOL isShortcut = CString(_T(".lnk")).CompareNoCase(PathFindExtension(param.mNormalAttr.mPath)) == 0;
	GetDlgItem(IDC_BUTTON_RESOLVESHORTCUT)->ShowWindow(isShortcut? SW_SHOW : SW_HIDE);

	if (param.mIconData.empty()) {
		CString resolvedPath(param.mNormalAttr.mPath);
		ExpandMacros(resolvedPath);
		in->mIcon.LoadFromPath(resolvedPath);
	}

	in->mIconLabelPtr->DrawIcon(in->mIcon);

	// 変換
	if (in->mShowType == 1) {
		param.mNormalAttr.mShowType = SW_MAXIMIZE;
	}
	else if (in->mShowType == 2) {
		param.mNormalAttr.mShowType = SW_SHOWMINIMIZED;
	}
	else {
		param.mNormalAttr.mShowType = SW_NORMAL;
	}


	// 名前チェック
	bool isNameValid =
	 	launcherapp::commands::validation::IsValidCommandName(param.mName, in->mOrgName, in->mMessage);
	if (isNameValid == false) {
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	if (param.mPatternStr.IsEmpty()) {
		in->mMessage = _T("パターンを入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	bool canPressOK = true;
	if (param.mNormalAttr.mPath.IsEmpty()) {
		in->mMessage.LoadString(IDS_ERR_PATHISEMPTY);
		canPressOK = false;
	}

	// パスチェック
	ExecutablePath path(param.mNormalAttr.mPath);
	if (path.IsExecutable() == false) {
		in->mMessage = _T("ファイルまたはディレクトリが存在しません。");
		canPressOK = false;
	}

	GetDlgItem(IDOK)->EnableWindow(canPressOK ? TRUE : FALSE);

	return true;
}

void CommandEditDialog::OnButtonBrowseFile1Clicked()
{
	UpdateData();
	CFileDialog dlg(TRUE, NULL, in->mParam.mNormalAttr.mPath, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	in->mParam.mNormalAttr.mPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::OnButtonBrowseDir1Clicked()
{
	UpdateData();
	CFolderDialog dlg(_T(""), in->mParam.mNormalAttr.mPath, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	in->mParam.mNormalAttr.mPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::OnButtonBrowseDir3Clicked()
{
	UpdateData();
	CFolderDialog dlg(_T(""), in->mParam.mNormalAttr.mDir, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	in->mParam.mNormalAttr.mDir = dlg.GetPathName();
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
		COLORREF crTxt = in->mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
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
		tregex regTmp((LPCTSTR)in->mParam.mPatternStr);
	}
	catch(std::regex_error& e) {
		CString msg((LPCTSTR)IDS_ERR_INVALIDREGEXP);
		msg += _T("\n");

		CStringA what(e.what());
		msg += _T("\n");
		msg += (CString)what;
		msg += _T("\n");
		msg += in->mParam.mPatternStr;
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
	ResolveShortcut(in->mParam.mNormalAttr.mPath);
}

LRESULT CommandEditDialog::OnUserMessageIconChanged(WPARAM wp, LPARAM lp)
{
	auto& param = in->mParam;
	if (wp != 0) {
		// 変更
		LPCTSTR iconPath = (LPCTSTR)lp;
		if (IconLoader::GetStreamFromPath(iconPath, param.mIconData) == false) {
			AfxMessageBox(_T("指定されたファイルは有効なイメージファイルではありません"));
			return 0;
		}

		in->mIcon.LoadFromStream(param.mIconData);
	}
	else {
		// デフォルトに戻す
		param.mIconData.clear();

		CString resolvedPath(param.mNormalAttr.mPath);
		ExpandMacros(resolvedPath);
		in->mIcon.LoadFromPath(resolvedPath);
	}

	// 再描画
	in->mIconLabelPtr->DrawIcon(in->mIcon);

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
	manual->Navigate("MacroList");
	*pResult = 0;
}


}
}
}

