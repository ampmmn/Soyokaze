#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>

class KeywordManagerDialog : public launcherapp::gui::SinglePageDialog
{
public:
	KeywordManagerDialog();
	virtual ~KeywordManagerDialog();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void ResetContents();
	void UpdateListItems();

	bool UpdateStatus();
// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEditFilterChanged();
	afx_msg void OnButtonNew();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonClone();
	afx_msg void OnButtonDelete();
	afx_msg void OnLbnDblClkCommands();
	afx_msg void OnLvnItemChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHeaderClicked(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFindCommand(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnKeywrodEditKeyDown(WPARAM wParam, LPARAM lParam);
};

