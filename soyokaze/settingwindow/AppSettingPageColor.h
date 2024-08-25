#pragma once

#include "gui/SettingPage.h"
// 
class AppSettingPageColor : public SettingPage
{
public:
	AppSettingPageColor(CWnd* parentWnd);
	virtual ~AppSettingPageColor();

	struct PImpl;
	std::unique_ptr<PImpl> in;

protected:
	void DisableAllControls();
	bool UpdateStatus();
	void ApplyToCtrl();
	void DrawPreview();

	BOOL OnKillActive() override;
	BOOL OnSetActive() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	void OnEnterSettings() override;
	bool GetHelpPageId(CString& id) override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	void OnButtonRestore();
	void OnUpdateStatus();
};

