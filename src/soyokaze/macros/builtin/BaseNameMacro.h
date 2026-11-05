#pragma once

#include "macros/core/MacroBase.h"

namespace launcherapp {
namespace macros {
namespace builtin {

class BaseNameMacro : public launcherapp::macros::core::MacroBase
{
	BaseNameMacro();
	virtual ~BaseNameMacro();

public:
	bool Evaluate(const std::vector<CString>& args, std::vector<CString>& results) override;

	DECLARE_LAUNCHERMACRO(BaseNameMacro)
};


}
}
}
