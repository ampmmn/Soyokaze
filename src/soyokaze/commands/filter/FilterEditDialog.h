#pragma once

#include "control/SinglePageDialog.h"
#include <memory>
#include "commands/filter/FilterCommandParam.h"
#include "hotkey/CommandHotKeyAttribute.h"

class IconLabel;

namespace launcherapp {
namespace commands {
namespace filter {


class FilterEditDialog : public launcherapp::control::SinglePageDialog
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

	// 編集開始時のコマンド名
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

	// 前段設定内容詳細
	CString mPreFilterDetail;
	CMFCMenuButton mPathMenuType1;
	CMenu mMenuForType1Btn;

	CString mAfterDetail;
	CMFCMenuButton mPathMenuType2;
	CMenu mMenuForType2Btn;
public:
	CommandParam mParam;

	std::unique_ptr<IconLabel> mIconLabelPtr;

	// ホットキー(表示用)
	CString mHotKey;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
	afx_msg void OnType1MenuBtnClicked();
	afx_msg void OnType2MenuBtnClicked();
	afx_msg void OnCheckDisplayName();

	void OnSelectSubProcessFilter();
	void OnSelectClipboardFilter();
	void OnSelectConstantFilter();
	void OnSelectAfterExecOtherCommand();
	void OnSelectAfterSubProcess();
	void OnSelectAfterCopyClipboard();
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

