// あ
#pragma once

#include "core/UnknownIF.h"

class CommandHotKeyAttribute;

namespace launcherapp {
namespace commands {
namespace core {

class ExtraCandidate : virtual public launcherapp::core::UnknownIF
{
public:
	virtual CString GetSourceName() = 0;
};


} // end of namespace core
} // end of namespace commands
} // end of namespace launcherapp
