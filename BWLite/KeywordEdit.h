#pragma once

class KeywordEdit : public CEdit
{
public:
	KeywordEdit(CWnd* pParent = nullptr);	// 標準コンストラクター
	virtual ~KeywordEdit();

	void SetCaretToEnd();

protected:
	//virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート

// 実装
protected:
	afx_msg void OnKeyDown(UINT,UINT,UINT);
	afx_msg UINT OnGetDlgCode();
	DECLARE_MESSAGE_MAP()
};
