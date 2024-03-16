#pragma once

#include "gui/SettingPage.h"
#include "hotkey/HotKeyDialog.h"

// 
class SoundSettingDialog : public SettingPage
{
public:
	SoundSettingDialog(CWnd* parentWnd);
	virtual ~SoundSettingDialog();

	// 入力欄への文字入力時に再生するmp3ファイル
	CString mSoundFilePathInput;
	// 候補欄の選択変更入力時に再生するmp3ファイル
	CString mSoundFilePathSelect;
	// コマンド実行時に再生するmp3ファイル
	CString mSoundFilePathExecute;

protected:
	bool UpdateStatus();

	bool SelectFile(CString& file);

	BOOL OnKillActive() override;
	BOOL OnSetActive() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	void OnEnterSettings() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonSoundFileInput();
	afx_msg void OnButtonSoundFileSelect();
	afx_msg void OnButtonSoundFileExecute();
};

