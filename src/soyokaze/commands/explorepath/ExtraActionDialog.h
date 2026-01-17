#pragma once

#include "control/SinglePageDialog.h"
#include "commands/explorepath/ExplorePathExtraActionSettings.h"

namespace launcherapp { namespace commands { namespace explorepath {

class ExtraActionDialog : public launcherapp::control::SinglePageDialog
{
	using Entry = ExtraActionSettings::Entry;

public:
	ExtraActionDialog(CWnd* parentWnd = nullptr);

	const Entry& GetEntry();
	void SetEntry(const Entry& entry);

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
	afx_msg void OnUpdateStatus();
protected:
	bool UpdateStatus();

	Entry mEntry;
	// メッセージ欄
	CString mMessage;
	// ホットキー(表示用)
	CString mHotKey;
	int mCommandIdx;
	int mTargetIdx;
};

}}}

