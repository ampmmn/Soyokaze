#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>
#include <set>
#include <vector>
#include "commands/snippetgroup/SnippetGroupParam.h"

namespace launcherapp {
namespace commands {
namespace snippetgroup {

class SnippetGroupItemDialog : public launcherapp::gui::SinglePageDialog
{
public:
	SnippetGroupItemDialog(CWnd* parentWnd = nullptr);
	virtual ~SnippetGroupItemDialog();

	void SetItem(const Item& item);
	const Item& GetItem();

	void SetExistingNames(const std::vector<Item>& items);

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();


	// 編集開始時のコマンド名
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

	// 許可しない名前の一覧
	std::set<CString> mExistingNames;

public:
	// 編集対象パラメータ
	Item mItem;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdate();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

} // end of namespace snippetgroup
} // end of namespace commands
} // end of namespace launcherapp

