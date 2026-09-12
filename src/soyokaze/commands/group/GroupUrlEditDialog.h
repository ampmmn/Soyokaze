#pragma once

#include "control/SinglePageDialog.h"
#include "commands/group/CommandParam.h"

namespace launcherapp { namespace commands { namespace group {

class GroupUrlEditDialog : public launcherapp::control::SinglePageDialog
{
public:
	GroupUrlEditDialog(CWnd* parentWnd = nullptr);
	void SetItem(const GroupItem& item);
	const GroupItem& GetItem() const;

 protected:
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void OnSiteMenuButtonClicked();
	afx_msg void OnUpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

 private:
	bool UpdateStatus();

	GroupItem mItem;
	CString mMessage;
	CMFCMenuButton mSiteMenuButton;
	CMenu mSiteMenu;
	DECLARE_MESSAGE_MAP()
};

}}}
