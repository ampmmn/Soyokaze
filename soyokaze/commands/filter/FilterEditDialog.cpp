#include "pch.h"
#include "framework.h"
#include "FilterEditDialog.h"
#include "gui/FolderDialog.h"
#include "icon/IconLabel.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/CommandEditValidation.h"
#include "utility/Accessibility.h"
#include "icon/IconLoader.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;
using Command = launcherapp::core::Command;

namespace launcherapp {
namespace commands {
namespace filter {



FilterEditDialog::FilterEditDialog() : 
	launcherapp::gui::SinglePageDialog(IDD_FILTEREDIT),
	mIconLabelPtr(std::make_unique<IconLabel>()),
	mCommandSelIndex(-1)
{
	SetHelpPageId(_T("FilterEdit"));
}

FilterEditDialog::~FilterEditDialog()
{
}

void FilterEditDialog::SetOrgName(const CString& name)
{
	mOrgName = name;
}

void FilterEditDialog::SetParam(const CommandParam& param)
{
	mParam = param;
}

void FilterEditDialog::GetParam(CommandParam& param)
{
	param = mParam;
}

void FilterEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_PATH, mParam.mPath);
	DDX_Text(pDX, IDC_EDIT_PARAM, mParam.mParameter);
	DDX_Text(pDX, IDC_EDIT_DIR, mParam.mDir);
	DDX_Text(pDX, IDC_EDIT_HOTKEY2, mHotKey);
	DDX_CBIndex(pDX, IDC_COMBO_AFTERCOMMAND, mCommandSelIndex);
	DDX_CBIndex(pDX, IDC_COMBO_CANDIDATECACHE, mParam.mCacheType);
	DDX_Text(pDX, IDC_EDIT_PARAM2, mParam.mAfterCommandParam);
	DDX_CBIndex(pDX, IDC_COMBO_PREFILTERTYPE, mParam.mPreFilterType);
	DDX_CBIndex(pDX, IDC_COMBO_AFTERTYPE, mParam.mPostFilterType);
	DDX_Text(pDX, IDC_EDIT_PATH2, mParam.mAfterFilePath);
}

BEGIN_MESSAGE_MAP(FilterEditDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_PATH, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_PATH2, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_DIR, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE1, OnButtonBrowseFile1Clicked)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR3, OnButtonBrowseDir3Clicked)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_CBN_SELCHANGE(IDC_COMBO_PREFILTERTYPE, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_AFTERTYPE, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_AFTERCOMMAND, OnUpdateStatus)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_BROWSEFILE3, OnButtonBrowseAfterCommandFile)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR4, OnButtonBrowseAfterCommandDir)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
END_MESSAGE_MAP()


