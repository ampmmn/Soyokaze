#pragma once

#include "macros/core/MacroBase.h"

namespace launcherapp {
namespace macros {
namespace powerstatus {

class PowerStatusMacro : public launcherapp::macros::core::MacroBase
{
	PowerStatusMacro();
	virtual ~PowerStatusMacro();

public:
	bool Evaluate(const std::vector<CString>& args, CString& result) override;

	DECLARE_LAUNCHERMACRO(PowerStatusMacro)
};


}
}
}
