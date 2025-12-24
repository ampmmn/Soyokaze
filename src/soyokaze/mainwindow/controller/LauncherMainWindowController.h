#pragma once

#include "mainwindow/controller/MainWindowControllerIF.h"

namespace launcherapp { namespace mainwindow { namespace controller {

enum LauncherMainWindowMessageID  {
	INPUTKEY = WM_APP + 1,
	ACTIVATEWINDOW = WM_APP + 2,
	RUNCOMMAND = WM_APP + 3,
	HIDEWINDOW = WM_APP + 7,
	QUITAPPLICATION = WM_APP + 8,
	SETCLIPBOARDSTRING = WM_APP + 9,
	GETCLIPBOARDSTRING = WM_APP + 10,
	SETTEXT = WM_APP + 11,
	BLOCKDEACTIVATE = WM_APP + 14,
	COPYINPUTTEXT = WM_APP + 16,
	UPDATECANDIDATE = WM_APP + 15,
	REQUESTCALLBACK = WM_APP+17,
	CLEARCONTENT = WM_APP + 18,
	MOVETEMPORARY = WM_APP + 19,
	BLOCKWINDOWDIAPLAY =WM_APP + 20, 
	POPUPMESSAGE =WM_APP + 21, 
	EXPANDMACRO =WM_APP + 22, 
	RELEASEMACROSTR =WM_APP + 23, 
	DELETEWORD = WM_APP + 24,
};
	
class LauncherMainWindowController : public MainWindowControllerIF
{
public:
	LauncherMainWindowController();
	~LauncherMainWindowController();

	// キーが編集されたことを通知する(指定した仮想キーを入力する)
	void InputKey(int vk) override;
	// ウインドウをアクティブにする
	void ActivateWindow(bool isToggle) override;
	// コマンドを実行する
	void RunCommand(LPCTSTR text, bool isWaitSync) override;
	// テキストを設定する
	void SetText(LPCTSTR text) override;
	// ウインドウを非表示にする
	void HideWindow() override;
	// アプリケーションを終了する
	void QuitApplication() override;
	// クリップボードにテキストを設定
	void SetClipboardString(HGLOBAL hMem, BOOL* isSetPtr) override;
	// クリップボードからテキストを取得
	void GetClipboardString(CString& text) override;
	// フォーカスを失ったときに隠れるのを阻害する
	void BlockDeactivateOnUnfocus(bool isBlock) override;
	// 候補を更新する依頼を出す(非同期)
	void UpdateCandidateRequest() override;
	// 入力欄のテキストをクリップボードにコピーする
	void CopyInputText() override;
	// 指定したコールバック関数をメインウインドウ側のスレッドで呼んでもらう
	void RequestCallback(CALLBACKFUNC func, void* param) override;
	// 入力欄のテキストを空にする
	void ClearContent() override;
	// コンテキストメニューを表示する
	void ShowContextMenu() override;
	// ウインドウを一時的に移動する
	bool MoveTemporary(int vk) override;
	// メインウインドウの表示を抑制する
	void BlockWindowDisplay(bool isBlock) override;
	// 単語単位の削除
	void DeleteWord() override;

};


}}} // end of namespace launcherapp::mainwindow::controller
