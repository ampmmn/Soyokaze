#pragma once

#include "control/SinglePageDialog.h"
#include <memory>
#include "commands/websearch/WebSearchCommandParam.h"

namespace launcherapp {
namespace commands {
namespace websearch {

// 
class SettingDialog : public launcherapp::control::SinglePageDialog
{
protected:
	using Param = CommandParam; 

public:
	SettingDialog(CWnd* parentWnd = nullptr);
	virtual ~SettingDialog();

	void SetName(const CString& name);
	void SetOriginalName(const CString& name);
	void SetParam(const Param& param);
	const Param& GetParam();

	void ResetHotKey();

	void SetIcon(HICON icon);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void UpdateStatus();
// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnUpdateStatus();
	afx_msg LRESULT OnUserMessageIconChanged(WPARAM wp, LPARAM lp);
	/** 検索URLのプリセットメニューを処理する */
	afx_msg void OnSiteMenuBtnClicked();
	/** 検索先の選択に応じてタイトル入力欄の表示を切り替える */
	afx_msg void OnApplicationChanged();
	/** ホットキー入力の変更を反映する */
	afx_msg LRESULT OnUserMessageHotKeyChange(WPARAM wp, LPARAM lp);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
