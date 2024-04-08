#include "pch.h"
#include "framework.h"
#include "FilterEditDialog.h"
#include "gui/FolderDialog.h"
#include "icon/IconLabel.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "commands/core/CommandRepository.h"
#include "utility/Accessibility.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = soyokaze::core::CommandRepository;
using Command = soyokaze::core::Command;

namespace soyokaze {
namespace commands {
namespace filter {



FilterEditDialog::FilterEditDialog() : 
	CDialogEx(IDD_FILTEREDIT),
	mIconLabelPtr(std::make_unique<IconLabel>()),
	mIsGlobal(false),
	mCommandSelIndex(-1)
{
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

	if (param.mShowType == SW_HIDE) {
		mShowTypeIdx = 3;
	}
	else if (param.mShowType == SW_SHOWMINIMIZED) {
		mShowTypeIdx = 2;
	}
	else if (param.mShowType== SW_MAXIMIZE) {
		mShowTypeIdx = 1;
	}
	else {
		mShowTypeIdx = 0;
	}
}

void FilterEditDialog::GetParam(CommandParam& param)
{
	if (mShowTypeIdx == 1) {
		mParam.mShowType = SW_MAXIMIZE;
	}
	else if (mShowTypeIdx == 2) {
		mParam.mShowType =  SW_SHOWMINIMIZED;
	}
	else if (mShowTypeIdx == 3) {
		mParam.mShowType =  SW_HIDE;
	}
	else {
		mParam.mShowType =  SW_NORMAL;
	}

	param = mParam;
}

void FilterEditDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mParam.mDescription);
	DDX_CBIndex(pDX, IDC_COMBO_SHOWTYPE, mShowTypeIdx);
	DDX_Text(pDX, IDC_EDIT_PATH, mParam.mPath);
	DDX_Text(pDX, IDC_EDIT_PARAM, mParam.mParameter);
	DDX_Text(pDX, IDC_EDIT_DIR, mParam.mDir);
	DDX_Text(pDX, IDC_EDIT_HOTKEY2, mHotKey);
	DDX_CBIndex(pDX, IDC_COMBO_AFTERCOMMAND, mCommandSelIndex);
	DDX_Text(pDX, IDC_EDIT_PARAM2, mParam.mAfterCommandParam);
	DDX_CBIndex(pDX, IDC_COMBO_PREFILTERTYPE, mParam.mPreFilterType);
	DDX_CBIndex(pDX, IDC_COMBO_AFTERTYPE, mParam.mPostFilterType);
	DDX_Text(pDX, IDC_EDIT_PATH2, mParam.mAfterFilePath);
}

BEGIN_MESSAGE_MAP(FilterEditDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnEditNameChanged)
	ON_EN_CHANGE(IDC_EDIT_PATH, OnEditPathChanged)
	ON_EN_CHANGE(IDC_EDIT_PATH2, OnEditPathChanged)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE1, OnButtonBrowseFile1Clicked)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR3, OnButtonBrowseDir3Clicked)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_CBN_SELCHANGE(IDC_COMBO_PREFILTERTYPE, OnCbnAfterTypeChanged)
	ON_CBN_SELCHANGE(IDC_COMBO_AFTERTYPE, OnCbnAfterTypeChanged)
	ON_CBN_SELCHANGE(IDC_COMBO_AFTERCOMMAND, OnCbnAfterCommandChanged)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_BROWSEFILE3, OnButtonBrowseAfterCommandFile)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR4, OnButtonBrowseAfterCommandDir)
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
	GetDlgItem(IDC_STATIC_PARAMHELP)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_STATIC_WORKDIR)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_EDIT_DIR)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_BUTTON_BROWSEDIR3)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_STATIC_SHOWTYPE)->ShowWindow(showTypePreFilter);
	GetDlgItem(IDC_COMBO_SHOWTYPE)->ShowWindow(showTypePreFilter);

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

	if (mParam.mName.IsEmpty()) {
		mMessage.LoadString(IDS_ERR_NAMEISEMPTY);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();

	// 重複チェック
	if (mParam.mName.CompareNoCase(mOrgName) != 0) {
		auto cmd = cmdRepoPtr->QueryAsWholeMatch(mParam.mName, false);
		if (cmd != nullptr) {
			cmd->Release();
			mMessage.LoadString(IDS_ERR_NAMEALREADYEXISTS);
			GetDlgItem(IDOK)->EnableWindow(FALSE);
			return false;
		}
	}

	// 使えない文字チェック
	if (cmdRepoPtr->IsValidAsName(mParam.mName) == false) {
		mMessage.LoadString(IDS_ERR_ILLEGALCHARCONTAINS);
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

	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	return true;
}

void FilterEditDialog::OnEditNameChanged()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void FilterEditDialog::OnEditPathChanged()
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

	CommandHotKeyDialog dlg(mHotKeyAttr, mIsGlobal);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttr);
	mIsGlobal = dlg.IsGlobal();

	UpdateStatus();
	UpdateData(FALSE);
}

void FilterEditDialog::OnCbnAfterTypeChanged()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void FilterEditDialog::OnCbnAfterCommandChanged()
{
	UpdateData();
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

} // end of namespace filter
} // end of namespace commands
} // end of namespace soyokaze

