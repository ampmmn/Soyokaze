#include "pch.h"
#include "SelectFilesDialog.h"
#include "utility/ShortcutFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


SelectFilesDialog::SelectFilesDialog() : 
	CDialogEx(IDD_REGISTER_ITEMS),
	mIsResolveLink(FALSE)
{
}

SelectFilesDialog::~SelectFilesDialog()
{
}

void SelectFilesDialog::SetFiles(
	const std::vector<CString>& files
)
{
	mFiles = files;
	if (mFiles.size() > 32) {
		// 一括で登録できる最大数は32個までとする
		mFiles.resize(32);
	}
}

void SelectFilesDialog::GetCheckedFiles(
	std::vector<CString>& files
)
{
	files = mCheckedFiles;
}

void SelectFilesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_RESOLVELINKS, mIsResolveLink);
}

BOOL SelectFilesDialog::OnInitDialog()
{
	__super::OnInitDialog();

	mListboxFiles.SubclassDlgItem(IDC_LIST_FILES, this);
	mIconLabel.SubclassDlgItem(IDC_STATIC_ICON, this);

	for (auto& filePath : mFiles) {
		int index  = mListboxFiles.AddString(filePath);
		mListboxFiles.SetCheck(index, TRUE);
	}

	return TRUE;
}

BEGIN_MESSAGE_MAP(SelectFilesDialog, CDialogEx)
	ON_LBN_SELCHANGE(IDC_LIST_FILES, OnListItemChanged)
	ON_CLBN_CHKCHANGE(IDC_LIST_FILES, OnListItemChecked)
END_MESSAGE_MAP()


void SelectFilesDialog::OnOK()
{
	UpdateData();


	ASSERT(mListboxFiles.GetSafeHwnd());

	int itemCount = mListboxFiles.GetCount();
	ASSERT(itemCount == (int)mFiles.size());

	CString linkExt(_T(".lnk"));

	mCheckedFiles.clear();
	mCheckedFiles.reserve(itemCount);
	for (int i = 0; i < itemCount; ++i) {

		BOOL isChecked = mListboxFiles.GetCheck(i);
		if (isChecked == FALSE) {
			continue;
		}

		if (mIsResolveLink && 
		    linkExt.CompareNoCase(PathFindExtension(mFiles[i])) == 0) {
		
			// ショートカットがあれば実際のパスに置き換える
			CString actualPath = ShortcutFile::ResolvePath(mFiles[i]);
			mCheckedFiles.push_back(actualPath);
		}
		else {
			mCheckedFiles.push_back(mFiles[i]);
		}
	}
	__super::OnOK();
}


void SelectFilesDialog::OnListItemChanged()
{
	int selIndex = mListboxFiles.GetCurSel();
	if (selIndex == -1 || selIndex >= mFiles.size()) {
		return;
	}

	SHFILEINFO sfi = {};
	SHGetFileInfo(mFiles[selIndex], 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON);
	HICON hIcon = sfi.hIcon;
	mIconLabel.DrawIcon(hIcon);
	DestroyIcon(hIcon);
}

void SelectFilesDialog::OnListItemChecked()
{
	// ひとつもチェックされた要素がなければOKボタンを無効化する
	bool hasCheckedItem = false;

	int items = mListboxFiles.GetCount();
	for (int i = 0; i < items; ++i) {
		if (mListboxFiles.GetCheck(i)) {
			hasCheckedItem = true;
			break;
		}
	}
	GetDlgItem(IDOK)->EnableWindow(hasCheckedItem);
}

