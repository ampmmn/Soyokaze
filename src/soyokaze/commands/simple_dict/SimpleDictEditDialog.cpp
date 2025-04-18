#include "pch.h"
#include "SimpleDictEditDialog.h"
#include "commands/simple_dict/SimpleDictParam.h"
#include "commands/simple_dict/ExcelWrapper.h"
#include "commands/simple_dict/SimpleDictPreviewDialog.h"
#include "commands/common/SubProcessDialog.h"
#include "commands/common/OtherCommandDialog.h"
#include "commands/common/CopyToClipboardDialog.h"
#include "gui/FolderDialog.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/CommandEditValidation.h"
#include "utility/Accessibility.h"
#include "utility/Path.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;
using Command =  launcherapp::core::Command;
using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace simple_dict {

constexpr int ID_POSTFILTER_COMMAND = 0;
constexpr int ID_POSTFILTER_SUBPROCESS = 1;
constexpr int ID_POSTFILTER_CLIPBOARD = 2;

struct SettingDialog::PImpl
{
	// 編集開始時のコマンド名
	CString mOrgName;
	// メッセージ欄
	CString mMessage;

	// 編集対象パラメータ
	SimpleDictParam mParam;

	// 後段の処理内容
	CString mAfterDetail;
	// 後段の処理の種別を選択するためのボタン
	CMFCMenuButton mMenuTypeBtn;
	// 後段の処理の種別を選択するためのメニュー
	CMenu mMenuBtn;


	bool mIsTestPassed{false};

	// ホットキー(表示用)
	CString mHotKey;

};


SettingDialog::SettingDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_SIMPLEDICT, parentWnd), in(new PImpl)
{
	SetHelpPageId(_T("SimpleDictEdit"));
	in->mIsTestPassed = false;
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::SetName(const CString& name)
{
	in->mParam.mName = name;
}

void SettingDialog::SetOriginalName(const CString& name)
{
	in->mOrgName = name;
	in->mIsTestPassed = true;
}

void SettingDialog::SetParam(const SimpleDictParam& param)
{
	in->mParam = param;
}

const SimpleDictParam& SettingDialog::GetParam() const
{
	return in->mParam;
}

void SettingDialog::ResetHotKey()
{
	in->mParam.mHotKeyAttr.Reset();
}

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, in->mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, in->mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_FILEPATH, in->mParam.mFilePath);
	DDX_Text(pDX, IDC_EDIT_SHEETNAME, in->mParam.mSheetName);
	DDX_Text(pDX, IDC_EDIT_FRONT, in->mParam.mRangeFront);
	DDX_Text(pDX, IDC_EDIT_BACK, in->mParam.mRangeBack);
	DDX_Text(pDX, IDC_EDIT_VALUE2, in->mParam.mRangeValue2);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTIONFORMAT, in->mParam.mDescriptionFormat);
	DDX_Text(pDX, IDC_EDIT_NAMEFORMAT, in->mParam.mNameFormat);
	DDX_Check(pDX, IDC_CHECK_FIRSTROWISHEADER, in->mParam.mIsFirstRowHeader);
	DDX_Check(pDX, IDC_CHECK_MATCHWITHOUTKEYWORD, in->mParam.mIsMatchWithoutKeyword);
	DDX_Check(pDX, IDC_CHECK_REVERSE, in->mParam.mIsEnableReverse);
	DDX_Check(pDX, IDC_CHECK_NOTIFYUPDATE, in->mParam.mIsNotifyUpdate);
	DDX_Check(pDX, IDC_CHECK_EXPANDMACRO, in->mParam.mIsExpandMacro);
	DDX_Text(pDX, IDC_EDIT_HOTKEY2, in->mHotKey);
	DDX_Control(pDX, IDC_BUTTON_TYPE, in->mMenuTypeBtn);
	DDX_Text(pDX, IDC_STATIC_AFTERDETAIL, in->mAfterDetail);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(SettingDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateName)
	ON_EN_CHANGE(IDC_EDIT_FILEPATH, OnUpdateCondition)
	ON_EN_CHANGE(IDC_EDIT_SHEETNAME, OnUpdateCondition)
	ON_EN_CHANGE(IDC_EDIT_FRONT, OnUpdateCondition)
	ON_EN_CHANGE(IDC_EDIT_BACK, OnUpdateCondition)
	ON_EN_CHANGE(IDC_EDIT_VALUE2, OnUpdateCondition)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_BROWSE, OnButtonFilePath)
	ON_COMMAND(IDC_BUTTON_TEST, OnButtonTest)
	ON_COMMAND(IDC_BUTTON_IMPORTFRONT, OnButtonFrontRange)
	ON_COMMAND(IDC_BUTTON_IMPORTBACK, OnButtonBackRange)
	ON_COMMAND(IDC_BUTTON_IMPORTVALUE2, OnButtonValue2Range)

	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_MACRO, OnNotifyLinkOpenMacro)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_MACRO, OnNotifyLinkOpenMacro)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_TEBIKI, OnNotifyLinkOpenTebiki)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_TEBIKI, OnNotifyLinkOpenTebiki)
	ON_BN_CLICKED(IDC_BUTTON_TYPE, OnTypeMenuBtnClicked)

