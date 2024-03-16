#pragma once

#include "icon/IconLabel.h"
#include <vector>

class SelectFilesDialog : public CDialogEx
{
public:
	SelectFilesDialog();
	virtual ~SelectFilesDialog();

	void SetFiles(const std::vector<CString>& files);

	void GetCheckedFiles(std::vector<CString>& files);

public:
	std::vector<CString> mFiles;
	std::vector<CString> mCheckedFiles;

	CCheckListBox mListboxFiles;
	BOOL mIsResolveLink;

	// アイコン描画用ラベル
	IconLabel mIconLabel;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnListItemChanged();
	afx_msg void OnListItemChecked();
};

