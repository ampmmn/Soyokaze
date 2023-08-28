#pragma once

#include "AppPreferenceListenerIF.h"
#include "Settings.h"
#include <memory>

class CommandHotKeyMappings;

class AppPreference
{
	class NotifyWindow;

public:
	static AppPreference* Get();

	void Init();

	void Load();
	void Save();
	void OnExit();

	void SetSettings(const Settings& settings);
	const Settings& GetSettings();

	CString GetFilerPath();
	CString GetFilerParam();
	bool IsUseFiler();

	bool IsHideOnStartup();
	bool IsHideOnInactive();

	bool IsTopMost();
	bool IsShowToggle();

	bool IsWindowTransparencyEnable();
	int GetAlpha();
	bool IsTransparencyInactiveOnly();

	UINT GetModifiers();
	UINT GetVirtualKeyCode();

	// 入力画面表示時にIMEをオフにするか?
	bool IsIMEOffOnActive();

	// ネットワークパスを無視する
	bool IsIgnoreUNC();

	// フィルタコマンドの同時実行を許可する
	bool IsArrowFilterCommandConcurrentRun();

	// コメント表示欄の初期表示文字列を取得
	CString GetDefaultComment();

	void SetCommandKeyMappings(const CommandHotKeyMappings& keyMap);
	void GetCommandKeyMappings(CommandHotKeyMappings& keyMap);

	void RegisterListener(AppPreferenceListenerIF* listener);
	void UnregisterListener(AppPreferenceListenerIF* listener);

	bool CreateUserDirectory();

	bool IsEnableCalculator();
	CString GetPythonDLLPath();

	bool IsEnableExcelWorksheet();

	// ウインドウの切り替え機能を有効にするか?
	bool IsEnableWindowSwitch();

	bool IsEnableBookmark();

	bool IsShowFolderIfCtrlKeyIsPressed();

	// 入力欄ウインドウをマウスカーソル位置に表示するか
	bool IsShowMainWindowOnCurorPos();

	// 入力欄ウインドウにコマンド種別を表示するか
	bool IsShowCommandType();

	// C/Migemo検索を利用するか
	bool IsEnableMigemo();

	// コントロールパネルのアイテム検索を使用するか
	bool IsEnableControlPanel();

	// スタートメニュー/最近使ったファイルのアイテム検索を使用するか
	bool IsEnableSpecialFolder();
	// デスクトップ上のファイルのアイテム検索を使用するか
	bool IsEnableDesktop();

	// UWPアプリの検索を使用するか
	bool IsEnableUWP();

	// Outlookのメール(受信トレイ)の検索を使用するか
	bool IsEnableOutlookMailItem();

	// 効果音ファイルパスを取得(文字入力)
	CString GetInputSoundFile();
	// 効果音ファイルパスを取得(候補選択)
	CString GetSelectSoundFile();
	// 効果音ファイルパスを取得(コマンド実行)
	CString GetExecuteSoundFile();

protected:
	AppPreference();
	~AppPreference();

public:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

