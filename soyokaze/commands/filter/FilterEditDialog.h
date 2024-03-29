#pragma once

#include <memory>
#include "commands/filter/FilterCommandParam.h"
#include "utility/TopMostMask.h"
#include "hotkey/HotKeyAttribute.h"

class IconLabel;

namespace soyokaze {
namespace commands {
namespace filter {


class FilterEditDialog : public CDialogEx
{
public:
	FilterEditDialog();
	virtual ~FilterEditDialog();

	void SetOrgName(const CString& name);

	void SetParam(const CommandParam& param);
	void GetParam(CommandParam& param);

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
	//
	int mShowTypeIdx;
	int mCommandSelIndex;

	std::unique_ptr<IconLabel> mIconLabelPtr;

	// ホットキー(表示用)
	CString mHotKey;
	HOTKEY_ATTR mHotKeyAttr;
	bool mIsGlobal;

private:
	TopMostMask mTopMostMask;
// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEditNameChanged();
	afx_msg void OnEditPathChanged();
	afx_msg void OnButtonBrowseFile1Clicked();
	afx_msg void OnButtonBrowseDir3Clicked();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
	afx_msg void OnCbnAfterTypeChanged();
	afx_msg void OnCbnAfterCommandChanged();
	afx_msg void OnButtonBrowseAfterCommandFile();
	afx_msg void OnButtonBrowseAfterCommandDir();
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace soyokaze

