// あ
#pragma once

#include "core/UnknownIF.h"
#include "commands/core/CommandIF.h"

namespace launcherapp { namespace core {

class SelectionBehavior : virtual public UnknownIF
{
public:
	// 入力ウインドウのクローズに関する方針
	enum CloseWindowPolicy {
		// コマンド実行時にGUIスレッド側でウインドウを閉じる(コマンド完了を待たない)
		CLOSEWINDOW_ASYNC,
		// コマンド実行完了後に実行スレッド側でウインドウを閉じる(コマンド完了を待つ)
		CLOSEWINDOW_SYNC,
		// ウインドウを閉じない
		CLOSEWINDOW_NOCLOSE,
	};
public:
	// 選択された
	virtual void OnSelect(Command* prior) = 0;
	// 選択解除された
	virtual void OnUnselect(Command* next) = 0;
	// 実行後のウインドウを閉じる方法
	virtual CloseWindowPolicy GetCloseWindowPolicy(uint32_t modifierMask) = 0;
	// 選択時に入力欄に設定するキーワードとキャレットを設定する
	virtual bool CompleteKeyword(CString& keyword, int& startPos, int& endPos) = 0;
};


}} // end of namespace launcherapp::core
