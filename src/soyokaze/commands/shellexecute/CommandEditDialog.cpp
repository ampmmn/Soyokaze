#include "pch.h"
#include "framework.h"
#include "CommandEditDialog.h"
#include "commands/shellexecute/ShellExecCommandParam.h"
#include "commands/common/ExpandFunctions.h"
#include "gui/FolderDialog.h"
#include "icon/IconLabel.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "commands/common/CommandEditValidation.h"
#include "commands/common/ExpandFunctions.h"
#include "utility/ShortcutFile.h"
#include "utility/Accessibility.h"
#include "utility/Path.h"
#include "utility/LocalPathResolver.h"
#include "icon/IconLoader.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using namespace launcherapp::commands::common;
using LocalPathResolver = launcherapp::utility::LocalPathResolver;

struct CommandEditDialog::PImpl
{
	// 編集開始時のコマンド名
	CString mOrgName;
	// メッセージ欄
	CString mMessage;

	CommandParam mParam;

	std::unique_ptr<IconLabel> mIconLabelPtr{std::make_unique<IconLabel>()};

	// ホットキー(表示用)
	CString mHotKey;

	// アイコン(表示用)
	HICON mIcon{nullptr};

	// 
	CMFCMenuButton mPathMenuBtn;
	CMenu mMenuForPathBtn;
};

static const tregex& GetRegexForArgument()
{
	static tregex reg(_T("\\$[1-9*]"));
	return reg;
}


CommandEditDialog::CommandEditDialog(CWnd* parentWnd) : 
	SettingPage(_T("基本"), IDD_NEWCOMMAND, parentWnd),
	in(new PImpl)
{
}

CommandEditDialog::~CommandEditDialog()
{
}

void CommandEditDialog::SetOriginalName(const CString& name)
{
	in->mOrgName = name;
}

