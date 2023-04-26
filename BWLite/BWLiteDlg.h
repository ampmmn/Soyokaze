
// BWLiteDlg.h : ヘッダー ファイル
//

#pragma once

#include <vector>

class CommandMap;
class Command;

class SharedHwnd;

// CBWLiteDlg ダイアログ
class CBWLiteDlg : public CDialogEx
{
// コンストラクション
public:
	CBWLiteDlg(CWnd* pParent = nullptr);	// 標準コンストラクター
	virtual ~CBWLiteDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BWLITE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート

	CommandMap* GetCommandMap();
	void SetDescription(const CString& msg);

// 実装
protected:
	HICON m_hIcon;
	CString m_strCommand;
	CString m_strDescription;
	CommandMap* m_pCommandMap;
	Command* m_pCurCommand;
	SharedHwnd* m_pSharedHwnd;
	CListBox mCandidateListBox;
	std::vector<Command*> mCandidates;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnEditCommandChanged();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_MESSAGE_MAP()
};
