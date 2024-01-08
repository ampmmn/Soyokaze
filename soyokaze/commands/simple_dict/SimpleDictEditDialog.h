#pragma once

#include <memory>
#include "commands/simple_dict/SimpleDictParam.h"

class ModalComboBox;

namespace soyokaze {
namespace commands {
namespace simple_dict {

class SettingDialog : public CDialogEx
{
public:
	SettingDialog();
	virtual ~SettingDialog();

	void SetParam(const SimpleDictParam& param);
	const SimpleDictParam& GetParam() const;

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	struct PImpl;
	std::unique_ptr<PImpl> in;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateName();
	afx_msg void OnUpdateCondition();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonFilePath();
	afx_msg void OnButtonTest();
	afx_msg void OnButtonFrontRange();
};

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace soyokaze