void CommandEditDialog::DoDataExchange(CDataExchange* pDX)
{
	auto& param = in->mParam;
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, param.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, param.mDescription);
	DDX_Check(pDX, IDC_CHECK_RUNASADMIN, param.mIsRunAsAdmin);
	DDX_Text(pDX, IDC_EDIT_PATH, param.mNormalAttr.mPath);
	DDX_Text(pDX, IDC_EDIT_PARAM, param.mNormalAttr.mParam);
	DDX_Text(pDX, IDC_EDIT_HOTKEY2, in->mHotKey);
	DDX_Check(pDX, IDC_CHECK_SHOWARGINPUT, param.mIsShowArgDialog);
	DDX_Check(pDX, IDC_CHECK_USEDESCRIPTIONFORMATCHING, param.mIsUseDescriptionForMatching);
	DDX_Control(pDX, IDC_BUTTON_MENU, in->mPathMenuBtn);
	// 以下のパラメータは通常と引数なし版で兼用
	DDX_CBIndex(pDX, IDC_COMBO_SHOWTYPE, param.mNormalAttr.mShowType);
	DDX_Text(pDX, IDC_EDIT_DIR, param.mNormalAttr.mDir);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CommandEditDialog, SettingPage)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_PATH, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_PARAM, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_COMMAND(IDC_BUTTON_RESOLVESHORTCUT, OnButtonResolveShortcut)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_APP + 11, OnUserMessageIconChanged)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
	ON_BN_CLICKED(IDC_BUTTON_MENU, OnPathMenuBtnClicked)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR3, OnButtonBrowseDir3Clicked)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL CommandEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	GetDlgItem(IDC_BUTTON_BROWSEDIR3)->SetWindowTextW(L"\U0001F4C2");

	in->mMenuForPathBtn.CreatePopupMenu();
	in->mMenuForPathBtn.InsertMenu((UINT)-1, 0, 1, _T("ファイル選択"));
	in->mMenuForPathBtn.InsertMenu((UINT)-1, 0, 2, _T("フォルダ選択"));
	in->mPathMenuBtn.m_hMenu = (HMENU)in->mMenuForPathBtn;

	in->mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);
	in->mIconLabelPtr->EnableIconChange();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool CommandEditDialog::UpdateStatus()
{
	auto& param = in->mParam;

	in->mHotKey = param.mHotKeyAttr.ToString();
	if (in->mHotKey.IsEmpty()) {
		in->mHotKey.LoadString(IDS_NOHOTKEY);
	}

	CString targetPath = param.mNormalAttr.mPath;
	ExpandMacros(targetPath);

	// 相対パスの場合はパス解決をする
	if (PathIsRelative(targetPath)) {
		LocalPathResolver resolver;
		CString resolvedPath;
		if (resolver.Resolve(targetPath, resolvedPath)) {
			targetPath = resolvedPath;
		}
	}

	BOOL isShortcut = CString(_T(".lnk")).CompareNoCase(PathFindExtension(targetPath)) == 0;

	// $1,2,3... または $*の指定がある場合は、引数必須を選択するチェックを表示
	const tregex& regArg = GetRegexForArgument();

	bool hasArg = std::regex_search((LPCTSTR)targetPath, regArg) ||
		            std::regex_search((LPCTSTR)param.mNormalAttr.mParam, regArg);

	GetDlgItem(IDC_CHECK_SHOWARGINPUT)->ShowWindow(hasArg? SW_SHOW : SW_HIDE);

	// パスが有効なファイルだったら編集メニューを表示する
	in->mMenuForPathBtn.DeleteMenu(3, MF_BYCOMMAND);
	if (IsEditableFileType(targetPath)) {
			in->mMenuForPathBtn.InsertMenu((UINT)-1, 0, 3, _T("編集"));
	}

	// .lnkだったらショートカット解決ボタンを表示する
	GetDlgItem(IDC_BUTTON_RESOLVESHORTCUT)->ShowWindow(isShortcut? SW_SHOW : SW_HIDE);

	// .exe/.batでなければ、管理者権限実行を無効化する
	BOOL isExecutable = CString(_T(".exe")).CompareNoCase(PathFindExtension(targetPath)) == 0 ||
	                    CString(_T(".bat")).CompareNoCase(PathFindExtension(targetPath)) == 0;
	GetDlgItem(IDC_CHECK_RUNASADMIN)->EnableWindow(isExecutable);
	if (isExecutable == FALSE) {
		param.mIsRunAsAdmin = FALSE;
	}

	if (param.mIconData.empty()) {
		in->mIcon = IconLoader::Get()->LoadIconFromPath(targetPath);
	}

	if (in->mIcon) {
		in->mIconLabelPtr->DrawIcon(in->mIcon);
	}

	
	// 名前チェック
	bool isNameValid =
	 	launcherapp::commands::common::IsValidCommandName(param.mName, in->mOrgName, in->mMessage);
	if (isNameValid == false) {
		DisalbleOKButton();
		return false;
	}

	// パスの妥当性チェック
	if (targetPath.IsEmpty()) {
		// パスが空
		in->mMessage.LoadString(IDS_ERR_PATHISEMPTY);
		DisalbleOKButton();
		return false;
	}
	if (PathIsURL(targetPath) == FALSE && Path::FileExists(targetPath) == false) {
		in->mMessage = _T("ファイルまたはディレクトリが存在しません。");
		DisalbleOKButton();
		return false;
	}

	in->mMessage.Empty();
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

void CommandEditDialog::OpenTarget()
{
	CString path(in->mParam.mNormalAttr.mPath);
	ExpandMacros(path);

	auto p = path.GetBuffer(path.GetLength() + 1);

	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_NORMAL;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = p;

	TCHAR verb[24];
	_tcscpy_s(verb, _T("edit"));
	si.lpVerb = verb;

	ShellExecuteEx(&si);

	path.ReleaseBuffer();

	CloseHandle(si.hProcess);
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
	auto& param = in->mParam;

	param = *(CommandParam*)GetParam();

	CString caption;
  GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

	caption += suffix;
	SetWindowText(caption);

	if (param.mIconData.empty()) {
		CString resolvedPath(param.mNormalAttr.mPath);
		ExpandMacros(resolvedPath);
		in->mIcon = IconLoader::Get()->LoadIconFromPath(resolvedPath);
	}
	else {
		in->mIcon = IconLoader::Get()->LoadIconFromStream(param.mIconData);
	}

}

bool CommandEditDialog::GetHelpPageId(CString& id)
{
	id = _T("ShellExecute_Basic");
	return true;
}



void CommandEditDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	auto& paramSrc = in->mParam;

	// Note : このページで設定するパラメータだけ書き戻す
	auto param = (CommandParam*)GetParam();

	param->mName = paramSrc.mName;
	param->mDescription = paramSrc.mDescription;

	param->mNormalAttr.mPath = paramSrc.mNormalAttr.mPath;
	param->mNormalAttr.mParam = paramSrc.mNormalAttr.mParam;

	param->mIsRunAsAdmin = paramSrc.mIsRunAsAdmin;
	param->mIsShowArgDialog = paramSrc.mIsShowArgDialog;
	param->mIsUseDescriptionForMatching = paramSrc.mIsUseDescriptionForMatching;
	param->mIconData = paramSrc.mIconData;
	param->mHotKeyAttr = paramSrc.mHotKeyAttr;

	// 以下、兼用パラメータ
	param->mNormalAttr.mShowType = paramSrc.mNormalAttr.mShowType;
	param->mNoParamAttr.mShowType = paramSrc.mNormalAttr.mShowType;
	param->mNormalAttr.mDir = paramSrc.mNormalAttr.mDir;
	param->mNoParamAttr.mDir = paramSrc.mNormalAttr.mDir;

	// 引数展開のキーワードが含まれていない場合は、引数入力ダイアログの表示を無効化する
	const tregex& regArg = GetRegexForArgument();
	bool hasArg = std::regex_search((LPCTSTR)paramSrc.mNormalAttr.mPath, regArg) ||
		          std::regex_search((LPCTSTR)paramSrc.mNormalAttr.mParam, regArg);
	if (hasArg == false) {
		param->mIsShowArgDialog = false;
	}

	__super::OnOK();
}


