#pragma once

class ShortcutDialog : public CDialogEx
{
public:
	ShortcutDialog();
	virtual ~ShortcutDialog();

	CString mBWLitePath;

	// 各種ショートカットのパス
	CString mSendToPath;
	CString mStartMenuDir;
	CString mStartMenuPath;
	CString mDesktopPath;
	CString mStartupPath;

	BOOL mSendTo;
	BOOL mStartMenu;
	BOOL mDesktop;
	BOOL mStartup;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	void UpdateStatus();

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonDelete();
};

