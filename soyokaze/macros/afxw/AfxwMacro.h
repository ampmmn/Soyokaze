#pragma once

#include "macros/core/MacroBase.h"

namespace launcherapp {
namespace macros {
namespace builtin {

class AfxwMacro : public launcherapp::macros::core::MacroBase
{
	AfxwMacro();
	virtual ~AfxwMacro();

public:
	bool Evaluate(const std::vector<CString>& args, CString& result) override;

	DECLARE_LAUNCHERMACRO(AfxwMacro)
};


}
}
}
