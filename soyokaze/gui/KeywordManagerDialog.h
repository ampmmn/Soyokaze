#pragma once

#include "utility/TopMostMask.h"
#include <vector>

class KeywordManagerListCtrl;
class IconLabel;

class KeywordManagerDialog : public CDialogEx
{
public:
	KeywordManagerDialog();
	virtual ~KeywordManagerDialog();

protected:
	CString mName;
	CString mDescription;

	KeywordManagerListCtrl* mListCtrlPtr;
	IconLabel* mIconLabelPtr;

private:
	TopMostMask mTopMostMask;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void ResetContents();

	bool UpdateStatus();
// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonNew();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonDelete();
	afx_msg void OnLbnDblClkCommands();
	afx_msg LRESULT OnUserMsgListItemChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserMsgListItemDblClk(WPARAM wParam, LPARAM lParam);
};

