#pragma once

#include "control/SinglePageDialog.h"
#include <vector>

class SelectCommandTypeDialog : public launcherapp::control::SinglePageDialog
{
public:
	SelectCommandTypeDialog();
	virtual ~SelectCommandTypeDialog();

	int AddType(const CString displayName, const CString& description, LPARAM itemData);
	LPARAM GetSelectedItem();

	INT_PTR DoModal() override;
private:
	CString mDescriptionStr;
	int mSelIndex;

	struct ITEM {
		CString mDisplayName;
		CString mDescription;
		LPARAM mItemData = 0;
	};
	std::vector<ITEM> mItems;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	bool UpdateStatus();
// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemDblClick(NMHDR *pNMHDR, LRESULT *pResult);
};

