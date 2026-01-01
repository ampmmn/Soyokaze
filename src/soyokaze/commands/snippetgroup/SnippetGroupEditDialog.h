#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>
#include "commands/snippetgroup/SnippetGroupParam.h"
#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace snippetgroup {

class SettingDialog : public launcherapp::gui::SinglePageDialog
{
	using CommandParam = SnippetGroupParam;
public:
	SettingDialog(CWnd* parentWnd = nullptr);
	virtual ~SettingDialog();

	void SetName(const CString& name);
	void SetOriginalName(const CString& name);
	void SetParam(const CommandParam& param);
	const CommandParam& GetParam() const;
	void ResetHotKey();

	bool UpdateStatus();

	void SwapItem(int srcIndex, int dstIndex);
	void SetItemToList(int index, const Item& item);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	CListCtrl* mCommandListPtr;

	// 編集開始時のコマンド名
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

	// 現在選択中の定型文
	Item* mCurItem{nullptr};
	//
	bool mIsItemChanging{false};

	HACCEL mAccel{nullptr};

public:
	// 編集対象パラメータ
	CommandParam mParam;

	// ホットキー(表示用)
	CString mHotKey;

// 実装
protected:
	void OnOK() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdate();
	afx_msg void OnUpdateListItem();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonUp();
	afx_msg void OnButtonDown();
	afx_msg void OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnViewPrev();
	afx_msg void OnViewNext();
	afx_msg void OnViewUp();
	afx_msg void OnViewDown();
};

} // end of namespace snippetgroup
} // end of namespace commands
} // end of namespace launcherapp

