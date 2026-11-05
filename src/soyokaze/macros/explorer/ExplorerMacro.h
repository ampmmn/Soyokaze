#pragma once

#include "macros/core/MacroBase.h"

namespace launcherapp { namespace macros { namespace builtin {

class ExplorerMacro : public launcherapp::macros::core::MacroBase
{
	ExplorerMacro();
	virtual ~ExplorerMacro();

public:
	bool Evaluate(const std::vector<CString>& args, std::vector<CString>& results) override;

	bool ExpandLocationPath(const std::vector<CString>& args, std::vector<CString>& results);
	bool ExpandSelectionPath(const std::vector<CString>& args, std::vector<CString>& results);

	DECLARE_LAUNCHERMACRO(ExplorerMacro)
};


}}}