BOOL FilterEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	SetIcon(IconLoader::Get()->LoadDefaultIcon(), FALSE);

	mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);

	// 後段のコマンド設定 排他の項目の位置を調整する
	Overlap(GetDlgItem(IDC_STATIC_AFTERCOMMAND), GetDlgItem(IDC_STATIC_PATH2));
	Overlap(GetDlgItem(IDC_COMBO_AFTERCOMMAND), GetDlgItem(IDC_EDIT_PATH2));
	GetDlgItem(IDC_BUTTON_BROWSEFILE3)->SetWindowTextW(L"\U0001F4C4");
	GetDlgItem(IDC_BUTTON_BROWSEDIR4)->SetWindowTextW(L"\U0001F4C2");

	CString caption;
  GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)mOrgName);

	caption += suffix;
	SetWindowText(caption);

	// File&Folder Select Button
	GetDlgItem(IDC_BUTTON_BROWSEFILE1)->SetWindowTextW(L"\U0001F4C4");
	GetDlgItem(IDC_BUTTON_BROWSEDIR3)->SetWindowTextW(L"\U0001F4C2");

	// コマンド一覧のコンボボックス
	std::vector<Command*> commands;
	auto cmdRepo = CommandRepository::GetInstance();
	cmdRepo->EnumCommands(commands);

	CComboBox* commandComboBox =
	 	(CComboBox*)GetDlgItem(IDC_COMBO_AFTERCOMMAND);
	ASSERT(commandComboBox);

	for (auto& cmd : commands) {
		CString name = cmd->GetName();
		int idx = commandComboBox->AddString(name);
		cmd->Release();

		if (name == mParam.mAfterCommandName) {
			mCommandSelIndex = idx;
		}
	}

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool FilterEditDialog::UpdateStatus()
{
	mHotKey = mHotKeyAttr.ToString();
	if (mHotKey.IsEmpty()) {
		mHotKey.LoadString(IDS_NOHOTKEY);
	}

	int showTypePreFilter = mParam.mPreFilterType == 0 ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_STATIC_PATH)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_EDIT_PATH)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_BUTTON_BROWSEFILE1)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_STATIC_PARAM)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_EDIT_PARAM)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_STATIC_WORKDIR)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_EDIT_DIR)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_BUTTON_BROWSEDIR3)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_SYSLINK_MACRO)->ShowWindow(showTypePreFilter);

	if (mParam.mPostFilterType == 0) {
		// 他のコマンドを実行する
		GetDlgItem(IDC_STATIC_AFTERCOMMAND)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_COMBO_AFTERCOMMAND)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_PATH2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_PATH2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUTTON_BROWSEFILE3)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUTTON_BROWSEDIR4)->ShowWindow(SW_HIDE);
	}
	else if (mParam.mPostFilterType == 1) {
		// 他のプログラムを実行する
		GetDlgItem(IDC_STATIC_AFTERCOMMAND)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMBO_AFTERCOMMAND)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_PATH2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_PATH2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BUTTON_BROWSEFILE3)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BUTTON_BROWSEDIR4)->ShowWindow(SW_SHOW);
	}
	else {
		// クリップボードコピー
		GetDlgItem(IDC_STATIC_AFTERCOMMAND)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMBO_AFTERCOMMAND)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_PATH2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_PATH2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUTTON_BROWSEFILE3)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUTTON_BROWSEDIR4)->ShowWindow(SW_HIDE);
	}


	if (mParam.mPreFilterType == 0) {
		mIconLabelPtr->DrawIcon(IconLoader::Get()->LoadIconFromPath(mParam.mPath));
	}
	else {
		mIconLabelPtr->DrawIcon(IconLoader::Get()->LoadDefaultIcon());
	}

	// 名前チェック
	bool isNameValid =
	 	launcherapp::commands::common::IsValidCommandName(mParam.mName, mOrgName, mMessage);
	if (isNameValid == false) {
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	//
	if (mParam.mPreFilterType == 0 && mParam.mPath.IsEmpty()) {
		mMessage.LoadString(IDS_ERR_PATHISEMPTY);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	if (mParam.mPostFilterType == 0 && mCommandSelIndex == -1) {
		mMessage = _T("絞込み後に実行するコマンドを選んでください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}
	if (mParam.mPostFilterType == 1 && mParam.mAfterFilePath.IsEmpty()) {
		mMessage = _T("絞込み後に実行するファイルまたはURLを入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}
	if (mParam.mDir.IsEmpty() == FALSE && PathIsDirectory(mParam.mDir) == FALSE) {
		mMessage = _T("作業フォルダは存在しません");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	return true;
}

void FilterEditDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void FilterEditDialog::OnButtonBrowseFile1Clicked()
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

void FilterEditDialog::OnButtonBrowseDir3Clicked()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mParam.mDir, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mDir = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH FilterEditDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}

void FilterEditDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	if (mParam.mPostFilterType == 0) {
		CComboBox* cmbBox = (CComboBox*)GetDlgItem(IDC_COMBO_AFTERCOMMAND);
		cmbBox->GetLBText(mCommandSelIndex, mParam.mAfterCommandName);
	}

	__super::OnOK();
}


void FilterEditDialog::OnButtonHotKey()	
{
	UpdateData();

	if (CommandHotKeyDialog::ShowDialog(mParam.mName, mHotKeyAttr, this) == false) {
		return ;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

void FilterEditDialog::OnButtonBrowseAfterCommandFile()
{
	UpdateData();
	CFileDialog dlg(TRUE, NULL, mParam.mAfterFilePath, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mAfterFilePath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void FilterEditDialog::OnButtonBrowseAfterCommandDir()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mParam.mAfterFilePath, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mAfterFilePath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

bool FilterEditDialog::Overlap(CWnd* dstWnd, CWnd* srcWnd)
{
	ASSERT(dstWnd && srcWnd);

	if (dstWnd->GetParent() != srcWnd->GetParent()) {
		return false;
	}

	CWnd* parentWnd = srcWnd->GetParent();

	CRect rcDst;
	dstWnd->GetClientRect(&rcDst);
	CPoint ptDst = rcDst.TopLeft();
	dstWnd->MapWindowPoints(parentWnd, &ptDst, 1);


	srcWnd->SetWindowPos(NULL, ptDst.x, ptDst.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	return true;
}

// マニュアル表示
void FilterEditDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate(_T("MacroList"));
	*pResult = 0;
}



} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

