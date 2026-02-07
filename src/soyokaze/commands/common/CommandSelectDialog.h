#pragma once

#include <memory>

namespace launcherapp { namespace commands { namespace common {

class CommandSelectDialog : public CDialogEx
{
public:
	CommandSelectDialog(CWnd* parent = nullptr);
	virtual ~CommandSelectDialog();

	void SetCommandName(const CString& name);
	CString GetCommandName();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV サポート
	BOOL OnInitDialog() override;
	void OnOK() override;
	void OnCancel() override;

	void ResetContents();
	void UpdateListItems();

	bool UpdateStatus();
// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEditFilterChanged();
	afx_msg void OnLbnDblClkCommands();
	afx_msg void OnLvnItemChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHeaderClicked(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFindCommand(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT_PTR timerId);
	afx_msg LRESULT OnUserMessageKeywrodEditKeyDown(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserMessageResetContent(WPARAM wParam, LPARAM lParam);
};

}}}
