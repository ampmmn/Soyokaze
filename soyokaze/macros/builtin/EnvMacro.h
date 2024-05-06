#pragma once

#include "macros/core/MacroBase.h"

namespace launcherapp {
namespace macros {
namespace builtin {

class EnvMacro : public launcherapp::macros::core::MacroBase
{
	EnvMacro();
	virtual ~EnvMacro();

public:
	bool Evaluate(const std::vector<CString>& args, CString& result) override;

	DECLARE_LAUNCHERMACRO(EnvMacro)
};


}
}
}