void CommandEditDialog::OnButtonHotKey()
{
	UpdateData();

	if (CommandHotKeyDialog::ShowDialog(in->mParam.mName, in->mParam.mHotKeyAttr, this) == false) {
		return ;
	}
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

// (テキストエディタなどで)編集するようなファイルタイプか?
bool CommandEditDialog::IsEditableFileType(CString pathStr)
{
	ExpandMacros(pathStr);
	Path pathObj(pathStr);
	if (pathObj.FileExists() == FALSE || pathObj.IsDirectory()) {
		return false;
	}

	CString ext(pathObj.FindExtension());

	// 実行ファイルはバイナリ形式なので普通は編集しない
	if (ext.CompareNoCase(_T(".exe")) == 0) {
		return false;
	}

	// それ以外は編集する可能性がある
	return true;
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

		in->mIcon = IconLoader::Get()->LoadIconFromStream(param.mIconData);
	}
	else {
		// デフォルトに戻す
		CString resolvedPath(param.mNormalAttr.mPath);
		ExpandMacros(resolvedPath);
		in->mIcon = IconLoader::Get()->LoadIconFromPath(resolvedPath);
		param.mIconData.clear();
	}

	// 再描画
	if (in->mIcon) {
		in->mIconLabelPtr->DrawIcon(in->mIcon);
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

void CommandEditDialog::OnPathMenuBtnClicked()
{
	switch (in->mPathMenuBtn.m_nMenuResult) {
		case 1:
			OnButtonBrowseFile1Clicked();
			break;
		case 2:
			OnButtonBrowseDir1Clicked();
			break;
		case 3:
			OpenTarget();
			break;
	}
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



