#include "pch.h"
#include "SimpleDictEditDialog.h"
#include "commands/simple_dict/SimpleDictParam.h"
#include "commands/simple_dict/ExcelWrapper.h"
#include "utility/TopMostMask.h"
#include "core/CommandRepository.h"
#include "utility/Accessibility.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
	DDX_Text(pDX, IDC_EDIT_FRONT, in->mParam.mRange);
	DDX_Check(pDX, IDC_CHECK_FIRSTROWISHEADER, in->mParam.mIsFirstRowHeader);
}

BEGIN_MESSAGE_MAP(SettingDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateName)
	ON_EN_CHANGE(IDC_EDIT_SHEETNAME, OnUpdateCondition)
	ON_EN_CHANGE(IDC_EDIT_FRONT, OnUpdateCondition)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_BROWSE, OnButtonFilePath)
	ON_COMMAND(IDC_BUTTON_TEST, OnButtonTest)
	ON_COMMAND(IDC_BUTTON_IMPORTFRONT, OnButtonFrontRange)

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

	CString strHeader(_T("レコード"));
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 400;
	lvc.fmt = LVCFMT_LEFT;
	in->mPreviewListPtr->InsertColumn(0,&lvc);

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool SettingDialog::UpdateStatus()
{
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
	else if (in->mParam.mRange.IsEmpty() || in->mParam.mSheetName.IsEmpty()) {
		in->mMessage = _T("シート名およびデータ範囲を指定してください");
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
	if (in->mParam.mSheetName.IsEmpty() || in->mParam.mFilePath.IsEmpty() || in->mParam.mRange.IsEmpty()) {
		return;
	}

	ExcelApplication app;
	std::vector<CString> texts;
	app.GetCellText(in->mParam.mFilePath, in->mParam.mSheetName, in->mParam.mRange, texts);


	size_t startIdx = in->mParam.mIsFirstRowHeader ? 1 : 0;

	in->mPreviewListPtr->DeleteAllItems();
	for (size_t i = startIdx; i < texts.size(); ++i) {
		in->mPreviewListPtr->InsertItem((int)i, texts[i]);
	}

	in->mIsTestPassed = true;
	in->mRecordMsg.Format(_T("%d件のレコードが見つかりました"), texts.size());
	UpdateStatus();

	UpdateData(FALSE);
}

void SettingDialog::OnButtonFrontRange()
{
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

	in->mParam.mRange = address;

	CString filePath;
	if (app.GetFilePath(filePath)) {
		in->mParam.mFilePath = filePath;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace soyokaze

