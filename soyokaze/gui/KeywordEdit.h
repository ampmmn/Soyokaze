#pragma once

#include <memory>

class KeywordEdit : public CEdit
{
public:
	KeywordEdit(CWnd* pParent = nullptr);	// 標準コンストラクター
	virtual ~KeywordEdit();

	void SetCaretToEnd();

	void SetIMEOff();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;

protected:
	//virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート

// 実装
protected:
	LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp) override;

	afx_msg void OnKeyDown(UINT,UINT,UINT);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnSetFocus(CWnd* oldWindow);
	afx_msg void OnKillFocus(CWnd* newWindow);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};
