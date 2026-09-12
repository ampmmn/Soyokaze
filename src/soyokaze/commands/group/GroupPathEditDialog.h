#pragma once

#include "control/SinglePageDialog.h"
#include "commands/group/CommandParam.h"
#include <afxbutton.h>

namespace launcherapp { namespace commands { namespace group {

class GroupPathEditDialog : public launcherapp::control::SinglePageDialog
{
public:
	GroupPathEditDialog(CWnd* parentWnd = nullptr);
	void SetItem(const GroupItem& item);
	const GroupItem& GetItem() const;

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	private:
	bool UpdateStatus();
	void OnButtonBrowseFileClicked();
	void OnButtonBrowseDirClicked();
	void OnButtonBrowseDir3Clicked();
	void OnPathMenuButtonClicked();
	afx_msg void OnUpdateStatus();

	GroupItem mItem;
	CString mMessage;
	int mShowIndex{0};
	CMFCMenuButton mPathMenuButton;
	CMenu mPathMenu;
	DECLARE_MESSAGE_MAP()
};

}}}
