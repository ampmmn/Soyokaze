#pragma once

#include <memory>

class KeywordEdit : public CEdit
{
public:
	KeywordEdit(CWnd* pParent = nullptr);	// 標準コンストラクター
	virtual ~KeywordEdit();

	void SetCaretToEnd();
	void SetIMEOff();
	void SetPlaceHolder(const CString& text);
	void SetNotifyKeyEvent(bool isNotify);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;

protected:
	//virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート

// 実装
protected:
	LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp) override;
	
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT,UINT,UINT);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnSetFocus(CWnd* oldWindow);
	afx_msg void OnKillFocus(CWnd* newWindow);
	afx_msg void OnDestroy();
	afx_msg void OnChar(UINT,UINT,UINT);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnPaste();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	DECLARE_MESSAGE_MAP()
};
