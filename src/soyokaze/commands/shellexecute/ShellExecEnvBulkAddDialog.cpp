#include "pch.h"
#include "framework.h"
#include "ShellExecEnvBulkAddDialog.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace shellexecute {
	
BulkAddDialog::BulkAddDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_SHELLEXEC_ENVBULKADD, parentWnd)
{
	SetHelpPageId(_T("ShellExecEnvBulkAdd"));
}

BulkAddDialog::~BulkAddDialog()
{
}

void BulkAddDialog::GetItems(ItemList& items)
{
	ItemList tmpItems;

	// ToDo: 実装
	int n = 0;
	CString tok = mText.Tokenize(_T("\n"), n);
	while(tok.IsEmpty() == FALSE) {

		int sep = tok.Find(_T('='));
		if (sep != -1) {
			auto name = tok.Left(sep);
			auto value = tok.Mid(sep+1);

			name.Trim();
			if (name.Find(_T(' ')) != -1) {
				// スペースを含むものは許可しない
				tok = mText.Tokenize(_T("\n"), n);
				continue;
			}
			tmpItems.emplace_back(name, value);
		}
		tok = mText.Tokenize(_T("\n"), n);
	}
	items.swap(tmpItems);
}

void BulkAddDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_VALUE, mText);
}

#pragma warning( push )

BEGIN_MESSAGE_MAP(BulkAddDialog, launcherapp::gui::SinglePageDialog)
END_MESSAGE_MAP()

#pragma warning( pop )


BOOL BulkAddDialog::OnInitDialog()
{
	__super::OnInitDialog();

	return TRUE;
}

void BulkAddDialog::OnOK()
{
	UpdateData();
	__super::OnOK();
}


}}} // end of namespace launcherapp::commands::shellexecute
