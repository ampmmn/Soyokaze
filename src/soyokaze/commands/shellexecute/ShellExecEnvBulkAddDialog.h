#pragma once

#include "gui/SinglePageDialog.h"

namespace launcherapp { namespace commands { namespace shellexecute {

// 環境変数一括追加画面
class BulkAddDialog : public launcherapp::gui::SinglePageDialog
{
public:
	using ItemList = std::vector<std::pair<CString, CString> >;
public:
	BulkAddDialog(CWnd* parentWnd);
	virtual ~BulkAddDialog();

	void GetItems(ItemList& items);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()

private:
	CString mText;
};

}}} // end of namespace launcherapp::commands::shellexecute




