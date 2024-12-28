#pragma once

#include "mainwindow/LauncherInputStatusIF.h"
#include <memory>

namespace launcherapp {
namespace mainwindow {

class MainWindowInput : public LauncherInput
{
public:
	MainWindowInput();
	~MainWindowInput() override;

public:
	bool HasKeyword() override;

	// キーワード文字列の長さを取得
	int GetLength();
	// キーワード文字列をクリア
	void Clear();
	// 末尾の1単語分の削除を行う
	void RemoveLastWord();
	//
	void SetKeyword(const CString& wholeText, bool withSpace=false);
	// キーワード文字列を得る
	const CString& GetKeyword();
	// キーワード文字列に(空白区切りで)引数を追加
	void AddArgument(const CString& arg);
	// Ctrl-Backspaceを押下時に入力される不可視文字(0x7E→Backspace)を消す
	bool ReplaceInvisibleChars();

	// DDX用
	CString& CommandStr();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}

