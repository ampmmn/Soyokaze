#include "pch.h"
#include "SnippetGroupItemDialog.h"
#include "utility/Accessibility.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace snippetgroup {

SnippetGroupItemDialog::SnippetGroupItemDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_SNIPPETGROUP_ITEM, parentWnd)
{
	SetHelpPageId("SnippetGroupItem");
}

SnippetGroupItemDialog::~SnippetGroupItemDialog()
{
}

void SnippetGroupItemDialog::SetItem(const Item& item)
{
	mItem = item;
	mOrgName = item.mName;
}

const Item& SnippetGroupItemDialog::GetItem()
{
	return mItem;
}

void SnippetGroupItemDialog::SetExistingNames(const std::vector<Item>& items)
{
	for (const auto& item : items) {
		if (item.mName == mOrgName) {
			continue;
		}
		mExistingNames.insert(item.mName);
	}
}

void SnippetGroupItemDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mItem.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mItem.mDescription);
	DDX_Text(pDX, IDC_EDIT_TEXT, mItem.mText);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(SnippetGroupItemDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdate)
	ON_EN_CHANGE(IDC_EDIT_TEXT, OnUpdate)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL SnippetGroupItemDialog::OnInitDialog()
{
	__super::OnInitDialog();

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

bool SnippetGroupItemDialog::UpdateStatus()
{
	if (mItem.mName.IsEmpty()) {
		mMessage = _T("名前を入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	if (mExistingNames.find(mItem.mName) != mExistingNames.end()) {
		mMessage = _T("同じ名前の定型文が既に登録されています");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	if (mItem.mText.IsEmpty()) {
		mMessage = _T("名前を入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(true);
	return true;
}

void SnippetGroupItemDialog::OnUpdate()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH SnippetGroupItemDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (::utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}

void SnippetGroupItemDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}
	__super::OnOK();
}

} // end of namespace snippetgroup
} // end of namespace commands
} // end of namespace launcherapp

