#pragma once

#include "gui/SinglePageDialog.h"

namespace launcherapp { namespace commands { namespace shellexecute {

// $B4D6-JQ?t0l3gDI2C2hLL(B
class BulkAddDialog : public launcherapp::gui::SinglePageDialog
{
public:
	using ItemList = std::vector<std::pair<CString, CString> >;
public:
	BulkAddDialog(CWnd* parentWnd);
	virtual ~BulkAddDialog();

	void GetItems(ItemList& items);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV $B%5%]!<%H(B
	virtual BOOL OnInitDialog();

// $B<BAu(B
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()

private:
	CString mText;
};

}}} // end of namespace launcherapp::commands::shellexecute




