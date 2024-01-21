#pragma once

#include <memory>
#include "utility/TopMostMask.h"

class IconLabel;

namespace soyokaze {
namespace commands {
namespace watchpath {

class CommandEditDialog : public CDialogEx
{
public:
	CommandEditDialog();
	virtual ~CommandEditDialog();

	void SetOrgName(const CString& name);
	void SetName(const CString& name);
	void SetDescription(const CString& desc);
	void SetParam(const CString& param);

	int GetShowType();
	void SetShowType(int type);

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void ResolveShortcut(CString& path);

	// 編集開始時のコマンド名
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

public:
	// コマンド名
	CString mName;
	// 説明
	CString mDescription;
	// パス
	CString mPath;

private:
	TopMostMask mTopMostMask;
// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonFileBrowse();
	afx_msg void OnButtonDirBrowse();
};


}
}
}

