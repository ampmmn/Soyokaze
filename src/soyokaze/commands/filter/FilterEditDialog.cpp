#include "pch.h"
#include "framework.h"
#include "FilterEditDialog.h"
#include "commands/filter/PreFilterSubProcessDialog.h"
#include "commands/filter/PreFilterConstantDialog.h"
#include "commands/filter/AfterSubProcessDialog.h"
#include "commands/filter/AfterCommandDialog.h"
#include "commands/filter/AfterCopyDialog.h"
#include "gui/FolderDialog.h"
#include "icon/IconLabel.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "commands/common/CommandEditValidation.h"
#include "commands/common/ExpandFunctions.h"
#include "utility/Accessibility.h"
#include "icon/IconLoader.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace filter {

constexpr int ID_PREFILTER_SUBPROCESS = FILTER_SUBPROCESS + 1;
constexpr int ID_PREFILTER_CLIPBOARD = FILTER_CLIPBOARD + 1;
constexpr int ID_PREFILTER_CONSTANT = FILTER_CONSTANT + 1;

constexpr int ID_POSTFILTER_COMMAND = POSTFILTER_COMMAND + 1;
constexpr int ID_POSTFILTER_SUBPROCESS = POSTFILTER_SUBPROCESS + 1;
constexpr int ID_POSTFILTER_CLIPBOARD = POSTFILTER_CLIPBOARD + 1;

FilterEditDialog::FilterEditDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_FILTEREDIT, parentWnd),
	mIconLabelPtr(std::make_unique<IconLabel>())
{
	SetHelpPageId(_T("FilterEdit"));
}

FilterEditDialog::~FilterEditDialog()
{
}

void FilterEditDialog::SetName(const CString& name)
{
	mParam.mName = name;
}

void FilterEditDialog::SetOriginalName(const CString& name)
{
	mOrgName = name;
}

void FilterEditDialog::SetParam(const CommandParam& param)
{
	mParam = param;
}

const CommandParam& FilterEditDialog::GetParam()
{
	return mParam;
}

void FilterEditDialog::ResetHotKey()
{
	mParam.mHotKeyAttr.Reset();
}

void FilterEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mParam.mDescription);
	DDX_Control(pDX, IDC_BUTTON_TYPE1, mPathMenuType1);
	DDX_Text(pDX, IDC_STATIC_PREFILTERDETAIL, mPreFilterDetail);
	DDX_Control(pDX, IDC_BUTTON_TYPE2, mPathMenuType2);
	DDX_Text(pDX, IDC_STATIC_AFTERDETAIL, mAfterDetail);
	DDX_Text(pDX, IDC_EDIT_HOTKEY2, mHotKey);
	DDX_Check(pDX, IDC_CHECK_DISPLAYNAME, mParam.mIsReplaceText);
	DDX_Text(pDX, IDC_EDIT_REGPATTERN, mParam.mReplacePattern);
	DDX_Text(pDX, IDC_EDIT_REPLACE, mParam.mReplaceText);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(FilterEditDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_BN_CLICKED(IDC_BUTTON_TYPE1, OnType1MenuBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON_TYPE2, OnType2MenuBtnClicked)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_CHECK_DISPLAYNAME, OnUpdateStatus)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL FilterEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	// 候補生成方法の選択肢
	mMenuForType1Btn.CreatePopupMenu();
	mMenuForType1Btn.InsertMenu((UINT)-1, 0, ID_PREFILTER_SUBPROCESS, _T("プログラムを実行する"));
	mMenuForType1Btn.InsertMenu((UINT)-1, 0, ID_PREFILTER_CLIPBOARD, _T("クリップボードの内容を取得する"));
	mMenuForType1Btn.InsertMenu((UINT)-1, 0, ID_PREFILTER_CONSTANT, _T("候補を定義する"));
	mPathMenuType1.m_hMenu = (HMENU)mMenuForType1Btn;

	// 後段の処理の選択肢
	mMenuForType2Btn.CreatePopupMenu();
	mMenuForType2Btn.InsertMenu((UINT)-1, 0, ID_POSTFILTER_COMMAND, _T("他のコマンドを実行する"));
	mMenuForType2Btn.InsertMenu((UINT)-1, 0, ID_POSTFILTER_SUBPROCESS, _T("プログラムを実行する"));
	mMenuForType2Btn.InsertMenu((UINT)-1, 0, ID_POSTFILTER_CLIPBOARD, _T("クリップボードにコピーする"));
	mPathMenuType2.m_hMenu = (HMENU)mMenuForType2Btn;

	SetIcon(IconLoader::Get()->LoadDefaultIcon(), FALSE);

	mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);

	CString caption;
  GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)mOrgName);

	caption += suffix;
	SetWindowText(caption);

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool FilterEditDialog::UpdateStatus()
{
	mHotKey = mParam.mHotKeyAttr.ToString();
	if (mHotKey.IsEmpty()) {
		mHotKey.LoadString(IDS_NOHOTKEY);
	}

	bool isOK = true;

	// 前段の処理の設定値
	if (mParam.mPreFilterType == FILTER_SUBPROCESS) {

		if (mParam.mPath.IsEmpty()) {
			mPathMenuType1.SetWindowText(_T("(生成方法を選択)"));
			mPreFilterDetail.Empty();
			mMessage = _T("候補の生成方法を設定してください");
			isOK = false;

		}
		else {
			mPathMenuType1.SetWindowText(_T("プログラムを実行する"));
			mPreFilterDetail.Format(_T("ファイルパス:%s\nパラメータ:%s\n作業フォルダ:%s"),
					(LPCTSTR)mParam.mPath, (LPCTSTR)mParam.mParameter, (LPCTSTR)mParam.mDir);
		}
	}
	else if (mParam.mPreFilterType == FILTER_CLIPBOARD) {
		mPathMenuType1.SetWindowText(_T("クリップボードの内容を使用する"));
		mPreFilterDetail = _T("クリップボードのテキストを行単位で分割して候補として表示します");
	}
	else if (mParam.mPreFilterType == FILTER_CONSTANT) {
		mPathMenuType1.SetWindowText(_T("候補を定義する"));
		mPreFilterDetail.Format(_T("候補の一覧:\n%s"), (LPCTSTR)mParam.mPreFilterText);
	}
	
	// 後段の処理の設定値
	if (mParam.mPostFilterType == POSTFILTER_COMMAND) {
		mPathMenuType2.SetWindowText(_T("他のコマンドを実行する"));
		mAfterDetail.Format(_T("コマンド:%s\nパラメータ:%s"),
					(LPCTSTR)mParam.mAfterCommandName, (LPCTSTR)mParam.mAfterCommandParam);
	}
	else if (mParam.mPostFilterType == POSTFILTER_SUBPROCESS) {
		mPathMenuType2.SetWindowText(_T("プログラムを実行する"));
		mAfterDetail.Format(_T("ファイルパス:%s\nパラメータ:%s\n作業フォルダ:%s"),
					(LPCTSTR)mParam.mAfterFilePath, (LPCTSTR)mParam.mAfterCommandParam, (LPCTSTR)mParam.mAfterDir);
	}
	else {
		mPathMenuType2.SetWindowText(_T("クリップボードにコピーする"));
		mAfterDetail.Format(_T("パラメータ:%s"), (LPCTSTR)mParam.mAfterCommandParam);
	}

	// 表示文字列の置換に関する設定
	GetDlgItem(IDC_EDIT_REGPATTERN)->EnableWindow(mParam.mIsReplaceText);
	GetDlgItem(IDC_EDIT_REPLACE)->EnableWindow(mParam.mIsReplaceText);

	if (mParam.mIsReplaceText) {
		CString msg;
		if (mParam.BuildCandidateTextRegExp(msg) == false) {
			AfxMessageBox(msg);
			mMessage = _T("検索パターンの内容が不正です");
			isOK = false;
		}
	}

	// 名前チェック
	bool isNameValid =
	 	launcherapp::commands::common::IsValidCommandName(mParam.mName, mOrgName, mMessage);
	if (isNameValid == false) {
		isOK = false;
	}

	mIconLabelPtr->DrawIcon(IconLoader::Get()->LoadPromptIcon());

	if (isOK == false) {
			GetDlgItem(IDOK)->EnableWindow(FALSE);
			return false;
	}

	if (mParam.mPreFilterType == 0 && mParam.mPath.IsEmpty()) {
		mMessage.LoadString(IDS_ERR_PATHISEMPTY);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	if (mParam.mPostFilterType == 0 && mParam.mAfterCommandName.IsEmpty()) {
		mMessage = _T("絞込み後に実行するコマンドを選んでください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}
	if (mParam.mPostFilterType == 1 && mParam.mAfterFilePath.IsEmpty()) {
		mMessage = _T("絞込み後に実行するファイルまたはURLを入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	CString workDir = mParam.mDir;
	ExpandMacros(workDir);

	if (workDir.IsEmpty() == FALSE && PathIsDirectory(workDir) == FALSE) {
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
	__super::OnOK();
}


void FilterEditDialog::OnButtonHotKey()	
{
	UpdateData();

	if (CommandHotKeyDialog::ShowDialog(mParam.mName, mParam.mHotKeyAttr, this) == false) {
		return ;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

void FilterEditDialog::OnType1MenuBtnClicked()
{
	int action = mPathMenuType1.m_nMenuResult;

	// ボタン部分が単に押された場合、mMenuResultに0が格納される
	if (action == 0) {
		action = mParam.mPreFilterType + 1;
	}

	switch (action) {
		case ID_PREFILTER_SUBPROCESS:
			OnSelectSubProcessFilter();
			break;
		case ID_PREFILTER_CLIPBOARD:
			OnSelectClipboardFilter();
			break;
		case ID_PREFILTER_CONSTANT:
			OnSelectConstantFilter();
			break;
	}
}

void FilterEditDialog::OnSelectSubProcessFilter()
{
	UpdateData();

	PreFilterSubProcessDialog dlg(this);
	dlg.SetParam(mParam);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam = dlg.GetParam();

	UpdateStatus();
	UpdateData(FALSE);
}

void FilterEditDialog::OnSelectClipboardFilter()
{
	UpdateData();

	mParam.mPreFilterType = FILTER_CLIPBOARD;

	UpdateStatus();
	UpdateData(FALSE);
}

void FilterEditDialog::OnSelectConstantFilter()
{
	UpdateData();

	PreFilterConstantDialog dlg(this);
	dlg.SetParam(mParam);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam = dlg.GetParam();

	UpdateStatus();
	UpdateData(FALSE);
}

void FilterEditDialog::OnType2MenuBtnClicked()
{
	int action = mPathMenuType2.m_nMenuResult;

	// ボタン部分が単に押された場合、mMenuResultに0が格納される
	if (action == 0) {
		action = mParam.mPostFilterType + 1;
	}

	switch (action) {
		case ID_POSTFILTER_COMMAND:
			// 他のコマンドを実行する
			OnSelectAfterExecOtherCommand();
			break;
		case ID_POSTFILTER_SUBPROCESS:
			// プログラムを実行する
			OnSelectAfterSubProcess();
			break;
		case ID_POSTFILTER_CLIPBOARD:
			// クリップボードにコピーする
			OnSelectAfterCopyClipboard();
			break;
	}
}

void FilterEditDialog::OnSelectAfterExecOtherCommand()
{
	AfterCommandDialog dlg(this);
	dlg.SetParam(mParam);
	if (dlg.DoModal() != IDOK) {
		return ;
	}
	mParam = dlg.GetParam();

	UpdateStatus();
	UpdateData(FALSE);
}

void FilterEditDialog::OnSelectAfterSubProcess()
{
	AfterSubProcessDialog dlg(this);
	dlg.SetParam(mParam);
	if (dlg.DoModal() != IDOK) {
		return ;
	}
	mParam = dlg.GetParam();

	UpdateStatus();
	UpdateData(FALSE);
}

void FilterEditDialog::OnSelectAfterCopyClipboard()
{
	AfterCopyDialog dlg(this);
	dlg.SetParam(mParam);
	if (dlg.DoModal() != IDOK) {
		return ;
	}
	mParam = dlg.GetParam();

	UpdateStatus();
	UpdateData(FALSE);
}

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

