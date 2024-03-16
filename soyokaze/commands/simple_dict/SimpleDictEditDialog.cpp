#include "pch.h"
#include "SimpleDictEditDialog.h"
#include "commands/simple_dict/SimpleDictParam.h"
#include "commands/simple_dict/ExcelWrapper.h"
#include "gui/FolderDialog.h"
#include "utility/TopMostMask.h"
#include "commands/core/CommandRepository.h"
#include "utility/Accessibility.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = soyokaze::core::CommandRepository;
using Command =  soyokaze::core::Command;

namespace soyokaze {
namespace commands {
namespace simple_dict {

struct SettingDialog::PImpl
{
	// 編集開始時のコマンド名
	CString mOrgName;
	// メッセージ欄
	CString mMessage;
	CString mRecordMsg;

	int mCommandSelIndex;

	// 編集対象パラメータ
	SimpleDictParam mParam;
	//
	CListCtrl* mPreviewListPtr;

	TopMostMask mTopMostMask;

	bool mIsTestPassed;
};


SettingDialog::SettingDialog() : 
	CDialogEx(IDD_SIMPLEDICT), in(new PImpl)
{
	in->mPreviewListPtr = nullptr;
	in->mIsTestPassed = false;
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::SetParam(const SimpleDictParam& param)
{
	in->mOrgName = param.mName;
	in->mParam = param;

	if (in->mOrgName.IsEmpty() == FALSE) {
		in->mIsTestPassed = true;
	}
}

const SimpleDictParam& SettingDialog::GetParam() const
{
	return in->mParam;
}

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_STATIC_RECORDS, in->mRecordMsg);
	DDX_Text(pDX, IDC_EDIT_NAME, in->mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, in->mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_FILEPATH, in->mParam.mFilePath);
	DDX_Text(pDX, IDC_EDIT_SHEETNAME, in->mParam.mSheetName);
	DDX_Text(pDX, IDC_EDIT_FRONT, in->mParam.mRangeFront);
	DDX_Text(pDX, IDC_EDIT_BACK, in->mParam.mRangeBack);
	DDX_Check(pDX, IDC_CHECK_FIRSTROWISHEADER, in->mParam.mIsFirstRowHeader);
	DDX_Check(pDX, IDC_CHECK_MATCHWITHOUTKEYWORD, in->mParam.mIsMatchWithoutKeyword);
	DDX_Check(pDX, IDC_CHECK_REVERSE, in->mParam.mIsEnableReverse);

	DDX_CBIndex(pDX, IDC_COMBO_AFTERCOMMAND, in->mCommandSelIndex);
	DDX_CBIndex(pDX, IDC_COMBO_AFTERTYPE, in->mParam.mActionType);
	DDX_Text(pDX, IDC_EDIT_PARAM2, in->mParam.mAfterCommandParam);
	DDX_Text(pDX, IDC_EDIT_PATH2, in->mParam.mAfterFilePath);
}

BEGIN_MESSAGE_MAP(SettingDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateName)
	ON_EN_CHANGE(IDC_EDIT_SHEETNAME, OnUpdateCondition)
	ON_EN_CHANGE(IDC_EDIT_FRONT, OnUpdateCondition)
	ON_EN_CHANGE(IDC_EDIT_BACK, OnUpdateCondition)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_BROWSE, OnButtonFilePath)
	ON_COMMAND(IDC_BUTTON_TEST, OnButtonTest)
	ON_COMMAND(IDC_BUTTON_IMPORTFRONT, OnButtonFrontRange)
	ON_COMMAND(IDC_BUTTON_IMPORTBACK, OnButtonBackRange)

	ON_CBN_SELCHANGE(IDC_COMBO_AFTERTYPE, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_AFTERCOMMAND, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_PATH2, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE3, OnButtonBrowseAfterCommandFile)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR4, OnButtonBrowseAfterCommandDir)

