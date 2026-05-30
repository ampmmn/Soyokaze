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
	// カレントディレクトリを変更
	CHANGE_DIRECTORY,
	// ウインドウを消す
	HIDE,
	//
	ACTIVATE_WINDOW,
};

struct SEND_COMMAND_PARAM
{
	bool mIsPasteOnly;
	char mText[1];
};

struct SET_CARETRANGE_PARAM
{
	int mStartPos;
	int mLength;
};

struct CHANGE_DIRECTORY_PARAM
{
	char mDirPath[1];
};


}
}
}
