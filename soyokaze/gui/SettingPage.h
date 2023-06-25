#pragma once

class SettingPage : public CPropertyPage
{
public:
	SettingPage(LPCTSTR nameStr, UINT resourceId, CWnd* parentWnd);
	SettingPage(UINT nNameId, UINT resourceId, CWnd* parentWnd);
	virtual ~SettingPage();

	BOOL Create();

	void SetName(const CString& name);
	const CString& GetName();

	void SetParam(void* param);
	void* GetParam();

	virtual void OnEnterSettings() = 0;

protected:
	CWnd* mParentWnd;
	CString mName;
	UINT mResourceId;

	void* mParamPtr;


protected:
	void DoDataExchange(CDataExchange* pDX) override;
	DECLARE_MESSAGE_MAP()
};

