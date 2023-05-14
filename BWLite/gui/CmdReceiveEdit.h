#pragma once

class CmdReceiveEdit : public CEdit
{
public:
	CmdReceiveEdit(CWnd* pParent = nullptr);	// 標準コンストラクター
	virtual ~CmdReceiveEdit();

// 実装
protected:
	afx_msg int OnSetText(LPCTSTR text);
	DECLARE_MESSAGE_MAP()
};
