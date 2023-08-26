#include "pch.h"
#include "framework.h"
#include "CommandEditDialog.h"
#include "commands/shellexecute/ShellExecCommandParam.h"
#include "gui/FolderDialog.h"
#include "gui/IconLabel.h"
#include "gui/CommandHotKeyDialog.h"
#include "core/CommandRepository.h"
#include "utility/ShortcutFile.h"
#include "utility/Accessibility.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const tregex& GetRegexForArgument()
{
	static tregex reg(_T("\\$[1-9*]"));
	return reg;
}


CommandEditDialog::CommandEditDialog(CWnd* parentWnd) : 
	SettingPage(_T("基本"), IDD_NEWCOMMAND, parentWnd),
	mIconLabelPtr(std::make_unique<IconLabel>())
{
}

CommandEditDialog::~CommandEditDialog()
{
}

void CommandEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mParam.mDescription);
	DDX_Check(pDX, IDC_CHECK_RUNASADMIN, mParam.mIsRunAsAdmin);
	DDX_Text(pDX, IDC_EDIT_PATH, mParam.mPath);
	DDX_Text(pDX, IDC_EDIT_PARAM, mParam.mParameter);
	DDX_Text(pDX, IDC_EDIT_HOTKEY2, mHotKey);
	DDX_Check(pDX, IDC_CHECK_SHOWARGINPUT, mParam.mIsShowArgDialog);
}

BEGIN_MESSAGE_MAP(CommandEditDialog, SettingPage)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_PATH, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_PARAM, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE1, OnButtonBrowseFile1Clicked)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR1, OnButtonBrowseDir1Clicked)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_COMMAND(IDC_BUTTON_RESOLVESHORTCUT, OnButtonResolveShortcut)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


BOOL CommandEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	SetIcon(IconLoader::Get()->LoadDefaultIcon(), FALSE);

	mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);

	// File&Folder Select Button
	GetDlgItem(IDC_BUTTON_BROWSEFILE1)->SetWindowTextW(L"\U0001F4C4");
	GetDlgItem(IDC_BUTTON_BROWSEDIR1)->SetWindowTextW(L"\U0001F4C2");

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool CommandEditDialog::UpdateStatus()
{
	mHotKey = mParam.mHotKeyAttr.ToString();
	if (mHotKey.IsEmpty()) {
		mHotKey.LoadString(IDS_NOHOTKEY);
	}

	BOOL isShortcut = CString(_T(".lnk")).CompareNoCase(PathFindExtension(mParam.mPath)) == 0;

	// $1,2,3... または $*の指定がある場合は、引数必須を選択するチェックを表示
	const tregex& regArg = GetRegexForArgument();

	bool hasArg = std::regex_search((LPCTSTR)mParam.mPath, regArg) ||
		            std::regex_search((LPCTSTR)mParam.mParameter, regArg);

	GetDlgItem(IDC_CHECK_SHOWARGINPUT)->ShowWindow(hasArg? SW_SHOW : SW_HIDE);

	// .lnkだったらショートカット解決ボタンを表示する
	GetDlgItem(IDC_BUTTON_RESOLVESHORTCUT)->ShowWindow(isShortcut? SW_SHOW : SW_HIDE);

	// .exe/.batでなければ、管理者権限実行を無効化する
	BOOL isExecutable = CString(_T(".exe")).CompareNoCase(PathFindExtension(mParam.mPath)) == 0 ||
	                    CString(_T(".bat")).CompareNoCase(PathFindExtension(mParam.mPath)) == 0;
	GetDlgItem(IDC_CHECK_RUNASADMIN)->EnableWindow(isExecutable);
	if (isExecutable == FALSE) {
		mParam.mIsRunAsAdmin = FALSE;
	}

	mIconLabelPtr->DrawIcon(IconLoader::Get()->LoadIconFromPath(mParam.mPath));

	if (mParam.mName.IsEmpty()) {
		mMessage.LoadString(IDS_ERR_NAMEISEMPTY);
		DisalbleOKButton();
		return false;
	}

	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();

	// 重複チェック
	if (mParam.mName.CompareNoCase(mOrgName) != 0) {
		auto cmd = cmdRepoPtr->QueryAsWholeMatch(mParam.mName, false);
		if (cmd != nullptr) {
			cmd->Release();
			mMessage.LoadString(IDS_ERR_NAMEALREADYEXISTS);
			DisalbleOKButton();
			return false;
		}
	}

	// 使えない文字チェック
	if (cmdRepoPtr->IsValidAsName(mParam.mName) == false) {
		mMessage.LoadString(IDS_ERR_ILLEGALCHARCONTAINS);
		DisalbleOKButton();
		return false;
	}

	//
	if (mParam.mPath.IsEmpty()) {
		mMessage.LoadString(IDS_ERR_PATHISEMPTY);
		DisalbleOKButton();
		return false;
	}

	mMessage.Empty();
	EnalbleOKButton();

	return true;
}

void CommandEditDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}


void CommandEditDialog::OnButtonBrowseFile1Clicked()
{
	UpdateData();
	CFileDialog dlg(TRUE, NULL, mParam.mPath, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::OnButtonBrowseDir1Clicked()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mParam.mPath, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}


HBRUSH CommandEditDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (utility::IsHighContrastMode()) {
		return __super::OnCtlColor(pDC, pWnd, nCtlColor);
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}

	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CommandEditDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL CommandEditDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void CommandEditDialog::OnEnterSettings()
{
	auto param = (CommandParam*)GetParam();

	mOrgName = param->mName;
	mParam = *param;

	CString caption;
  GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)mOrgName);

	caption += suffix;
	SetWindowText(caption);

}


void CommandEditDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	auto param = (CommandParam*)GetParam();

	param->mName = mParam.mName;
	param->mDescription = mParam.mDescription;
	param->mIsRunAsAdmin = mParam.mIsRunAsAdmin;
	param->mShowType = mParam.mShowType;
	param->mPath = mParam.mPath;
	param->mParameter = mParam.mParameter;
	param->mIsShowArgDialog = mParam.mIsShowArgDialog;
	param->mHotKeyAttr = mParam.mHotKeyAttr;

	const tregex& regArg = GetRegexForArgument();
	bool hasArg = std::regex_search((LPCTSTR)mParam.mPath, regArg) ||
		            std::regex_search((LPCTSTR)mParam.mParameter, regArg);
	if (hasArg == false) {
		param->mIsShowArgDialog = false;
	}

	__super::OnOK();
}


void CommandEditDialog::OnButtonHotKey()
{
	UpdateData();

	CommandHotKeyDialog dlg(mParam.mHotKeyAttr);
	dlg.mIsGlobal = mParam.mIsGlobal;
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mParam.mHotKeyAttr);
	mParam.mIsGlobal = dlg.IsGlobal();

	UpdateStatus();
	UpdateData(FALSE);
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
	ResolveShortcut(mParam.mPath);
}