END_MESSAGE_MAP()

#pragma warning( pop )

BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	// 後段の処理の選択肢
	in->mMenuBtn.CreatePopupMenu();
	in->mMenuBtn.InsertMenu((UINT)-1, 0, 1, _T("他のコマンドを実行する"));
	in->mMenuBtn.InsertMenu((UINT)-1, 0, 2, _T("プログラムを実行する"));
	in->mMenuBtn.InsertMenu((UINT)-1, 0, 3, _T("クリップボードにコピーする"));
	in->mMenuTypeBtn.m_hMenu = (HMENU)in->mMenuBtn;

	CString caption;
  GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

	caption += suffix;
	SetWindowText(caption);

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool SettingDialog::UpdateStatus()
{
	in->mHotKey = in->mParam.mHotKeyAttr.ToString();
	if (in->mHotKey.IsEmpty()) {
		in->mHotKey.LoadString(IDS_NOHOTKEY);
	}

	// 後段の処理の設定値
	auto& param = in->mParam;
	if (param.mActionType == 0) {
		in->mMenuTypeBtn.SetWindowText(_T("他のコマンドを実行する"));
		in->mAfterDetail.Format(_T("コマンド:%s\nパラメータ:%s"),
					(LPCTSTR)param.mAfterCommandName, (LPCTSTR)param.mAfterCommandParam);
	}
	else if (param.mActionType == 1) {
		in->mMenuTypeBtn.SetWindowText(_T("プログラムを実行する"));
		in->mAfterDetail.Format(_T("ファイルパス:%s\nパラメータ:%s\n作業フォルダ:%s"),
					(LPCTSTR)param.mAfterFilePath, (LPCTSTR)param.mAfterCommandParam, (LPCTSTR)param.mAfterDir);
	}
	else {
		in->mMenuTypeBtn.SetWindowText(_T("クリップボードにコピーする"));
		in->mAfterDetail.Format(_T("パラメータ:%s"), (LPCTSTR)param.mAfterCommandParam);
	}

	BOOL canTest = TRUE;
	BOOL canCreate = TRUE;

	// 名前チェック
	bool isNameValid =
	 	launcherapp::commands::common::IsValidCommandName(in->mParam.mName, in->mOrgName, in->mMessage);
	if (isNameValid == false) {
		canTest = FALSE;
		canCreate = FALSE;
	}
	else if (in->mParam.mFilePath.IsEmpty()) {
		in->mMessage = _T("Excelファイルを指定してください");
		canTest = FALSE;
		canCreate = FALSE;
	}
	else if (PathIsURL(in->mParam.mFilePath) == FALSE && Path::FileExists(in->mParam.mFilePath) == FALSE) {
		in->mMessage = _T("ファイルが存在しません");
		canTest = FALSE;
		canCreate = FALSE;
	}
	else if (in->mParam.mSheetName.IsEmpty()) {
		in->mMessage = _T("シート名を指定してください");
		canTest = FALSE;
		canCreate = FALSE;
	}
	else if (in->mParam.mRangeFront.IsEmpty()) {
		in->mMessage = _T("キーのデータ範囲を指定してください");
		canTest = FALSE;
		canCreate = FALSE;
	}
	else if (in->mParam.mRangeBack.IsEmpty()) {
		in->mMessage = _T("値のデータ範囲を指定してください");
		canTest = FALSE;
		canCreate = FALSE;
	}
	else if (in->mIsTestPassed == false) {
		in->mMessage = _T("取得内容確認ボタンを押して内容を確認してください");
		canCreate = FALSE;
	}
	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(canTest);

	if (canCreate) {
		in->mMessage.Empty();
	}
	GetDlgItem(IDOK)->EnableWindow(canCreate);

	return canCreate != FALSE;
}

