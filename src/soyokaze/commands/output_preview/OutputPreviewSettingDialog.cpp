#include "pch.h"
#include "OutputPreviewSettingDialog.h"
#include "commands/validation/CommandEditValidation.h"
#include "icon/IconLabel.h"
#include "icon/IconLoader.h"
#include "utility/Path.h"
#include "resource.h"
#include <shlobj.h>

namespace launcherapp {
namespace commands {
namespace output_preview {

struct OutputPreviewSettingDialog::PImpl
{
	OutputPreviewParam mParam;
	CString mOriginalName;
	CString mMessage;
	CString mTimeoutText;
	IconLabel mIconLabel;
};

OutputPreviewSettingDialog::OutputPreviewSettingDialog(CWnd* parentWnd) :
	launcherapp::control::SinglePageDialog(IDD_COMMAND_OUTPUTPREVIEW, parentWnd),
	in(std::make_unique<PImpl>())
{
	SetHelpPageId("OutputPreviewSetting");
}

OutputPreviewSettingDialog::~OutputPreviewSettingDialog()
{
}

void OutputPreviewSettingDialog::SetParam(const OutputPreviewParam& param)
{
	in->mParam = param;
	in->mTimeoutText.Format(_T("%d"), param.mTimeout);
}

const OutputPreviewParam& OutputPreviewSettingDialog::GetParam() const
{
	return in->mParam;
}

void OutputPreviewSettingDialog::SetOriginalName(const CString& name)
{
	in->mOriginalName = name;
}

void OutputPreviewSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, in->mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, in->mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_FILEPATH, in->mParam.mFilePath);
	DDX_Text(pDX, IDC_EDIT_PARAM, in->mParam.mParameter);
	DDX_Text(pDX, IDC_EDIT_DIR, in->mParam.mWorkDir);
	DDX_Text(pDX, IDC_EDIT_EXTENSION, in->mParam.mExtension);
	DDX_CBIndex(pDX, IDC_COMBO_SHOWTYPE, in->mParam.mShowType);
	DDX_CBIndex(pDX, IDC_COMBO_RESULTTYPE, in->mParam.mResultType);
	DDX_CBIndex(pDX, IDC_COMBO_PREVIEWTYPE, in->mParam.mPreviewType);
	DDX_Text(pDX, IDC_EDIT_TIMEOUT, in->mTimeoutText);
	DDX_Check(pDX, IDC_CHECK_FORCE, in->mParam.mForce);
	DDX_Control(pDX, IDC_STATIC_ICON, in->mIconLabel);
}

BEGIN_MESSAGE_MAP(OutputPreviewSettingDialog, launcherapp::control::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_FILEPATH, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_PARAM, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_DIR, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_TIMEOUT, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_RESULTTYPE, OnUpdateStatus)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnBrowseFile)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEDIR, OnBrowseWorkDir)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL OutputPreviewSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	GetDlgItem(IDC_BUTTON_BROWSEDIR)->SetWindowTextW(L"\U0001F4C2");

	CString caption(_T("実行結果確認コマンドの設定"));
	CString suffix;
	suffix.Format(_T("【%s】"), in->mOriginalName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOriginalName);
	caption += suffix;
	SetWindowText(caption);

	UpdateData(FALSE);
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

/**
  入力値を検証し、エラー表示とOKボタンの状態を更新する
*/
void OutputPreviewSettingDialog::UpdateStatus()
{
	bool canPressOK = true;
	in->mMessage.Empty();

	if (launcherapp::commands::validation::IsValidCommandName(
		in->mParam.mName, in->mOriginalName, in->mMessage) == false) {
		canPressOK = false;
	}
	else if (in->mParam.mFilePath.IsEmpty() || Path::FileExists(in->mParam.mFilePath) == FALSE ||
		Path::IsDirectory(in->mParam.mFilePath)) {
		in->mMessage = _T("実行するファイルを指定してください");
		canPressOK = false;
	}
	else if (in->mParam.mWorkDir.IsEmpty() == FALSE && Path::IsDirectory(in->mParam.mWorkDir) == FALSE) {
		in->mMessage = _T("存在する作業フォルダを指定してください");
		canPressOK = false;
	}
	else if (in->mParam.mResultType == 1 && in->mParam.mParameter.Find(_T("$output")) < 0) {
		in->mMessage = _T("\"output のファイル\"を選択した場合は パラメータとして $output を指定してください");
		canPressOK = false;
	}
	else {
		int timeout = _ttoi(in->mTimeoutText);
		if (timeout <= 1 || timeout > 60000) {
			in->mMessage = _T("タイムアウトは2～60000ミリ秒で指定してください");
			canPressOK = false;
		}
	}

	GetDlgItem(IDOK)->EnableWindow(canPressOK ? TRUE : FALSE);
	GetDlgItem(IDC_EDIT_EXTENSION)->EnableWindow(in->mParam.mResultType == 1 ? TRUE : FALSE);
	HICON icon = nullptr;
	if (Path::FileExists(in->mParam.mFilePath) && Path::IsDirectory(in->mParam.mFilePath) == FALSE) {
		icon = IconLoader::Get()->LoadIconFromPath(in->mParam.mFilePath);
	}
	if (icon) {
		in->mIconLabel.DrawIcon(icon);
	}
	else {
		in->mIconLabel.DrawIcon(IconLoader::Get()->LoadPromptIcon());
	}
}

void OutputPreviewSettingDialog::OnUpdateStatus()
{
	if (UpdateData(TRUE) == FALSE) {
		return;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

void OutputPreviewSettingDialog::OnBrowseFile()
{
	CFileDialog dialog(TRUE, nullptr, nullptr,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
		_T("すべてのファイル (*.*)|*.*||"), this);
	if (dialog.DoModal() != IDOK) {
		return;
	}
	in->mParam.mFilePath = dialog.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void OutputPreviewSettingDialog::OnBrowseWorkDir()
{
	BROWSEINFO browseInfo{};
	browseInfo.hwndOwner = GetSafeHwnd();
	browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	browseInfo.lpszTitle = _T("作業フォルダを選択してください");
	LPITEMIDLIST itemId = SHBrowseForFolder(&browseInfo);
	if (itemId == nullptr) {
		return;
	}

	TCHAR path[MAX_PATH_NTFS]{};
	if (SHGetPathFromIDList(itemId, path)) {
		in->mParam.mWorkDir = path;
	}
	CoTaskMemFree(itemId);
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH OutputPreviewSettingDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		pDC->SetTextColor(in->mMessage.IsEmpty() ? RGB(0, 0, 0) : RGB(255, 0, 0));
	}
	return brush;
}

void OutputPreviewSettingDialog::OnOK()
{
	if (UpdateData(TRUE) == FALSE) {
		return;
	}
	UpdateStatus();
	if (GetDlgItem(IDOK)->IsWindowEnabled() == FALSE) {
		UpdateData(FALSE);
		return;
	}
	in->mParam.mTimeout = _ttoi(in->mTimeoutText);
	__super::OnOK();
}

}
}
}
