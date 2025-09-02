#include "pch.h"
#include "SimpleDictPreviewDialog.h"
#include "utility/Accessibility.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace simple_dict {

struct PreviewDialog::PImpl
{
	void UpdateListCtrlItems();

	int GetTotalRecordCount() {
		return (int)mRecords.size();
	}

	CListCtrl* mPreviewListPtr{nullptr};

	// 全体の件数
	int mTotalRecords{0};
	// プレビュー上表示するレコード
	std::vector<Record> mRecords;
	CString mRecordMsg;
};

void PreviewDialog::PImpl::UpdateListCtrlItems()
{
	mPreviewListPtr->DeleteAllItems();

	int listIndex = 0;
	for (auto& record : mRecords) {

		if (record.mKey.IsEmpty() && record.mValue.IsEmpty()) {
			continue;
		}
		int n = mPreviewListPtr->InsertItem(listIndex++, record.mKey);
		mPreviewListPtr->SetItemText(n, 1, record.mValue);
		mPreviewListPtr->SetItemText(n, 2, record.mValue2);
	}

	if (mTotalRecords == (int)mRecords.size()) {
		mRecordMsg.Format(_T("%d件のレコード"), GetTotalRecordCount());
	}
	else {
		mRecordMsg.Format(_T("%d件のレコード(先頭%d件のみ表示)"), mTotalRecords, GetTotalRecordCount());
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



PreviewDialog::PreviewDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_SIMPLEDICT_TEST, parentWnd), in(new PImpl)
{
	SetHelpPageId("SimpleDictPreview");
}

PreviewDialog::~PreviewDialog()
{
}

void PreviewDialog::SetTotalRecordCount(int total)
{
	in->mTotalRecords = total;
}

void PreviewDialog::AddRecord(const Record& record)
{
	in->mRecords.push_back(record);
}

int PreviewDialog::GetActualRecordCount()
{
	return (int)in->mRecords.size();
}

void PreviewDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_RECORDS, in->mRecordMsg);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(PreviewDialog, launcherapp::gui::SinglePageDialog)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL PreviewDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->mPreviewListPtr = (CListCtrl*)GetDlgItem(IDC_LIST_PREVIEW);
	ASSERT(in->mPreviewListPtr);
	in->mPreviewListPtr->SetExtendedStyle(in->mPreviewListPtr->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

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

	strHeader = (_T("値2"));
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 200;
	lvc.fmt = LVCFMT_LEFT;
	in->mPreviewListPtr->InsertColumn(2,&lvc);

	// 行登録
	in->UpdateListCtrlItems();

	UpdateData(FALSE);

	return TRUE;
}

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

