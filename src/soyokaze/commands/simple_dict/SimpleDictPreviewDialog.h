#pragma once

#include "control/SinglePageDialog.h"
#include <memory>

#include "commands/simple_dict/SimpleDictionary.h"

namespace launcherapp {
namespace commands {
namespace simple_dict {

class PreviewDialog : public launcherapp::control::SinglePageDialog
{
public:
	PreviewDialog(CWnd* parentWnd = nullptr);
	virtual ~PreviewDialog();

	void SetTotalRecordCount(int total);
	void AddRecord(const Record& record);
	int GetActualRecordCount();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	struct PImpl;
	std::unique_ptr<PImpl> in;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

