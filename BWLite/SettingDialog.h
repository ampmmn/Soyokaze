#pragma once

// 
class SettingDialog : public CDialogEx
{
public:
	SettingDialog();
	virtual ~SettingDialog();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

