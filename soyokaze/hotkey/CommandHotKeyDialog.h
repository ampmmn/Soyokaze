// あ
#pragma once

#include "gui/SinglePageDialog.h"
#include "hotkey/CommandHotKeyAttribute.h"

// 
class CommandHotKeyDialog : public launcherapp::gui::SinglePageDialog
{
public:
	CommandHotKeyDialog(const CommandHotKeyAttribute& attr, CWnd* parentWnd = nullptr);
	virtual ~CommandHotKeyDialog();

	void SetTargetName(const CString& name);

	void GetAttribute(CommandHotKeyAttribute& attr);

	CommandHotKeyAttribute mHotKeyAttr;
	// 初期値
	CommandHotKeyAttribute mHotKeyAttrInit;

	int mHotKeyType;
	short mVK;

	BOOL mIsUseHotKey;

	// メッセージ欄
	CString mMessage;
	//
	CString mTargetName;

	static bool ShowDialog(const CString& name, CommandHotKeyAttribute& attr, CWnd* parent=nullptr);

private:
	void UpdateCtrlState(UINT ctrlID, bool isShow, bool isEnable);

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV サポート
	BOOL OnInitDialog() override;
	void OnOK() override;

	bool UpdateStatusForHotKey();
	bool UpdateStatusForSandS();

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void UpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

