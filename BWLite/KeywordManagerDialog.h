#pragma once

class CommandRepository;

#include <vector>

class KeywordManagerListCtrl;
class IconLabel;

class KeywordManagerDialog : public CDialogEx
{
public:
	KeywordManagerDialog(CommandRepository* cmdMapPtr);
	virtual ~KeywordManagerDialog();

protected:
	CommandRepository* mCmdMapPtr;

	CString mName;
	CString mDescription;

	KeywordManagerListCtrl* mListCtrlPtr;
	IconLabel* mIconLabelPtr;
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

