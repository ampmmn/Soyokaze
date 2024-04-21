#pragma once

#include <memory>
#include "utility/TopMostMask.h"
#include "hotkey/HotKeyAttribute.h"

class IconLabel;

namespace launcherapp {
namespace commands {
namespace snippet {


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
	// テキスト
	CString mText;
	// ホットキー(表示用)
	CString mHotKey;
	HOTKEY_ATTR mHotKeyAttr;
	bool mIsGlobal;

private:
	TopMostMask mTopMostMask;
// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
};


}
}
}

