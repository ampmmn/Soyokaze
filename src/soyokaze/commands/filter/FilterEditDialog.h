#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>
#include "commands/filter/FilterCommandParam.h"
#include "hotkey/CommandHotKeyAttribute.h"

class IconLabel;

namespace launcherapp {
namespace commands {
namespace filter {


class FilterEditDialog : public launcherapp::gui::SinglePageDialog
{
public:
	FilterEditDialog(CWnd* parentWnd = nullptr);
	virtual ~FilterEditDialog();

	void SetName(const CString& name);
	void SetOriginalName(const CString& name);

	void SetParam(const CommandParam& param);
	const CommandParam& GetParam();

	void ResetHotKey();

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	bool Overlap(CWnd* dstWnd, CWnd* srcWnd);

	// 編集開始時のコマンド名
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

public:
	CommandParam mParam;
	int mCommandSelIndex;
	int mPreFilterCodePageIndex;

	std::unique_ptr<IconLabel> mIconLabelPtr;

	// ホットキー(表示用)
	CString mHotKey;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg void OnButtonBrowseFile1Clicked();
	afx_msg void OnButtonBrowseDir3Clicked();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
	afx_msg void OnButtonBrowseAfterCommandFile();
	afx_msg void OnButtonBrowseAfterCommandDir();
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

