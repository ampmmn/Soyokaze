#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>
#include "commands/group/CommandParam.h"
#include "hotkey/CommandHotKeyAttribute.h"

class ModalComboBox;

namespace launcherapp {
namespace commands {
namespace group {

struct GroupItem;

class GroupEditDialog : public launcherapp::gui::SinglePageDialog
{
public:
	GroupEditDialog(CWnd* parentWnd = nullptr);
	virtual ~GroupEditDialog();

	void SetName(const CString& name);
	void SetOriginalName(const CString& name);
	void SetParam(const CommandParam& param);
	const CommandParam& GetParam() const;

	bool UpdateStatus();

	void SwapItem(int srcIndex, int dstIndex);
	void SetItemToList(int index, const GroupItem& item);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	bool SelectCommand(int index);

	//
	CListCtrl* mCommandListPtr;

	std::unique_ptr<ModalComboBox> mCommandSelectBox;

	// 編集開始時のコマンド名
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

public:
	// 編集対象パラメータ
	CommandParam mParam;

	// ホットキー(表示用)
	CString mHotKey;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdate();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonUp();
	afx_msg void OnButtonDown();
	afx_msg void OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult);
};

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

