#pragma once

#include "commands/core/CommandProviderIF.h"

namespace launcherapp { namespace core {

class UserCommandProvider : virtual public CommandProvider
{
public:
	// 派生クラス側で実装する必要のあるメソッド
	virtual void OnBeforeLoad() = 0;
	virtual bool LoadFrom(CommandEntryIF* entry, Command** command) = 0;
};

}}

