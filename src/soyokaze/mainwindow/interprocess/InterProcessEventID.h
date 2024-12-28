#pragma once

namespace launcherapp {
namespace mainwindow {
namespace interprocess {

enum EVENT_ID
{
	// コマンド文字列を通知
	SEND_COMMAND = 0,
	// 選択範囲を通知
	SET_CARETRANGE,
	// パスを登録
	REGISTER_PATH,
	// カレントディレクトリを変更
	CHANGE_DIRECTORY,
	// ウインドウを消す
	HIDE,
	// ウインドウを表示
	SHOW
};

struct SEND_COMMAND_PARAM
{
	bool mIsPasteOnly;
	TCHAR mText[1];
};

struct SET_CARETRANGE_PARAM
{
	int mStartPos;
	int mLength;
};

struct CHANGE_DIRECTORY_PARAM
{
	TCHAR mDirPath[1];
};


}
}
}
