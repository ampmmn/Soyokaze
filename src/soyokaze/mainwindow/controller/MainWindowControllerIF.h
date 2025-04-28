#pragma once

namespace launcherapp { namespace mainwindow { namespace controller {
	
class MainWindowControllerIF
{
public:
	typedef LRESULT (*CALLBACKFUNC)(LPARAM param);

	virtual ~MainWindowControllerIF(){}

	// キーが編集されたことを通知する(指定した仮想キーを入力する)
	virtual void InputKey(int vk) = 0;
	// ウインドウをアクティブにする
	virtual void ActivateWindow(bool isToggle) = 0;
	// コマンドを実行する
	virtual void RunCommand(LPCTSTR text, bool isWaitSync) = 0;
	// テキストを設定する
	virtual void SetText(LPCTSTR text) = 0;
	// ウインドウを非表示にする
	virtual void HideWindow() = 0;
	// アプリケーションを終了する
	virtual void QuitApplication() = 0;
	// クリップボードにテキストを設定
	virtual void SetClipboardString(HGLOBAL hMem, BOOL* isSetPtr) = 0;
	// クリップボードからテキストを取得
	virtual void GetClipboardString(CString& text) = 0;
	// フォーカスを失ったときに隠れるのを阻害する
	virtual void BlockDeactivateOnUnfocus(bool isBlock) = 0;
	// 候補を更新する依頼を出す(非同期)
	virtual void UpdateCandidateRequest() = 0;
	// 入力欄のテキストをクリップボードにコピーする
	virtual void CopyInputText() = 0;
	// 指定したコールバック関数をメインウインドウ側のスレッドで呼んでもらう
	virtual void RequestCallback(CALLBACKFUNC func, void* param) = 0;
	// 入力欄のテキストを空にする
	virtual void ClearContent() = 0;
	// コンテキストメニューを表示する
	virtual void ShowContextMenu() = 0;
};


}}} // end of namespace launcherapp::mainwindow::controller
