#pragma once

class CmdReceiveEdit : public CEdit
{
public:
	CmdReceiveEdit(CWnd* pParent = nullptr);	// 標準コンストラクター
	virtual ~CmdReceiveEdit();

	bool mIsPasteOnly;
// 実装
protected:
	afx_msg int OnSetText(LPCTSTR text);
	afx_msg LRESULT OnUserMessagePasteOnly(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnUserMessageSetPos(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
};
