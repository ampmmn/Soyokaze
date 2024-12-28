#pragma once

#include "macros/core/MacroBase.h"

namespace launcherapp {
namespace macros {
namespace builtin {

class AppNameMacro : public launcherapp::macros::core::MacroBase
{
	AppNameMacro();
	virtual ~AppNameMacro();

public:
	bool Evaluate(const std::vector<CString>& args, CString& result) override;

	DECLARE_LAUNCHERMACRO(AppNameMacro)
};


}
}
}
