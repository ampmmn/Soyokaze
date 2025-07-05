#pragma once

#include "macros/core/MacroBase.h"

namespace launcherapp { namespace macros { namespace builtin {

class ExplorerMacro : public launcherapp::macros::core::MacroBase
{
	ExplorerMacro();
	virtual ~ExplorerMacro();

public:
	bool Evaluate(const std::vector<CString>& args, CString& result) override;

	bool ExpandLocationPath(const std::vector<CString>& args, CString& result);
	bool ExpandSelectionPath(const std::vector<CString>& args, CString& result);

	DECLARE_LAUNCHERMACRO(ExplorerMacro)
};


}}}
