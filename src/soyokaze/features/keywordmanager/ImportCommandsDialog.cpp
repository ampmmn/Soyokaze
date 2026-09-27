#include "pch.h"
#include "features/keywordmanager/ImportCommandsDialog.h"
#include "commands/core/CommandIF.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct ImportCommandsDialog::PImpl
{
	bool CanImport();

	std::vector<launcherapp::core::Command*> mCommands;
	std::vector<int> mSelectedIndices;
	bool mIsOverwriteSelected{false};
	CListCtrl mListCtrl;
};

// インポートは実行可能な状態か
bool ImportCommandsDialog::PImpl::CanImport()
{
	return ImportCommandsDialog::CanImport(mListCtrl.GetSelectedCount());
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


ImportCommandsDialog::ImportCommandsDialog() :
	CDialogEx(IDD_IMPORT_COMMANDS),
	in(std::make_unique<PImpl>())
{
}

ImportCommandsDialog::~ImportCommandsDialog()
{
}

void ImportCommandsDialog::SetCommands(const std::vector<launcherapp::core::Command*>& commands)
{
	in->mCommands = commands;
}

std::vector<int> ImportCommandsDialog::GetSelectedIndices() const
{
	return in->mSelectedIndices;
}

bool ImportCommandsDialog::IsOverwriteSelected() const
{
	return in->mIsOverwriteSelected;
}

BEGIN_MESSAGE_MAP(ImportCommandsDialog, CDialogEx)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_IMPORT_COMMANDS, OnLvnItemChanged)
END_MESSAGE_MAP()

BOOL ImportCommandsDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->mListCtrl.SubclassDlgItem(IDC_LIST_IMPORT_COMMANDS, this);
	in->mListCtrl.SetExtendedStyle(in->mListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	LVCOLUMN lvc{};
	lvc.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
	lvc.fmt = LVCFMT_LEFT;

	CString header;
	header.LoadString(IDS_NAME);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)header);
	lvc.cx = 150;
	in->mListCtrl.InsertColumn(0, &lvc);

	header.LoadString(IDS_COMMANDTYPE);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)header);
	lvc.cx = 120;
	in->mListCtrl.InsertColumn(1, &lvc);

	header.LoadString(IDS_DESCRIPTION);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)header);
	lvc.cx = 380;
	in->mListCtrl.InsertColumn(2, &lvc);

	for (int index = 0; index < (int)in->mCommands.size(); ++index) {
		auto command = in->mCommands[index];
		int row = in->mListCtrl.InsertItem(index, command->GetName());
		in->mListCtrl.SetItemText(row, 1, command->GetTypeDisplayName());
		in->mListCtrl.SetItemText(row, 2, command->GetDescription());
		in->mListCtrl.SetItemState(row, LVIS_SELECTED, LVIS_SELECTED);
	}

	if (in->mCommands.empty() == false) {
		in->mListCtrl.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	CheckRadioButton(IDC_RADIO_OVERWRITE, IDC_RADIO_RENAME, IDC_RADIO_RENAME);
	UpdateStatus();

	return TRUE;
}

void ImportCommandsDialog::OnOK()
{
	if (in->CanImport() == false) {
		return;
	}

	// ダイアログ終了後にコントロールを参照しないよう、閉じる前に選択内容を保存する
	in->mSelectedIndices.clear();
	POSITION pos = in->mListCtrl.GetFirstSelectedItemPosition();
	while (pos != nullptr) {
		in->mSelectedIndices.push_back(in->mListCtrl.GetNextSelectedItem(pos));
	}
	in->mIsOverwriteSelected = IsDlgButtonChecked(IDC_RADIO_OVERWRITE) == BST_CHECKED;

	__super::OnOK();
}

void ImportCommandsDialog::OnLvnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	*pResult = 0;
	UpdateStatus();
}

void ImportCommandsDialog::UpdateStatus()
{
	GetDlgItem(IDOK)->EnableWindow(in->CanImport() ? TRUE : FALSE);
}

// ユニットテスト用
bool ImportCommandsDialog::CanImport(int n)
{
	return n != 0;
}
