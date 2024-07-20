#pragma once

#include "utility/TopMostMask.h"

// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
	virtual ~CAboutDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
#endif

	CString mVersionStr;
	CString mBuildDateStr; 

private:
	TopMostMask mTopMostMask;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
};

