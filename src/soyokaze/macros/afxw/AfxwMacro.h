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
	bool Evaluate(const std::vector<CString>& args, std::vector<CString>& results) override;

	bool ExpandLocationPath(const std::vector<CString>& args, std::vector<CString>& results);
	bool ExpandSelectionPath(const std::vector<CString>& args, std::vector<CString>& results);

	DECLARE_LAUNCHERMACRO(AfxwMacro)
};


}
}
}
