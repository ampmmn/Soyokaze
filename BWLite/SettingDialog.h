#pragma once

#include "HotKeyDialog.h"

// 
class SettingDialog : public CDialogEx
{
public:
	SettingDialog();
	virtual ~SettingDialog();

	// ランチャー呼び出しキー（表示用)
	CString mHotKey;
	HOTKEY_ATTR mHotKeyAttr;

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

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonHotKey();
	afx_msg void OnButtonBrowseFile();
	afx_msg void OnCheckUseFilter();
	afx_msg void OnCbnTransparencyChanged();
};