END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->mPreviewListPtr = (CListCtrl*)GetDlgItem(IDC_LIST_PREVIEW);
	ASSERT(in->mPreviewListPtr);
	in->mPreviewListPtr->SetExtendedStyle(in->mPreviewListPtr->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	CString caption;
  GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

	caption += suffix;
	SetWindowText(caption);

	// ヘッダー追加
	ASSERT(in->mPreviewListPtr);

	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	CString strHeader(_T("キー"));
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 200;
	lvc.fmt = LVCFMT_LEFT;
	in->mPreviewListPtr->InsertColumn(0,&lvc);

	strHeader = (_T("値"));
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 200;
	lvc.fmt = LVCFMT_LEFT;
	in->mPreviewListPtr->InsertColumn(1,&lvc);

	// 後段のコマンド設定 排他の項目の位置を調整する
	Overlap(GetDlgItem(IDC_STATIC_AFTERCOMMAND), GetDlgItem(IDC_STATIC_PATH2));
	Overlap(GetDlgItem(IDC_COMBO_AFTERCOMMAND), GetDlgItem(IDC_EDIT_PATH2));
	GetDlgItem(IDC_BUTTON_BROWSEFILE3)->SetWindowTextW(L"\U0001F4C4");
	GetDlgItem(IDC_BUTTON_BROWSEDIR4)->SetWindowTextW(L"\U0001F4C2");

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool SettingDialog::UpdateStatus()
{
	int actionType = in->mParam.mActionType;
	if (actionType== 0) {
		// 他のコマンドを実行する
		GetDlgItem(IDC_STATIC_AFTERCOMMAND)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_COMBO_AFTERCOMMAND)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_PATH2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_PATH2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUTTON_BROWSEFILE3)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUTTON_BROWSEDIR4)->ShowWindow(SW_HIDE);
	}
	else if (actionType== 1) {
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
	if (actionType == 0 && in->mCommandSelIndex == -1) {
		in->mMessage = _T("絞込み後に実行するコマンドを選んでください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}
	if (actionType == 1 && in->mParam.mAfterFilePath.IsEmpty()) {
		in->mMessage = _T("絞込み後に実行するファイルまたはURLを入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

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

		if (name == in->mParam.mAfterCommandName) {
			in->mCommandSelIndex = idx;
		}
	}


	BOOL canTest = TRUE;
	BOOL canCreate = TRUE;

	if (in->mParam.mName.IsEmpty()) {
		in->mMessage.LoadString(IDS_ERR_NAMEISEMPTY);
		canTest = FALSE;
		canCreate = FALSE;
	}
	else if (in->mParam.mFilePath.IsEmpty()) {
		in->mMessage = _T("Excelファイルを指定してください");
		canTest = FALSE;
		canCreate = FALSE;
	}
	else if (PathFileExists(in->mParam.mFilePath) == FALSE) {
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
		in->mMessage = _T("テストを押下して内容を確認してください");
		canCreate = FALSE;
	}

	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(canTest);

	// 重複チェック
	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	if (in->mParam.mName.CompareNoCase(in->mOrgName) != 0) {
		auto cmd = cmdRepoPtr->QueryAsWholeMatch(in->mParam.mName, false);
		if (cmd != nullptr) {
			cmd->Release();
			in->mMessage.LoadString(IDS_ERR_NAMEALREADYEXISTS);
			GetDlgItem(IDOK)->EnableWindow(FALSE);
			return false;
		}
	}

	// 使えない文字チェック
	if (cmdRepoPtr->IsValidAsName(in->mParam.mName) == false) {
		in->mMessage.LoadString(IDS_ERR_ILLEGALCHARCONTAINS);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

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
	if (utility::IsHighContrastMode()) {
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

	// 「他のコマンドを実行」の場合、リストコントロールからコマンド名を取得する
	if (in->mParam.mActionType == 0) {
		CComboBox* cmbBox = (CComboBox*)GetDlgItem(IDC_COMBO_AFTERCOMMAND);
		cmbBox->GetLBText(in->mCommandSelIndex, in->mParam.mAfterCommandName);
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

	ExcelApplication app;
	std::vector<CString> frontTexts;
	app.GetCellText(in->mParam.mFilePath, in->mParam.mSheetName, in->mParam.mRangeFront, frontTexts);
	std::vector<CString> backTexts;
	app.GetCellText(in->mParam.mFilePath, in->mParam.mSheetName, in->mParam.mRangeBack, backTexts);

	// 表面と裏面のうち、少ないほうを選択
	size_t recordCount = frontTexts.size() < backTexts.size() ? frontTexts.size() : backTexts.size();
	// 一行目をヘッダとして扱う場合は2行目からカウントする
	size_t startIdx = in->mParam.mIsFirstRowHeader ? 1 : 0;

	int emptyCount = 0;

	in->mPreviewListPtr->DeleteAllItems();
	int listIndex = 0;
	for (size_t i = startIdx; i < recordCount; ++i) {
		if (frontTexts[i].IsEmpty() && backTexts[i].IsEmpty()) {
			emptyCount++;
			continue;
		}
		int n = in->mPreviewListPtr->InsertItem(listIndex++, frontTexts[i]);
		in->mPreviewListPtr->SetItemText(n, 1, backTexts[i]);
	}

	in->mIsTestPassed = true;
	in->mRecordMsg.Format(_T("%d件のレコードが見つかりました"), recordCount - startIdx - emptyCount);
	UpdateStatus();

	UpdateData(FALSE);
}

void SettingDialog::OnButtonFrontRange()
{
	UpdateData();

	ExcelApplication app;
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

	ExcelApplication app;
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

void SettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonBrowseAfterCommandFile()
{
	UpdateData();
	CFileDialog dlg(TRUE, NULL, in->mParam.mAfterFilePath, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	in->mParam.mAfterFilePath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonBrowseAfterCommandDir()
{
	UpdateData();
	CFolderDialog dlg(_T(""), in->mParam.mAfterFilePath, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	in->mParam.mAfterFilePath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

bool SettingDialog::Overlap(CWnd* dstWnd, CWnd* srcWnd)
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


} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace soyokaze

