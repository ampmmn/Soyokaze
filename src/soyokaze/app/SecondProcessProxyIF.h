#pragma once


namespace launcherapp {

/**
 *  後発プロセスが先行プロセスに対してコマンドを送信する処理としてのI/F
 */
class SecondProcessProxyIF
{
public:
	virtual ~SecondProcessProxyIF() {}

	// コマンド文字列を通知する
	virtual bool SendCommandString(const CString& commandStr, bool isPasteOnly) = 0;
	// 選択範囲を通知する
	virtual bool SendCaretRange(int startPos, int length) = 0;
	// パス登録
	virtual bool RegisterPath(const CString& pathStr) = 0;
	// カレントディレクトリを変更する
	virtual bool ChangeDirectory(const CString& pathStr) = 0;
	// ウインドウを消す
	virtual bool Hide() = 0;
	// ウインドウを表示する
	virtual bool Show() = 0;
};

}

