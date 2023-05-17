#pragma once

#include "gui/SettingPage.h"
#include "gui/HotKeyDialog.h"

// 
class BasicSettingDialog : public SettingPage
{
public:
	BasicSettingDialog(CWnd* parentWnd);
	virtual ~BasicSettingDialog();

	// ランチャー呼び出しキー（表示用)
	CString mHotKey;
	HOTKEY_ATTR mHotKeyAttr;
	BOOL mIsShowToggle;

	// 絞込方法(0:前方一致 1:部分一致 2:スキップマッチング)
	int mMatchLevel;

	// フォルダを開くファイラーを指定
	BOOL mIsUseExternalFiler;
	// ファイラーのパス
	CString mFilerPath;
	// ファイラーのパラメータ
	CString mFilerParam;

	// 入力画面を常に最前面に表示
	BOOL mIsTopMost;

	// 半透明の表示方法
	int mTransparencyType;
	// 半透明表示の透明度
	UINT mAlpha;

protected:
	bool UpdateStatus();

	BOOL OnKillActive() override;
	BOOL OnSetActive() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonHotKey();
	afx_msg void OnButtonBrowseFile();
	afx_msg void OnCheckUseFilter();
	afx_msg void OnCbnTransparencyChanged();
	afx_msg void OnButtonShortcut();
};

