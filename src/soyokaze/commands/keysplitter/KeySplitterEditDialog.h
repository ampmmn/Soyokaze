#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>
#include "commands/keysplitter/KeySplitterParam.h"

class ModalComboBox;

namespace launcherapp {
namespace commands {
namespace keysplitter {

class SettingDialog : public launcherapp::gui::SinglePageDialog
{
public:
	SettingDialog(CWnd* parentWnd = nullptr);
	~SettingDialog() override;

	void SetName(const CString& name);
	void SetOriginalName(const CString& name);
	void SetParam(const CommandParam& param);
	const CommandParam& GetParam() const;

	bool UpdateStatus();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void SetItemToList(int index, const ModifierState& state, const ITEM& item);
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

// 実装
protected:

	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdate();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonDelete();
	afx_msg void OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult);
};

} // end of namespace keysplitter
} // end of namespace commands
} // end of namespace launcherapp

