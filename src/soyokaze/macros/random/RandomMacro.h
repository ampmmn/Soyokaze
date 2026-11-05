#pragma once

#include "macros/core/MacroBase.h"

namespace launcherapp {
namespace macros {
namespace random {

class RandomMacro : public launcherapp::macros::core::MacroBase
{
	RandomMacro();
	virtual ~RandomMacro();

public:
	bool Evaluate(const std::vector<CString>& args, std::vector<CString>& results) override;

	DECLARE_LAUNCHERMACRO(RandomMacro)
};


}
}
}
