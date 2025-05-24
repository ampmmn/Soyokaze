// あ
#pragma once

#include "commands/core/UnknownIF.h"

namespace launcherapp { namespace core {

class SelectionBehavior : virtual public UnknownIF
{
public:
	enum CloseWindowPolicy {
		CLOSEWINDOW_ASYNC,
		CLOSEWINDOW_SYNC,
	};
public:
	// 選択された
	virtual void OnSelect(Command* prior) = 0;
	// 選択解除された
	virtual void OnUnselect(Command* next) = 0;
	// 実行後のウインドウを閉じる方法
	virtual CloseWindowPolicy GetCloseWindowPolicy() = 0;
};


}} // end of namespace launcherapp::core
