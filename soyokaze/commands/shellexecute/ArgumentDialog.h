#pragma once

#include <vector>

namespace soyokaze {
namespace commands {
namespace shellexecute {


class ArgumentDialog : public CDialogEx
{
public:
	ArgumentDialog(const CString& cmdName, CWnd* parentWnd = nullptr);
	virtual ~ArgumentDialog();

	const CString& GetArguments();
	
	CString mCommandName;
	CString mArguments;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
};

}
}
}
