#pragma once


class FirstStartDialog : public CDialogEx
{
public:
	explicit FirstStartDialog(CWnd* parent = nullptr);

	bool IsRunAsPortable() const;

protected:
	void DoDataExchange(CDataExchange* dx) override;
	virtual BOOL OnInitDialog();
	virtual void OnOK();

private:
	int mSelectedMode{0};
	CString mSavePathText;
};



