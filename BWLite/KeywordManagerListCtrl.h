#pragma once


/**
 *  キーワードマネージャ画面のリストコントロール
 */
class KeywordManagerListCtrl : public CListCtrl
{
public:
	KeywordManagerListCtrl();
	~KeywordManagerListCtrl();

	CString GetSelectedCommandName();

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLvnItemChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
};

