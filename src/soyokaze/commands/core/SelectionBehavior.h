// あ
#pragma once

#include "commands/core/UnknownIF.h"

namespace launcherapp { namespace core {

class SelectionBehavior : virtual public UnknownIF
{
public:
	// 選択された
	virtual void OnSelect(Command* prior) = 0;
	// 選択解除された
	virtual void OnUnselect(Command* next) = 0;
};


}} // end of namespace launcherapp::core
