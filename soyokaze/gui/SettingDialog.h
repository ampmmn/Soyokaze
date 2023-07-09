#pragma once

#include "gui/SettingDialogBase.h"
#include "Settings.h"
#include <memory>

// 
class SettingDialog : public SettingDialogBase
{
public:
	SettingDialog();
	virtual ~SettingDialog();

	void SetSettings(const Settings& settings);
	const Settings& GetSettings();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	//virtual void OnOK();

// 実装
protected:
	DECLARE_MESSAGE_MAP()

	HTREEITEM OnSetupPages() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

