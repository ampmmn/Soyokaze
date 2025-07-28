#pragma once

#include "setting/AppPreferenceListenerIF.h"
#include "setting/Settings.h"
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
	bool IsEnablePathFind();

	// 管理者権限で起動する
	bool ShouldRunAsAdmin();
	// 管理者権限で実行しているときにコマンドを一般ユーザ権限で実行する
	bool ShouldDemotePriviledge();

	bool IsHideOnStartup();
	bool IsHideOnInactive();

	// 入力欄にアイコンを描画するか
	bool IsDrawIcon();
	// 入力欄にプレースホルダーを表示するか
	bool IsDrawPlaceHolder();
	// 入力画面にアプリケーション設定ボタンを表示するか?
	bool IsShowOptionButton();

	bool IsTopMost();
	bool IsShowToggle();

	bool IsWindowTransparencyEnable();
	int GetAlpha();
	bool IsTransparencyInactiveOnly();

	UINT GetModifiers();
	UINT GetVirtualKeyCode();

	bool IsEnableAppHotKey();
	bool IsEnableModifierHotKey();
	bool IsEnableModifierHotKeyOnRemoteDesktop();
	UINT GetFirstModifierVirtualKeyCode();
	UINT GetSecondModifierVirtualKeyCode();

	// 入力履歴機能を使用するか?
	bool IsUseInputHistory();
	// 履歴件数の上限を取得
	int GetHistoryLimit();

	// 入力画面表示時にIMEをオフにするか?
	bool IsIMEOffOnActive();

	// ネットワークパスを無視する
	bool IsIgnoreUNC();

	// 追加パスを取得
	void GetAdditionalPaths(std::vector<CString>& paths);

	// コメント表示欄の初期表示文字列を取得
	CString GetDefaultComment();

	void SetCommandKeyMappings(const CommandHotKeyMappings& keyMap);
	void GetCommandKeyMappings(CommandHotKeyMappings& keyMap);

	void RegisterListener(AppPreferenceListenerIF* listener);
	void UnregisterListener(AppPreferenceListenerIF* listener);

	bool CreateUserDirectory();

	bool IsEnableCalculator();
	CString GetPythonDLLPath();

	// ワークシート名による切り替え機能
	bool IsEnableExcelWorksheet();
	// ワークシート名による切り替え機能のプレフィックス
	CString GetWorksheetSwitchPrefix();
	// 
	bool IsEnablePowerPointSlide();
	// PowerPointシート名による切り替え機能のプレフィックス
	CString GetPresentationSwitchPrefix();

	// ウインドウの切り替え機能を有効にするか?
	bool IsEnableWindowSwitch();
	// ウインドウの切り替え機能のプレフィックス
	CString GetWindowSwitchPrefix();

	bool IsEnableBookmark();
	bool IsUseURLForBookmarkSearch();
	bool IsEnableHistoryChrome();
	bool IsEnableHistoryEdge();
	int GetBrowserHistoryTimeout();
	int GetBrowserHistoryCandidates();
	bool IsUseMigemoForBrowserHistory();
	bool IsUseURLForBrowserHistory();

	// 入力欄ウインドウをマウスカーソル位置に表示するか
	bool IsShowMainWindowOnCurorPos();
	// 入力欄ウインドウをアクティブなウインドウの中央に表示するか
	bool IsShowMainWindowOnActiveWindowCenter();

	// 入力欄ウインドウにコマンド種別を表示するか
	bool IsShowCommandType();

	// 操作ガイド欄を表示するか
	bool IsShowGuide();

	// 候補欄の背景色を交互に変えるか
	bool IsAlternateColor();

	// 候補欄の各項目にアイコンを描画するか
	bool IsDrawIconOnCandidate();

	// C/Migemo検索を利用するか
	bool IsEnableMigemo();

	// コントロールパネルのアイテム検索を使用するか
	bool IsEnableControlPanel();
	// ms-settingsのアイテム検索を使用するか
	bool IsEnableMSSettings();

	// スタートメニュー/最近使ったファイルのアイテム検索を使用するか
	bool IsEnableSpecialFolder();

	// UWPアプリの検索を使用するか
	bool IsEnableUWP();
	// MMCスナップインの検索を使用するか
	bool IsEnableMMCSnapin();

	// GitBashパス変換機能を使用するか
	bool IsEnableGitBashPath();
// fileプロトコルパス変換機能を使用するか
	bool IsEnableFileProtocolPathConvert();

	// 入力窓が消えるときにテキストを消去しない(コマンドを実行したときだけ入力欄をクリアする)
	bool IsKeepTextWhenDlgHide();

	// 効果音ファイルパスを取得(文字入力)
	CString GetInputSoundFile();
	// 効果音ファイルパスを取得(候補選択)
	CString GetSelectSoundFile();
	// 効果音ファイルパスを取得(コマンド実行)
	CString GetExecuteSoundFile();

	// 長時間の連続稼働を警告する
	bool IsWarnLongOperation();
	// 長時間連続稼働警告までの時間を取得する(分単位)
	int GetTimeToWarnLongOperation();

	// メイン画面のフォント名
	bool GetMainWindowFontName(CString& fontName);
	int GetMainWindowFontSize();

	// Everything検索コマンドでEverything APIを使うか?
	bool IsUseEverythingAPI();
	// ウインドウメッセージ経由でEverything検索を行うか?
	bool IsUseEverythingViaWM();
	// Everything検索実行時にEverythingが起動していなかった場合に起動するか?
	bool IsRunEverythingApp();
	// (Everything APIを使わない場合)Everything.exeのパスを取得する
	CString GetEverythingExePath();

	// キーワード未登録時のアクション
	CString GetDefaultActionType();

	// システム設定の色を使用するか?
	bool IsUseSystemColorSettings();
	COLORREF GetWindowTextColor();
	COLORREF GetWindowBackgroundColor();
	COLORREF GetEditTextColor();
	COLORREF GetEditBackgroundColor();
	COLORREF GetListTextColor();
	COLORREF GetListBackgroundColor();
	COLORREF GetListBackgroundAltColor();
	COLORREF GetListTextHighlightColor();
	COLORREF GetListBackgroundHighlightColor();

	// クリップボード履歴
	bool IsEnableClipboardHistory();
	CString GetClipboardHistoryPrefix();
	int GetClipboardHistoryNumberOfResults();
	int GetClipboardHistorySizeLimit();
	int GetClipboardHistoryCountLimit();
	int GetClipboardHistoryInterval();
	CString GetClipboardHistoryExcludePattern();
	bool IsDisableMigemoForClipboardHistory();

	// ログレベル
	int GetLogLevel();
	// 性能ログを出力するか?
	bool UsePerformanceLog();

protected:
	AppPreference();
	~AppPreference();

public:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

