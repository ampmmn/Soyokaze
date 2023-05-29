#include "pch.h"
#include "framework.h"
#include "gui/KeywordManagerDialog.h"
#include "gui/KeywordManagerListCtrl.h"
#include "gui/IconLabel.h"
#include "core/CommandRepository.h"
#include "IconLoader.h"
#include "resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


KeywordManagerDialog::KeywordManagerDialog() : 
	CDialogEx(IDD_KEYWORDMANAGER),
	mListCtrlPtr(new KeywordManagerListCtrl()),
	mIconLabelPtr(new IconLabel())
{
}

KeywordManagerDialog::~KeywordManagerDialog()
{
	delete mIconLabelPtr;
	delete mListCtrlPtr;
}

void KeywordManagerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_NAME, mName);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, mDescription);
}

BEGIN_MESSAGE_MAP(KeywordManagerDialog, CDialogEx)
	ON_COMMAND(IDC_BUTTON_NEW, OnButtonNew)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_MESSAGE(WM_APP+1, OnUserMsgListItemChanged)
	ON_MESSAGE(WM_APP+2, OnUserMsgListItemDblClk)
END_MESSAGE_MAP()


BOOL KeywordManagerDialog::OnInitDialog()
{
	__super::OnInitDialog();

	SetIcon(IconLoader::Get()->LoadDefaultIcon(), FALSE);


	mListCtrlPtr->SubclassDlgItem(IDC_LIST_COMMANDS, this);
	mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);

	// リスト　スタイル変更
	mListCtrlPtr->SetExtendedStyle(mListCtrlPtr->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	// ヘッダー追加
	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	CString strHeader;
	strHeader.LoadString(IDS_NAME);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 120;
	lvc.fmt = LVCFMT_LEFT;
	mListCtrlPtr->InsertColumn(0,&lvc);

	strHeader.LoadString(IDS_DESCRIPTION);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 200;
	lvc.fmt = LVCFMT_LEFT;
	mListCtrlPtr->InsertColumn(1,&lvc);

	ResetContents();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

void KeywordManagerDialog::ResetContents()
{
	// 更新前の選択位置を覚えておく
	int itemIndex = -1;
	POSITION pos = mListCtrlPtr->GetFirstSelectedItemPosition();
	if (pos) {
		itemIndex = mListCtrlPtr->GetNextSelectedItem(pos);
	}

	// コマンド一覧を取得する
	std::vector<soyokaze::core::Command*> commands;
	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	cmdRepoPtr->EnumCommands(commands);

	int cmdCount = (int)commands.size();
	int listItemCount = mListCtrlPtr->GetItemCount();

	// 実際のコマンド数とリスト上の項目数の差を埋める
	while (cmdCount > listItemCount) {
		mListCtrlPtr->InsertItem(listItemCount++, _T(""), 0);
	}
	while (cmdCount < listItemCount) {
		mListCtrlPtr->DeleteItem(--listItemCount);
	}

	// リスト項目の表示内容をコマンド内容で上書きする
	int index = 0;
	for (auto command : commands) {
		const CString& name = command->GetName();
		const CString& description = command->GetDescription();

		mListCtrlPtr->SetItemText(index, 0, name);
		mListCtrlPtr->SetItemText(index, 1, description);
		index++;
	}

	// 更新前に選択していた項目があれば再選択する
	if(itemIndex != -1) {
		mListCtrlPtr->SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
	}


}

bool KeywordManagerDialog::UpdateStatus()
{
	mName.Empty();
	mDescription.Empty();

	CWnd* btnEdit = GetDlgItem(IDC_BUTTON_EDIT);
	CWnd* btnDel = GetDlgItem(IDC_BUTTON_DELETE);
	ASSERT(btnEdit && btnDel);

	if (mListCtrlPtr->GetSelectedCount() == 0) {
		btnEdit->EnableWindow(FALSE);
		btnDel->EnableWindow(FALSE);
		return false;
	}

	CString name = mListCtrlPtr->GetSelectedCommandName();

	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	auto cmd = cmdRepoPtr->QueryAsWholeMatch(name);
	if (cmd) {
		mIconLabelPtr->DrawIcon(cmd->GetIcon());
		mName = name;
		mDescription = cmd->GetDescription();
	}

	bool isBuiltin = cmdRepoPtr->IsBuiltinName(name);
	if (isBuiltin) {
		btnEdit->EnableWindow(FALSE);
		btnDel->EnableWindow(FALSE);
		return false;
	}

	btnEdit->EnableWindow(TRUE);
	btnDel->EnableWindow(TRUE);

	return true;
}

void KeywordManagerDialog::OnButtonNew()
{
	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	cmdRepoPtr->NewCommandDialog(nullptr, nullptr);
	ResetContents();
}

void KeywordManagerDialog::OnButtonEdit()
{
	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	CString name = mListCtrlPtr->GetSelectedCommandName();
	if (cmdRepoPtr->IsBuiltinName(name)) {
		return;
	}

	cmdRepoPtr->EditCommandDialog(name);

	ResetContents();
}

void KeywordManagerDialog::OnButtonDelete()
{
	CString name = mListCtrlPtr->GetSelectedCommandName();

	CString confirmMsg((LPCTSTR)IDS_CONFIRM_DELETE);
	confirmMsg += _T("\n");
	confirmMsg += _T("\n");
	confirmMsg += name;

	int sel = AfxMessageBox(confirmMsg, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
	if (sel != IDYES) {
		return ;
	}

	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	if (cmdRepoPtr->DeleteCommand(name) == false) {
		return ;
	}


	ResetContents();
	UpdateStatus();
	UpdateData(FALSE);
}

LRESULT KeywordManagerDialog::OnUserMsgListItemChanged(
	WPARAM wParam,
	LPARAM lParam
)
{
	UpdateStatus();
	UpdateData(FALSE);
	return 0;
}

LRESULT KeywordManagerDialog::OnUserMsgListItemDblClk(
	WPARAM wParam,
	LPARAM lParam
)
{
	OnButtonEdit();
	return 0;
}