void SettingDialog::OnUpdateName()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnUpdateCondition()
{
	UpdateData();
	in->mIsTestPassed = false;
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH SettingDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void SettingDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}
	__super::OnOK();
}

void SettingDialog::OnButtonFilePath()
{
	// 現在開いているExcelシートからワークシートのパスを得る
	CString filePath;
	ExcelApplication app;
	if (app.GetFilePath(filePath) == false) {
		AfxMessageBox(_T("ファイルパスを取得するにはExcelでファイルを開いておく必要があります"));
		return;
	}

	// ToDo: パスのチェックが必要かも

	in->mParam.mFilePath = filePath;
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonTest()
{
	UpdateData();

	// ファイルパス or シート名 or キー範囲 or 値範囲がからの場合はテスト負荷
	if (in->mParam.mSheetName.IsEmpty() || in->mParam.mFilePath.IsEmpty() ||
	    in->mParam.mRangeFront.IsEmpty() || in->mParam.mRangeBack.IsEmpty()) {
		return;
	}

	CWaitCursor wc;

	ExcelApplication app;
	std::vector<CString> frontTexts;
	app.GetCellText(in->mParam.mFilePath, in->mParam.mSheetName, in->mParam.mRangeFront, frontTexts);
	std::vector<CString> backTexts;
	app.GetCellText(in->mParam.mFilePath, in->mParam.mSheetName, in->mParam.mRangeBack, backTexts);

	std::vector<CString> value2Texts;
	// 値2は任意
	if (in->mParam.mRangeValue2.IsEmpty() == FALSE) {
		app.GetCellText(in->mParam.mFilePath, in->mParam.mSheetName, in->mParam.mRangeValue2, value2Texts);
	}

	// 表面と裏面のうち、少ないほうを選択
	size_t recordCount = frontTexts.size() < backTexts.size() ? frontTexts.size() : backTexts.size();

	bool isSkipFirst =  in->mParam.mIsFirstRowHeader != FALSE;

	PreviewDialog dlg(this);

	int totalCount = 0;
	for (size_t i = 0; i < recordCount; ++i) {
		if (frontTexts[i].IsEmpty() && backTexts[i].IsEmpty()) {
			continue;
		}
		if (isSkipFirst) {
			// 「一行目をヘッダとして扱う」場合は初回のデータを無視する
			isSkipFirst = false;
			continue;
		}


		Record record(frontTexts[i], backTexts[i]);

		if (i < value2Texts.size()) {
			record.mValue2 = value2Texts[i];
		}

		if (dlg.GetActualRecordCount() < 100) {
			dlg.AddRecord(record);
		}
		totalCount++;
	}

	dlg.SetTotalRecordCount(totalCount);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	in->mIsTestPassed = true;

	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonFrontRange()
{
	UpdateData();

	ExcelApplication app(true);
	CString sheetName = app.GetActiveSheetName();
	if (sheetName.IsEmpty()) {
		AfxMessageBox(_T("Excelでファイルを開いておく必要があります"));
		return;
	}
	in->mParam.mSheetName = sheetName;

	int col,row;
	CString address = app.GetSelectionAddress(col, row);

	if (address.Find(_T(',')) != -1) {
		AfxMessageBox(_T("Ctrlキーによる複数領域選択については非対応です"));
		return;
	}

	in->mParam.mRangeFront = address;

	CString filePath;
	if (app.GetFilePath(filePath)) {
		in->mParam.mFilePath = filePath;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonBackRange()
{
	UpdateData();

	ExcelApplication app(true);
	CString sheetName = app.GetActiveSheetName();
	if (sheetName.IsEmpty()) {
		AfxMessageBox(_T("Excelでファイルを開いておく必要があります"));
		return;
	}
	in->mParam.mSheetName = sheetName;

	int col,row;
	CString address = app.GetSelectionAddress(col, row);

	if (address.Find(_T(',')) != -1) {
		AfxMessageBox(_T("Ctrlキーによる複数領域選択については非対応です"));
		return;
	}

	in->mParam.mRangeBack = address;

	CString filePath;
	if (app.GetFilePath(filePath)) {
		in->mParam.mFilePath = filePath;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonValue2Range()
{
	UpdateData();

	ExcelApplication app(true);
	CString sheetName = app.GetActiveSheetName();
	if (sheetName.IsEmpty()) {
		AfxMessageBox(_T("Excelでファイルを開いておく必要があります"));
		return;
	}
	in->mParam.mSheetName = sheetName;

	int col,row;
	CString address = app.GetSelectionAddress(col, row);

	if (address.Find(_T(',')) != -1) {
		AfxMessageBox(_T("Ctrlキーによる複数領域選択については非対応です"));
		return;
	}

	in->mParam.mRangeValue2 = address;

	CString filePath;
	if (app.GetFilePath(filePath)) {
		in->mParam.mFilePath = filePath;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonHotKey()	
{
	UpdateData();

	if (CommandHotKeyDialog::ShowDialog(in->mParam.mName, in->mParam.mHotKeyAttr, this) == false) {
		return ;
	}

	UpdateStatus();
	UpdateData(FALSE);
}

// マニュアル表示
void SettingDialog::OnNotifyLinkOpenMacro(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate(_T("MacroList"));
	*pResult = 0;
}

void SettingDialog::OnNotifyLinkOpenTebiki(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate(_T("SimpleDictTebiki"));
	*pResult = 0;
}

void SettingDialog::OnTypeMenuBtnClicked()
{
	UpdateData();

	int action = in->mMenuTypeBtn.m_nMenuResult;

	// ボタン部分が単に押された場合、mMenuResultに0が格納される
	if (action == 0) {
		action = in->mParam.mActionType + 1;    // メニューID(1始まり)に変換
	}

	switch (action) {
		case 1:
			// 他のコマンドを実行する
			OnSelectExecOtherCommand();
			break;
		case 2:
			// プログラムを実行する
			OnSelectSubProcess();
			break;
		case 3:
			// クリップボードにコピーする
			OnSelectCopyClipboard();
			break;
	}
}

void SettingDialog::OnSelectExecOtherCommand()
{
	// ダイアログ用のパラメータに変換
	OtherCommandDialog::Param param;
	param.mCommandName = in->mParam.mAfterCommandName;
	param.mCommandParam = in->mParam.mAfterCommandParam;

	// ダイアログを表示
	OtherCommandDialog dlg(_T("SimpleDictAfterOtherCommand"), this);
	dlg.SetParam(param);
	dlg.SetVariableDescription(_T("$key:キー $value:値 $value2:値2"));
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	// ダイアログの設定値を取得
	param = dlg.GetParam();
	in->mParam.mAfterCommandName = param.mCommandName;
	in->mParam.mAfterCommandParam = param.mCommandParam;

	in->mParam.mActionType = 0;

	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnSelectSubProcess()
{
	// ダイアログ用のパラメータに変換
	SubProcessDialog::Param param;
	param.mFilePath = in->mParam.mAfterFilePath;
	param.mCommandParam = in->mParam.mAfterCommandParam;
	param.mWorkDir = in->mParam.mAfterDir;
	param.mShowType = in->mParam.mAfterShowType;

	// ダイアログを表示
	SubProcessDialog dlg(_T("SimpleDictAfterSubProcess"), this);
	dlg.SetParam(param);
	dlg.SetVariableDescription(_T("$key:キー $value:値 $value2:値2"));
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	// ダイアログの設定値を取得
	param = dlg.GetParam();
	in->mParam.mAfterFilePath = param.mFilePath;
	in->mParam.mAfterCommandParam = param.mCommandParam;
	in->mParam.mAfterDir = param.mWorkDir;
	in->mParam.mAfterShowType = param.mShowType;

	in->mParam.mActionType = 1;

	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnSelectCopyClipboard()
{
	// ダイアログ用のパラメータに変換
	CopyToClipboardDialog::Param param;
	param.mCommandParam = in->mParam.mAfterCommandParam;

	// ダイアログを表示
	CopyToClipboardDialog dlg(_T("SimpleDictCopyToClipboard"), this);
	dlg.SetParam(param);
	dlg.SetVariableDescription(_T("$key:キー $value:値 $value2:値2"));
	if (dlg.DoModal() != IDOK) {
		return ;
	}
	param = dlg.GetParam();

	// ダイアログの設定値を取得
	in->mParam.mAfterCommandParam = param.mCommandParam;

	in->mParam.mActionType = 2;

	UpdateStatus();
	UpdateData(FALSE);
}



} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

